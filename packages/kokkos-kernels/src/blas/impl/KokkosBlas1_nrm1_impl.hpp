/*
//@HEADER
// ************************************************************************
//
//                        Kokkos v. 3.0
//       Copyright (2020) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
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
// THIS SOFTWARE IS PROVIDED BY NTESS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NTESS OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Siva Rajamanickam (srajama@sandia.gov)
//
// ************************************************************************
//@HEADER
*/
#ifndef KOKKOSBLAS1_NRM1_IMPL_HPP_
#define KOKKOSBLAS1_NRM1_IMPL_HPP_

#include <KokkosKernels_config.h>
#include <Kokkos_Core.hpp>
#include <KokkosBlas1_nrm1_spec.hpp>
#include <KokkosBlas_util.hpp>

namespace KokkosBlas {
namespace Impl {

//
// nrm1
//

/// \brief 1-norm functor for single vectors.
///
/// \tparam RV 0-D output View
/// \tparam XV 1-D input View
/// \tparam SizeType Index type.  Use int (32 bits) if possible.
template <class RV, class XV, class SizeType = typename XV::size_type>
struct V_Nrm1_Functor {
  typedef typename XV::execution_space execution_space;
  typedef SizeType size_type;
  typedef typename XV::non_const_value_type xvalue_type;
  typedef Kokkos::ArithTraits<xvalue_type> XAT;
  typedef typename XAT::mag_type value_type;
  typedef Kokkos::ArithTraits<value_type> MAT;

  typename XV::const_type m_x;

  V_Nrm1_Functor(const XV& x) : m_x(x) {
    static_assert(Kokkos::is_view<RV>::value,
                  "KokkosBlas::Impl::V_Nrm1_Functor: "
                  "R is not a Kokkos::View.");
    static_assert(Kokkos::is_view<XV>::value,
                  "KokkosBlas::Impl::V_Nrm1_Functor: "
                  "X is not a Kokkos::View.");
    static_assert(std::is_same<typename RV::value_type,
                               typename RV::non_const_value_type>::value,
                  "KokkosBlas::Impl::V_Nrm1_Functor: R is const.  "
                  "It must be nonconst, because it is an output argument "
                  "(we have to be able to write to its entries).");
    static_assert(RV::rank == 0 && XV::rank == 1,
                  "KokkosBlas::Impl::V_Nrm1_Functor: "
                  "RV must have rank 0 and XV must have rank 1.");
  }

  KOKKOS_INLINE_FUNCTION
  void operator()(const size_type& i, value_type& sum) const {
    xvalue_type val = m_x(i);
    sum += MAT::abs(XAT::real(val)) + MAT::abs(XAT::imag(val));
  }
};

template <class ExecSpace, class RV, class XV, class size_type>
struct Nrm1_MV_Functor {
  typedef typename RV::non_const_value_type rvalue_type;
  typedef typename XV::non_const_value_type xvalue_type;
  typedef Kokkos::ArithTraits<xvalue_type> XAT;
  typedef typename XAT::mag_type value_type;
  typedef Kokkos::ArithTraits<value_type> MAT;

  using TeamMem = typename Kokkos::TeamPolicy<ExecSpace>::member_type;

  RV r;
  XV x;

  size_type
      teamsPerVec;  // number of teams collectively performing a dot product

  Nrm1_MV_Functor(const RV& r_, const XV& x_, int teamsPerVec_)
      : r(r_), x(x_), teamsPerVec(teamsPerVec_) {}

  KOKKOS_INLINE_FUNCTION
  void operator()(const TeamMem& t) const {
    size_type globalRank = t.league_rank();
    size_type localRank  = globalRank % teamsPerVec;
    size_type i          = globalRank / teamsPerVec;
    size_type begin      = localRank * (x.extent(0) / teamsPerVec);
    size_type end        = (localRank + 1) * (x.extent(0) / teamsPerVec);
    if (localRank == teamsPerVec - 1) end = x.extent(0);
    value_type localResult = MAT::zero();
    Kokkos::parallel_reduce(
        Kokkos::TeamThreadRange(t, begin, end),
        [&](size_type k, value_type& update) {
          auto val = x(k, i);
          update += MAT::abs(XAT::real(val)) + MAT::abs(XAT::imag(val));
        },
        localResult);

    Kokkos::single(Kokkos::PerTeam(t), [&]() {
      Kokkos::atomic_add(&r(i), rvalue_type(localResult));
    });
  }
};

/// \brief Compute the 2-norm (or its square) of the single vector (1-D
///   View) X, and store the result in the 0-D View r.
template <class RV, class XV, class SizeType>
void V_Nrm1_Invoke(const RV& r, const XV& X) {
  typedef typename XV::execution_space execution_space;
  const SizeType numRows = static_cast<SizeType>(X.extent(0));
  Kokkos::RangePolicy<execution_space, SizeType> policy(0, numRows);

  typedef V_Nrm1_Functor<RV, XV, SizeType> functor_type;
  functor_type op(X);
  Kokkos::parallel_reduce("KokkosBlas1::Nrm1::S0", policy, op, r);
}

/// \brief Compute the 2-norms (or their square) of the columns of the
///   multivector (2-D View) X, and store result(s) in the 1-D View r.
// Main version: the result view is accessible from execution space, so it can
// be computed in-place
template <class RV, class XV, class size_type>
void MV_Nrm1_Invoke(
    const RV& r, const XV& x,
    typename std::enable_if<Kokkos::SpaceAccessibility<
        typename XV::execution_space,
        typename RV::memory_space>::accessible>::type* = nullptr) {
  using execution_space = typename XV::execution_space;
  if (r.extent(0) != x.extent(1)) {
    std::ostringstream oss;
    oss << "KokkosBlas::nrm1 (rank-2): result vector has wrong length ("
        << r.extent(0) << ", but x has " << x.extent(1) << " columns)";
    throw std::runtime_error(oss.str());
  }
  // Zero out the result vector
  Kokkos::deep_copy(
      r, Kokkos::ArithTraits<typename RV::non_const_value_type>::zero());
  size_type teamsPerVec;
  KokkosBlas::Impl::multipleReductionWorkDistribution<execution_space,
                                                      size_type>(
      x.extent(0), x.extent(1), teamsPerVec);
  size_type numTeams = x.extent(1) * teamsPerVec;
  Kokkos::TeamPolicy<execution_space> pol(numTeams, Kokkos::AUTO);
  Kokkos::parallel_for(
      "KokkosBlas1::Nrm1::S1", pol,
      Nrm1_MV_Functor<execution_space, RV, XV, size_type>(r, x, teamsPerVec));
}

// Version for when a temporary result view is needed (implemented in terms of
// the other version)
template <class RV, class XV, class size_type>
void MV_Nrm1_Invoke(
    const RV& r, const XV& x,
    typename std::enable_if<!Kokkos::SpaceAccessibility<
        typename XV::execution_space,
        typename RV::memory_space>::accessible>::type* = nullptr) {
  Kokkos::View<typename RV::non_const_value_type*, typename XV::memory_space>
      tempResult(
          Kokkos::view_alloc(Kokkos::WithoutInitializing, "Nrm1 temp result"),
          r.extent(0));
  MV_Nrm1_Invoke<decltype(tempResult), XV, size_type>(tempResult, x);
  Kokkos::deep_copy(r, tempResult);
}

}  // namespace Impl
}  // namespace KokkosBlas

#endif  // KOKKOSBLAS1_NRM1_IMPL_HPP_
