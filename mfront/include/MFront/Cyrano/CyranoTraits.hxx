/*!
 * \file   mfront/include/MFront/Cyrano/CyranoTraits.hxx
 * \brief  
 * 
 * \author Helfer Thomas
 * \date   21 f�v 2014
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef _LIB_MFRONT_CYRANOTRAITS_HXX_
#define _LIB_MFRONT_CYRANOTRAITS_HXX_ 

namespace cyrano{

  enum CyranoSymmetryType{
    ISOTROPIC,
    ORTHOTROPIC
  }; // end of enum CyranoSymmetryType

  template<typename Behaviour>
  struct CyranoTraits{
    static constexpr bool useTimeSubStepping = false;
    static constexpr bool doSubSteppingOnInvalidResults  = false;
    static constexpr unsigned short maximumSubStepping   = 0u;
    static constexpr bool requiresStiffnessTensor        = false;
    static constexpr bool requiresThermalExpansionCoefficientTensor = false;
    static constexpr unsigned short propertiesOffset     = 0u;
    static constexpr CyranoSymmetryType stype = cyrano::ISOTROPIC;
  }; // end of class CyranoTraits

} // end of namespace cyrano  

#endif /* _LIB_MFRONT_CYRANOTRAITS_HXX */

