/*!
 * \file   mfront/src/AsterComputeStiffnessTensor.cxx
 * \brief
 * \author Thomas Helfer
 * \date   18 mar 2008
 * \copyright Copyright (C) 2006-2018 CEA/DEN, EDF R&D. All rights
 * reserved.
 * This project is publicly released under either the GNU GPL Licence
 * or the CECILL-A licence. A copy of thoses licences are delivered
 * with the sources of TFEL. CEA or EDF may also distribute this
 * project under specific licensing conditions.
 */

#include <algorithm>

#include "TFEL/Math/st2tost2.hxx"
#include "TFEL/Material/StiffnessTensor.hxx"
#include "MFront/Aster/AsterComputeStiffnessTensor.hxx"

namespace aster {

  void AsterComputeIsotropicStiffnessTensor2D(
      tfel::config::Types<2u, AsterReal, false>::StiffnessTensor& C,
      const AsterReal* const props) {
    using namespace tfel::material;
    using STAC = StiffnessTensorAlterationCharacteristic;
    computeIsotropicStiffnessTensorII<2u, STAC::UNALTERED>(C, props[0],
                                                           props[1]);
  }

  void AsterComputeIsotropicPlaneStressAlteredStiffnessTensor(
      tfel::config::Types<2u, AsterReal, false>::StiffnessTensor& C,
      const AsterReal* const props)

  {
    using namespace tfel::material;
    using STAC = StiffnessTensorAlterationCharacteristic;
    computeIsotropicStiffnessTensorII<2u, STAC::ALTERED>(C, props[0], props[1]);
  }  // end of struct AsterComputeIsotropicPlaneStressAlteredStiffnessTensor

  void AsterComputeIsotropicStiffnessTensor3D(
      tfel::config::Types<3u, AsterReal, false>::StiffnessTensor& C,
      const AsterReal* const props) {
    using namespace tfel::material;
    using STAC = StiffnessTensorAlterationCharacteristic;
    computeIsotropicStiffnessTensorII<3u, STAC::UNALTERED>(C, props[0],
                                                           props[1]);
  }  // end of struct AsterComputeStiffnessTensor

  void AsterComputeOrthotropicStiffnessTensor2D(
      tfel::config::Types<2u, AsterReal, false>::StiffnessTensor& C,
      const AsterReal* const props) {
    using namespace tfel::material;
    using STAC = StiffnessTensorAlterationCharacteristic;
    computeOrthotropicStiffnessTensorII<2u, STAC::UNALTERED>(
        C, props[0], props[1], props[2], props[3], props[4], props[5], props[6],
        props[6], props[6]);
  }  // end of struct AsterComputeStiffnessTensor

  void AsterComputeOrthotropicPlaneStressAlteredStiffnessTensor(
      tfel::config::Types<2u, AsterReal, false>::StiffnessTensor& C,
      const AsterReal* const props) {
    using namespace tfel::material;
    using STAC = StiffnessTensorAlterationCharacteristic;
    computeOrthotropicStiffnessTensorII<2u, STAC::ALTERED>(
        C, props[0], props[1], props[2], props[3], props[4], props[5], props[6],
        props[6], props[6]);
  }  // end of struct AsterComputeOrthotropicPlaneStressAlteredStiffnessTensor

  void AsterComputeOrthotropicStiffnessTensor3D(
      tfel::config::Types<3u, AsterReal, false>::StiffnessTensor& C,
      const AsterReal* const props) {
    using namespace tfel::material;
    using STAC = StiffnessTensorAlterationCharacteristic;
    computeOrthotropicStiffnessTensorII<3u, STAC::UNALTERED>(
        C, props[0], props[1], props[2], props[3], props[4], props[5], props[6],
        props[7], props[8]);
  }  // end of struct AsterComputeStiffnessTensor

}  // end of namespace aster
