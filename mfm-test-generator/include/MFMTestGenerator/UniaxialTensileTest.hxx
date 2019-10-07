/*!
 * \file   UniaxialTensileTest.hxx
 * \brief
 * \author Thomas Helfer
 * \date   17/09/2019
 * \copyright Copyright (C) 2006-2018 CEA/DEN, EDF R&D. All rights
 * reserved.
 * This project is publicly released under either the GNU GPL Licence
 * or the CECILL-A licence. A copy of thoses licences are delivered
 * with the sources of TFEL. CEA or EDF may also distribute this
 * project under specific licensing conditions.
 */

#ifndef LIB_MFM_TEST_GENERATOR_UNIAXIALTENSILETEST_HXX
#define LIB_MFM_TEST_GENERATOR_UNIAXIALTENSILETEST_HXX

#include <map>
#include <vector>
#include <string>
#include "MFMTestGenerator/Config.hxx"
#include "MFMTestGenerator/Forward.hxx"
#include "MFMTestGenerator/TestCaseBase.hxx"
#include "MFMTestGenerator/BehaviourData.hxx"

namespace mfmtg {

  //! \brief a test on a unit cube
  struct MFMTG_VISIBILITY_EXPORT UniaxialTensileTest : TestCaseBase,
                                                       BehaviourData {
    //! \brief default constructor
    UniaxialTensileTest(const TestCaseParameters&);
    //! \brief imposed strain
    const std::map<double, double> imposed_strain;
    //! \brief destructor
    ~UniaxialTensileTest() override;
  };  // end of struct UniaxialTensileTest

}  // end of namespace mfmtg

#endif /* LIB_MFM_TEST_GENERATOR_UNIAXIALTENSILETEST_HXX */
