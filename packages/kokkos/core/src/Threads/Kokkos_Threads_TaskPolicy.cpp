/*
//@HEADER
// ************************************************************************
//
//   Kokkos: Manycore Performance-Portable Multidimensional Arrays
//              Copyright (2012) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact  H. Carter Edwards (hcedwar@sandia.gov)
//
// ************************************************************************
//@HEADER
*/

// Experimental unified task-data parallel manycore LDRD

#include <stdio.h>
#include <iostream>
#include <Threads/Kokkos_Threads_TaskPolicy.hpp>

#if defined( KOKKOS_HAVE_PTHREAD )

namespace Kokkos {
namespace Experimental {
namespace Impl {

enum { paranoid_atomic_protection = true };

typedef TaskMember< Kokkos::Threads , void , void > Task ;

namespace {

int    volatile s_ready_count = 0 ;
Task * volatile s_ready_team = 0 ;
Task * volatile s_ready_serial = 0 ;
Task * const    s_lock   = reinterpret_cast<Task*>( ~((unsigned long)0) );
Task * const    s_denied = reinterpret_cast<Task*>( ~((unsigned long)0) - 1 );
Task * const    s_bogus  = reinterpret_cast<Task*>( ~((unsigned long)0) - 2 );

} /* namespace */
} /* namespace Impl */
} /* namespace Experimental */
} /* namespace Kokkos */

