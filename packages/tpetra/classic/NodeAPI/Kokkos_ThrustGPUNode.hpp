//@HEADER
// ************************************************************************
//
//          Kokkos: Node API and Parallel Node Kernels
//              Copyright (2008) Sandia Corporation
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
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#ifndef KOKKOS_THRUSTGPUNODE_HPP_
#define KOKKOS_THRUSTGPUNODE_HPP_

/// \file Kokkos_ThrustGPUNode.hpp
/// \brief Declaration and definition of the (now DEPRECATED)
///   KokkosClassic::ThrustGPUNode Node type.
/// \warning KokkosClassic::ThrustGPUNode has been DEPRECATED.  For a
///   Node that uses CUDA for thread-level parallelism, please use
///   Kokkos::Compat::KokkosCudaWrapperNode instead.

#include "Kokkos_ConfigDefs.hpp"

// mfh 08 Jan 2015: Don't enable the contents of this file unless the
// appropriate CMake option is enabled.  This avoids deprecation
// warnings once we deprecate this Node type.
#ifdef HAVE_TPETRACLASSIC_THRUST

#include "Kokkos_NodeAPIConfigDefs.hpp"
#include "Kokkos_CUDANodeMemoryModel.hpp"
#include "Kokkos_ThrustGPUWrappers.hpp"

#ifdef HAVE_TPETRACLASSIC_TEUCHOSKOKKOSCOMPAT
#  include "Kokkos_Core.hpp"
#endif

namespace KokkosClassic {

/// \brief Node API implementation that uses CUDA via the Thrust TPL
///   for thread-level parallelism.
/// \ingroup kokkos_node_api
/// \ingroup kokkos_cuda_support
///
/// \warning KokkosClassic::ThrustGPUNode has been DEPRECATED.  For a
///   Node that uses CUDA for thread-level parallelism, please use
///   Kokkos::Compat::KokkosCudaWrapperNode instead.
class TPETRA_DEPRECATED ThrustGPUNode : public CUDANodeMemoryModel {
public:
  /// \brief This is a "classic" Node type.
  ///
  /// That means we plan to deprecate it with the 11.14 release of
  /// Trilinos, and remove it entirely with the 12.0 release.
  static const bool classic = true;

  //! Constructor that sets default parameters.
  ThrustGPUNode ();

  /*! \brief Constructor that takes a list of parameters.

    We accept the following parameters:
    - "Device Number" [int] The CUDA device to which the Node will attach.
    - "Verbose" [int] Non-zero parameter specifies that the
      constructor is verbose, printing information about the the
      attached device. Default: 0.

    The constructor will throw std::runtime_error if "Device Number"
    is outside the range \f$[0,numDevices)\f$, where \c numDevices is
    the number of CUDA devices reported by cudaGetDeviceCount().
  */
  ThrustGPUNode (Teuchos::ParameterList &pl);

  //! Destructor.
  ~ThrustGPUNode ();

  //! Get default parameters for this node.
  static ParameterList getDefaultParameters();

  //@{ Computational methods

  /// \brief Parallel for "skeleton"; wraps thrust::for_each.
  ///
  /// See \ref kokkos_node_api "Kokkos Node API"
  template <class WDP>
  static void parallel_for(int begin, int end, WDP wdp);

  /// \brief Parallel reduction "skeleton"; wraps thrust::transform_reduce.
  ///
  /// See \ref kokkos_node_api "Kokkos Node API"
  template <class WDP>
  static typename WDP::ReductionType
  parallel_reduce(int begin, int end, WDP wd);

  /// \brief Block until all node work is complete.
  ///
  /// Aids in accurate timing of multiple kernels.
  void sync() const;

  /// \brief Return the human-readable name of this Node.
  ///
  /// See \ref kokkos_node_api "Kokkos Node API"
  static std::string name ();
  //@}

private:
  int totalMem_;
};

template <class WDP>
void ThrustGPUNode::parallel_for(int begin, int end, WDP wd) {
  ThrustGPUNodeDetails::parallel_for(begin,end,wd);
}

template <class WDP>
typename WDP::ReductionType
ThrustGPUNode::parallel_reduce(int begin, int end, WDP wd)
{
  return ThrustGPUNodeDetails::parallel_reduce(begin,end,wd);
}

} // namespace KokkosClassic


#if defined(HAVE_TPETRACLASSIC_TEUCHOSKOKKOSCOMPAT) && defined(KOKKOS_HAVE_CUDA)
namespace Kokkos {
  namespace Compat {
    template <>
    struct NodeDevice<KokkosClassic::ThrustGPUNode> {
      typedef Kokkos::Cuda type;
    };
  }
}
#endif

#endif // HAVE_TPETRACLASSIC_THRUST
#endif // KOKKOS_THRUSTGPUNODE_HPP_