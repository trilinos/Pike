/** \HEADER
 *************************************************************************
 *
 *                            Kokkos
 *                 Copyright 2010 Sandia Corporation
 *
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the Corporation nor the names of the
 *  contributors may be used to endorse or promote products derived from
 *  this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************
 */

#ifndef KOKKOS_HOSTMAP_HPP
#define KOKKOS_HOSTMAP_HPP

#include <Kokkos_HostDevice.hpp>

namespace Kokkos {

template< typename ValueType , class DeviceMapType > class MDArrayView ;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class HostMap {
public:

  typedef HostMap    device_map_type ;
  typedef HostDevice device_type ;
  typedef size_t     size_type ;

  ~HostMap() {}

  HostMap( size_type parallel_work_count )
    : m_parallel_work_count( parallel_work_count ) {}

  size_type parallel_work_count() const { return m_parallel_work_count ; }

  /*------------------------------------------------------------------------*/
  /** \brief Allocate rank-8 array mapped onto the device. */
  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_labeled_mdarray( size_type n0 , size_type n1 ,
                            size_type n2 , size_type n3 ,
                            size_type n4 , size_type n5 ,
                            size_type n6 , const std::string & label )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,n1,n2,n3,n4,n5,n6,m_parallel_work_count,label);
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_labeled_mdarray( size_type n0 , size_type n1 ,
                            size_type n2 , size_type n3 ,
                            size_type n4 , size_type n5 ,
                            const std::string & label )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,n1,n2,n3,n4,n5,m_parallel_work_count,label);
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_labeled_mdarray( size_type n0 , size_type n1 ,
                            size_type n2 , size_type n3 ,
                            size_type n4 , const std::string & label )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,n1,n2,n3,n4,m_parallel_work_count,label);
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_labeled_mdarray( size_type n0 , size_type n1 ,
                            size_type n2 , size_type n3 ,
                            const std::string & label )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,n1,n2,n3,m_parallel_work_count,label);
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_labeled_mdarray( size_type n0 , size_type n1 ,
                            size_type n2 ,
                            const std::string & label )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,n1,n2,m_parallel_work_count,label);
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_labeled_mdarray( size_type n0 , size_type n1 ,
                            const std::string & label )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,n1,m_parallel_work_count,label);
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_labeled_mdarray( size_type n0 , const std::string & label )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,m_parallel_work_count,label);
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_labeled_mdarray( const std::string & label )
    {
      return MDArrayView<ValueType,device_map_type>
        (m_parallel_work_count,label);
    }

  /*------------------------------------------------------------------------*/
  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_mdarray( size_type n0 , size_type n1 ,
                    size_type n2 , size_type n3 ,
                    size_type n4 , size_type n5 ,
                    size_type n6 )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,n1,n2,n3,n4,n5,n6,m_parallel_work_count,std::string());
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_mdarray( size_type n0 , size_type n1 ,
                    size_type n2 , size_type n3 ,
                    size_type n4 , size_type n5 )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,n1,n2,n3,n4,n5,m_parallel_work_count,std::string());
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_mdarray( size_type n0 , size_type n1 ,
                    size_type n2 , size_type n3 ,
                    size_type n4 )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,n1,n2,n3,n4,m_parallel_work_count,std::string());
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_mdarray( size_type n0 , size_type n1 ,
                    size_type n2 , size_type n3 )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,n1,n2,n3,m_parallel_work_count,std::string());
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_mdarray( size_type n0 , size_type n1 ,
                    size_type n2 )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,n1,n2,m_parallel_work_count,std::string());
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_mdarray( size_type n0 , size_type n1 )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,n1,m_parallel_work_count,std::string());
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_mdarray( size_type n0 )
    {
      return MDArrayView<ValueType,device_map_type>
        (n0,m_parallel_work_count,std::string());
    }

  template< typename ValueType >
  inline
  MDArrayView<ValueType,device_map_type>
    create_mdarray()
    {
      return MDArrayView<ValueType,device_map_type>
        (m_parallel_work_count,std::string());
    }

  /*------------------------------------------------------------------------*/

private:

  size_type m_parallel_work_count ;

  HostMap();
  HostMap( const HostMap & );
  HostMap & operator = ( const HostMap & );
};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

} // namespace Kokkos

#endif /* KOKKOS_HOSTMAP_HPP */