namespace Kokkos {
namespace Experimental {
namespace Impl {

//----------------------------------------------------------------------------

void Task::throw_error_verify_type()
{
  Kokkos::Impl::throw_runtime_exception("TaskMember< Threads >::verify_type ERROR");
}

//----------------------------------------------------------------------------

int Task::team_fixed_size()
{
  // If a team task then claim for execution until count is zero
  // Issue: team collectives cannot assume which pool members are in the team.
  // Issue: team must only span a single NUMA region.

  // If more than one thread per core then map cores to work team,
  // else  map numa to work team.

  const int threads_per_numa = Threads::thread_pool_size(1);
  const int threads_per_core = Threads::thread_pool_size(2);
  const int threads_per_team = 1 < threads_per_core ? threads_per_core : threads_per_numa ;

  return threads_per_team ;
}

//----------------------------------------------------------------------------

void Task::deallocate( void * ptr )
{
  free( ptr );
}

void * Task::allocate( const unsigned n )
{
  void * const ptr = malloc(n);

  return ptr ;
}

Task::~TaskMember()
{
}

//----------------------------------------------------------------------------

void Task::reschedule()
{
  // Reschedule transitions from executing back to waiting.
  const int old_state = atomic_compare_exchange( & m_state , int(TASK_STATE_EXECUTING) , int(TASK_STATE_WAITING) );

  if ( old_state != int(TASK_STATE_EXECUTING) ) {

fprintf( stderr
       , "reschedule ERROR task[%lx] state(%d)\n"
       , (unsigned long) this
       , old_state
       );
fflush(stderr);

  }
}

void Task::schedule()
{
  //----------------------------------------
  // State is either constructing or already waiting.
  // If constructing then transition to waiting.

  {
    const int old_state = atomic_compare_exchange( & m_state , int(TASK_STATE_CONSTRUCTING) , int(TASK_STATE_WAITING) );
    Task * const waitTask = *((Task * volatile const *) & m_wait );
    Task * const next = *((Task * volatile const *) & m_next );

    if ( s_denied == waitTask || 0 != next ||
         ( old_state != int(TASK_STATE_CONSTRUCTING) &&
           old_state != int(TASK_STATE_WAITING) ) ) {
      fprintf(stderr,"Task::schedule task(0x%lx) STATE ERROR: state(%d) wait(0x%lx) next(0x%lx)\n"
                    , (unsigned long) this
                    , old_state
                    , (unsigned long) waitTask
                    , (unsigned long) next );
      fflush(stderr);
      Kokkos::Impl::throw_runtime_exception("Kokkos::Impl::Task spawn or respawn state error");
    }
  }

  //----------------------------------------
  // Insert this task into another dependence that is not complete
  // Push on to the wait queue, fails if ( s_denied == m_dep[i]->m_wait )

  bool insert_in_ready_queue = true ;

  {
    Task * wait_queue_head = s_bogus ; // Start with impossible value

    for ( int i = 0 ; i < m_dep_size && insert_in_ready_queue ; ) {

      if ( s_denied == wait_queue_head ) {
        // Wait queue is closed, try again with the next queue
        ++i ;
        wait_queue_head = s_bogus ; // Reset to impossible value
      }
      else {

        Task * const task_dep = m_dep[i] ;

        // Wait queue is open and not locked.
        // Read the queue exclusively via CAS, if CAS succeeds then have acquired the lock.

        // Have exclusive access to this task.
        // Assign m_next assuming a successfull insertion into the queue.
        // Fence the memory assignment before attempting the insert.

        *((Task * volatile *) & m_next ) = wait_queue_head ;

        memory_fence();

        Task * const head_value_old = wait_queue_head ;

        // Attempt to insert this task into the queue

        wait_queue_head = atomic_compare_exchange( & task_dep->m_wait , head_value_old , this );

        if ( head_value_old == wait_queue_head ) {
          insert_in_ready_queue = false ;
        }
      }
    }
  }

  //----------------------------------------
  // All dependences are complete, insert into the ready list

  if ( insert_in_ready_queue ) {

    // Increment the count of ready tasks.
    // Count is decremented when task is complete.

    atomic_increment( & s_ready_count );

    Task * ready_queue_head = s_bogus ; // Start with impossible value

    while ( insert_in_ready_queue ) {

      // Read the head of ready queue, if same as previous value then CAS locks the ready queue
      // Only access via CAS

      // Have exclusive access to this task, assign to head of queue, assuming successful insert
      // Fence assignment before attempting insert.
      *((Task * volatile *) & m_next ) = ready_queue_head ;

      memory_fence();

      Task * const head_value_old = ready_queue_head ;

      ready_queue_head = atomic_compare_exchange( & s_ready_serial , head_value_old , this );

      if ( head_value_old == ready_queue_head ) {
        // Successful insert
        insert_in_ready_queue = false ; // done
      }
      else if ( s_lock == ready_queue_head ) {
        // Ready queue was locked, don't try to steal the lock
        ready_queue_head = s_bogus ;
      }
    }
  }
}

//----------------------------------------------------------------------------

#if defined( KOKKOS_ACTIVE_EXECUTION_MEMORY_SPACE_HOST )

void Task::assign( Task ** const lhs_ptr , Task * rhs )
{
  // Assign the pointer and retrieve the previous value.

  Task * const old_lhs = atomic_exchange( lhs_ptr , rhs );

  // Increment rhs reference count.
  if ( rhs ) { atomic_increment( & rhs->m_ref_count ); }

  if ( old_lhs ) {

    // Decrement former lhs reference count.
    // If reference count is zero task must be complete, then delete task.
    // Task is ready for deletion when  wait == s_denied

    int    const count = atomic_fetch_add( & (old_lhs->m_ref_count) , -1 ) - 1 ;
    Task * const wait  = *((Task * const volatile *) & old_lhs->m_wait );

    if ( count < 0 || ( count == 0 && wait != s_denied ) ) {

      static const char msg_error_header[]  = "Kokkos::Impl::TaskManager<Kokkos::Threads>::assign ERROR deleting" ;

      fprintf( stderr , "%s task(0x%lx) m_ref_count(%d) , m_wait(0x%ld)\n"
                      , msg_error_header
                      , (unsigned long) old_lhs
                      , count
                      , (unsigned long) wait );
      fflush(stderr);

      Kokkos::Impl::throw_runtime_exception( msg_error_header );
    }

    if ( count == 0 ) {
      const Task::function_dealloc_type d = old_lhs->m_dealloc ;
      (*d)( old_lhs );
    }
  }
}

#endif

//----------------------------------------------------------------------------

Task * Task::get_dependence( int i ) const
{
  Task * const t = m_dep[i] ;

  if ( Kokkos::Experimental::TASK_STATE_EXECUTING != m_state || i < 0 || m_dep_size <= i || 0 == t ) {

fprintf( stderr
       , "TaskMember< Threads >::get_dependence ERROR : task[%lx]{ state(%d) dep_size(%d) dep[%d] = %lx }\n"
       , (unsigned long) this
       , m_state
       , m_dep_size
       , i
       , (unsigned long) t
       );
fflush( stderr );

    Kokkos::Impl::throw_runtime_exception("TaskMember< Threads >::get_dependence ERROR");
  }

  return t ;
}

//----------------------------------------------------------------------------

void Task::add_dependence( Task * before )
{
  int const state = *((volatile const int *) & m_state );

 // Can add dependence during construction or during execution

  if ( ( Kokkos::Experimental::TASK_STATE_CONSTRUCTING == state ||
         Kokkos::Experimental::TASK_STATE_EXECUTING    == state ) &&
       before != 0 &&
       m_dep_size < m_dep_capacity ) {

    ++m_dep_size ;

    assign( m_dep + (m_dep_size-1) , before );

    memory_fence();
  }
  else {

fprintf( stderr
       , "TaskMember< Threads >::add_dependence ERROR : task[%lx]{ state(%d) dep_size(%d) before(%lx) }\n"
       , (unsigned long) this
       , m_state
       , m_dep_size
       , (unsigned long) before
       );
fflush( stderr );

    Kokkos::Impl::throw_runtime_exception("TaskMember< Threads >::add_dependence ERROR");
  }
}

//----------------------------------------------------------------------------

void Task::clear_dependence()
{
  for ( int i = m_dep_size - 1 ; 0 <= i ; --i ) {
    assign( m_dep + i , 0 );
  }

  *((volatile int *) & m_dep_size ) = 0 ;

  memory_fence();
}

//----------------------------------------------------------------------------

void Task::execute_serial( Task * const task )
{
  if ( task->m_serial ) (*task->m_serial)( task );

  // State is either executing or if respawned then waiting,
  // try to transition from executing to complete.
  // Reads the current value.

  const int state_old =
    atomic_compare_exchange( & task->m_state
                           , int(Kokkos::Experimental::TASK_STATE_EXECUTING)
                           , int(Kokkos::Experimental::TASK_STATE_COMPLETE) );

  if ( Kokkos::Experimental::TASK_STATE_WAITING == state_old ) {
    task->schedule(); /* Task requested a respawn so reschedule it */
  }
  else if ( Kokkos::Experimental::TASK_STATE_EXECUTING != state_old ) {
    fprintf( stderr
           , "TaskMember< Threads >::execute_serial completion ERROR : task[%lx]{ state_old(%d) dep_size(%d) }\n"
           , (unsigned long) & task
           , state_old
           , task->m_dep_size
           );
    fflush( stderr );
  }
  else {

    // Clear dependences of this task before locking wait queue

    task->clear_dependence();

    // Stop other tasks from adding themselves to this task's wait queue.
    // The wait queue is updated concurrently so guard with an atomic.
    // Setting the wait queue to denied denotes delete-ability of the task by any thread.
    // Therefore, once 'denied' the task pointer must be treated as invalid.

    Task * wait_queue     = s_bogus ;
    Task * wait_queue_old = 0 ;

    do {
      wait_queue_old = wait_queue ;
      wait_queue     = atomic_compare_exchange( & task->m_wait , wait_queue_old , s_denied );
    } while ( wait_queue_old != wait_queue );

    // Pop waiting tasks and schedule them
    while ( wait_queue ) {
      Task * const x = wait_queue ; wait_queue = x->m_next ; x->m_next = 0 ;
      x->schedule();
    }
  }
}

bool Task::execute_ready_team_task( Kokkos::Impl::ThreadsExecTeamMember & member )
{
  enum { skip = true };

  std::pair< Task * , int > task_work(0,0) ;

  // If the team-task queue has work then try to execute from there

  if ( skip ) {

  if ( member.team_broadcast_root() ) {

    // One member of the team attempts to claim a unit of work.

    while ( ( 0 != ( task_work.first = s_ready_team ) ) &&
            ( ( task_work.second = atomic_fetch_add( & task_work.first->m_league_rank , -1 ) ) < 0 ) );

    if ( 0 != task_work.first ) {

      // Claimed the last unit of work for this task so remove this task from the queue.

      s_ready_team = task_work.first->m_next ;

      task_work.first->m_next = 0 ;
    }
  }

  // Broadcast task to team members, a barrier for the team
  member.team_broadcast_root( task_work );

  if ( 0 != task_work.first ) {

    // Work is claimed from the team task queue.

    // If state is not executing then set state to executing.
    // ok if this happens concurrently-redundantly

    if ( task_work.first->m_state != Kokkos::Experimental::TASK_STATE_EXECUTING ) {
      task_work.first->m_state = Kokkos::Experimental::TASK_STATE_EXECUTING ;
    }

    // Set league rank and size on member
    // 'm_league_rank' is initialized to league_size - 1.

    member.set_league_shmem( task_work.second                // league rank
                           , task_work.first->m_league_size  // league size
                           , task_work.first->m_shmem_size   // team shared size
                           );

    (*task_work.first->m_team)( task_work.first , member );

    member.team_barrier();

    if ( ( member.team_rank() == 0 ) &&
         ( 0 == atomic_fetch_add( & task_work.first->m_league_end , -1 ) ) ) {
      execute_serial( task_work.first );
    }
  }
  }

  // Return if team task queue was not empty
  return task_work.first != 0 ;
}

void Task::execute_ready_tasks_driver( Kokkos::Impl::ThreadsExec & exec , const void * )
{
  // Whole pool is calling this function

  Kokkos::Impl::ThreadsExecTeamMember member( exec , TeamPolicy< Kokkos::Threads >( 1 , team_fixed_size() ) , 0 );

  long int iteration_count = 0 ;

  // Each team must iterate this loop synchronously to insure team-execution of team-task

  Task * task = s_bogus ;

  while ( 0 < s_ready_count ) {

    // Team grabs work from s_ready_team:  execute_ready_team_task( member );
    // OR
    // Read the head of the ready queue, if head is previous value then acquire and lock the ready queue

    Task * const task_old = task ;

    task = atomic_compare_exchange( & s_ready_serial , task_old , s_lock );

    if ( task_old == task ) {

      // May have acquired the lock and task.
      // One or more other threads may have acquired this same task and lock
      // due to respawning ABA race condition.
      // Can only be sure of acquire with a successful state transition from waiting to executing

      const int old_state = atomic_compare_exchange( & task->m_state, int(TASK_STATE_WAITING), int(TASK_STATE_EXECUTING) );

      if ( old_state == int(TASK_STATE_WAITING) ) {

        // Transitioned this task from waiting to executing
        // Update the queue to the next entry and release the lock

        Task * const next_old = *((Task * volatile *) & task->m_next );

        Task * const s = atomic_compare_exchange( & s_ready_serial , s_lock , next_old );

        if ( s != s_lock ) {
          fprintf(stderr,"Task::execute serial pop() UNLOCK ERROR\n");
          fflush(stderr);
        }

        *((Task * volatile *) & task->m_next ) = 0 ;

        execute_serial( task );

        atomic_decrement( & s_ready_count );
      }
      else if ( old_state == int(TASK_STATE_EXECUTING) || old_state == int(TASK_STATE_COMPLETE) ) {
        // Failed the race condition probably due to ABA
        // and the task is either already executing or has completed.
        task = s_bogus ;
      }
      else {
        fprintf(stderr,"Task::execute serial task(0x%lx) state(%d) ERROR\n"
                      , (unsigned long) task
                      , old_state );
        fflush(stderr);
      }
    }
    else if ( s_lock == task || 0 == task ) {
      // Don't attempt to steal a lock or acquire a null task, reset to bogus value.
      task = s_bogus ;
    }

    ++iteration_count ;
  }

  exec.fan_in();
}

void Task::execute_ready_tasks()
{
  Kokkos::Impl::ThreadsExec::start( & Task::execute_ready_tasks_driver , 0 );
  Kokkos::Impl::ThreadsExec::fence();
}

void Task::wait( const Future< void , Kokkos::Threads > & f )
{
  Kokkos::Impl::ThreadsExec::start( & Task::execute_ready_tasks_driver , 0 );
  Kokkos::Impl::ThreadsExec::fence();
}

} /* namespace Impl */
} /* namespace Experimental */
} /* namespace Kokkos */

#endif /* #if defined( KOKKOS_HAVE_PTHREAD ) */