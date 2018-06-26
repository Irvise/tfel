/*!
 * \file   include/TFEL/Material/PiPlane.ixx
 * \brief
 * \author Thomas Helfer
 * \date   14/12/2017
 * \copyright Copyright (C) 2006-2018 CEA/DEN, EDF R&D. All rights
 * reserved.
 * This project is publicly released under either the GNU GPL Licence
 * or the CECILL-A licence. A copy of thoses licences are delivered
 * with the sources of TFEL. CEA or EDF may also distribute this
 * project under specific licensing conditions.
 */

#ifndef LIB_TFEL_MATERIAL_PIPLANE_IXX
#define LIB_TFEL_MATERIAL_PIPLANE_IXX

namespace tfel {

  namespace material {

    template <typename stress>
    std::tuple<stress, stress> projectOnPiPlane(
        const tfel::math::tvector<3u, stress>& s) {
      using namespace tfel::math;
      using real = tfel::typetraits::base_type<stress>;
      constexpr const auto zero = stress{0};
      constexpr const auto isqrt2 = Cste<stress>::isqrt2;
      constexpr const auto isqrt3 = Cste<stress>::isqrt3;
      constexpr const auto isqrt6 = isqrt2 * isqrt3;
      const auto n0 = tvector<3u, real>{{isqrt2, -isqrt2, zero}};
      const auto n1 = tvector<3u, real>{{-isqrt6, -isqrt6, 2 * isqrt6}};
      return std::tuple<stress, stress>{(s | n0), (s | n1)};
    }  // end of projectOnPiPlane

    template <typename stress>
    std::tuple<stress, stress> projectOnPiPlane(const stress s0,
                                                const stress s1,
                                                const stress s2) {
      const auto vp = tfel::math::tvector<3u, stress>{{s0, s1, s2}};
      return projectOnPiPlane(vp);
    }  // end of projectOnPiPlane

    template <typename StressStensor,
              tfel::math::stensor_common::EigenSolver es>
    std::tuple<tfel::math::StensorNumType<StressStensor>,
               tfel::math::StensorNumType<StressStensor>>
    projectOnPiPlane(const StressStensor& s) {
      const auto vp = s.template computeEigenValues<es>();
      return projectOnPiPlane(vp);
    }  // end of projectOnPiPlane

    template <typename stress>
    std::tuple<stress, stress, stress> buildFromPiPlane(const stress s0,
                                                        const stress s1) {
      using namespace tfel::math;
      using real = tfel::typetraits::base_type<stress>;
      constexpr const auto zero = stress{0};
      constexpr const auto isqrt2 = Cste<stress>::isqrt2;
      constexpr const auto isqrt3 = Cste<stress>::isqrt3;
      constexpr const auto isqrt6 = isqrt2 * isqrt3;
      const auto n0 = tvector<3u, real>{{isqrt2, -isqrt2, zero}};
      const auto n1 = tvector<3u, real>{{-isqrt6, -isqrt6, 2 * isqrt6}};
      const auto s = s0 * n0 + s1 * n1;
      return std::tuple<stress, stress, stress>{s[0], s[1], s[2]};
    }  // end of buildFromPiPlane

  }  // end of namespace material

}  // end of namespace tfel

#endif /* LIB_TFEL_MATERIAL_PIPLANE_IXX */
