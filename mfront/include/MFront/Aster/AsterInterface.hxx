/*!
 * \file   AsterInterface.hxx
 * \brief  This file implements the AsterInterface class.
 * \author Helfer Thomas
 * \date   28 Jul 2006
 */

#ifndef _LIB_MFRONT_ASTER_CALL_H_
#define _LIB_MFRONT_ASTER_CALL_H_ 

#include<string>
#include<algorithm>
#include<vector>
#include<limits>

#include"TFEL/Config/TFELTypes.hxx"

#include"TFEL/Metaprogramming/IF.hxx"
#include"TFEL/Exception/TFELException.hxx"
#include"TFEL/FSAlgorithm/copy.hxx"

#include"TFEL/Utilities/Info.hxx"
#include"TFEL/Utilities/Name.hxx"

#include"TFEL/Math/stensor.hxx"

#include"TFEL/Material/MechanicalBehaviourTraits.hxx"
#include"TFEL/Material/MaterialException.hxx"
#include"TFEL/Material/ModellingHypothesis.hxx"

#include"MFront/Aster/Aster.hxx"
#include"MFront/Aster/AsterConfig.hxx"
#include"MFront/Aster/AsterTraits.hxx"
#include"MFront/Aster/AsterException.hxx"
#include"MFront/Aster/AsterInterfaceBase.hxx"

namespace aster
{

  template<unsigned short N>
  struct AsterModellingHypothesis;

  template<>
  struct AsterModellingHypothesis<1u>
  {
    typedef tfel::material::ModellingHypothesis ModellingHypothesis;
    static const ModellingHypothesis::Hypothesis value = ModellingHypothesis::AXISYMMETRICALGENERALISEDPLANESTRAIN;
  };

  template<>
  struct AsterModellingHypothesis<2u>
  {
    typedef tfel::material::ModellingHypothesis ModellingHypothesis;
    static const ModellingHypothesis::Hypothesis value = ModellingHypothesis::GENERALISEDPLANESTRAIN;
  };

  template<>
  struct AsterModellingHypothesis<3u>
  {
    typedef tfel::material::ModellingHypothesis ModellingHypothesis;
    static const ModellingHypothesis::Hypothesis value = ModellingHypothesis::TRIDIMENSIONAL;
  };

  /*!
   * forward declaration
   */
  template<unsigned short N,
	   template<tfel::material::ModellingHypothesis::Hypothesis,
		    typename,bool> class Behaviour>
  struct AsterIsotropicBehaviourHandler;

  /*!
   * forward declaration
   */
  template<unsigned short N,
	   template<tfel::material::ModellingHypothesis::Hypothesis,
		    typename,bool> class Behaviour>
  struct AsterOrthotropicBehaviourHandler;

  /*!
   * \class  AsterInterface
   * \brief This class create an interface between a behaviour class
   * and the aster finite element code
   *
   * \note : most of branching is done at compile-time. to the very
   * exeception of the stress-free expansion which are handled through
   * two functions pointers which take into account the specificities
   * of finite strain strategy used. The choice of introducing those
   * runtime-functions comes from the fact that we did not want a code
   * duplication between two different finite strain strategies (to
   * reduce both compile-time and library size).
   *
   * \author Helfer Thomas
   * \date   28 Jul 2006
   */
  template<template<tfel::material::ModellingHypothesis::Hypothesis,
		    typename,bool> class Behaviour>
  struct TFEL_VISIBILITY_LOCAL AsterInterface
    : protected AsterInterfaceBase
  {

    TFEL_ASTER_INLINE2 static
     int exe(const AsterInt  *const NTENS, const AsterReal *const DTIME,
	     const AsterReal *const DROT,  AsterReal *const DDSOE,
	     const AsterReal *const STRAN, const AsterReal *const DSTRAN,
	     const AsterReal *const TEMP,  const AsterReal *const DTEMP,
	     const AsterReal *const PROPS, const AsterInt  *const NPROPS,
	     const AsterReal *const PREDEF,const AsterReal *const DPRED,
	     AsterReal *const STATEV,const AsterInt  *const NSTATV,
	     AsterReal *const STRESS,
	     const StressFreeExpansionHandler& sfeh)
    {
      using namespace tfel::meta;
      using namespace tfel::material;
      if(*NTENS==3u){
	typedef Behaviour<AsterModellingHypothesis<1u>::value,AsterReal,false> BV;
	typedef MechanicalBehaviourTraits<BV> MTraits;
	const bool is_defined_ = MTraits::is_defined;
	typedef typename IF<is_defined_,
			    DimensionDispatch<1u>,
			    BehaviourWrapper<1u> >::type Handler;
	return Handler::exe(NTENS,DTIME,DROT,DDSOE,STRAN,DSTRAN,
			    TEMP,DTEMP,PROPS,NPROPS,PREDEF,DPRED,
			    STATEV,NSTATV,STRESS,sfeh);
      } else if(*NTENS==4){
	typedef Behaviour<AsterModellingHypothesis<2u>::value,AsterReal,false> BV;
	typedef MechanicalBehaviourTraits<BV> MTraits;
	const bool is_defined_ = MTraits::is_defined;
	typedef typename IF<is_defined_,
			    DimensionDispatch<2u>,
			    BehaviourWrapper<2u> >::type Handler;
	return Handler::exe(NTENS,DTIME,DROT,DDSOE,STRAN,DSTRAN,
			    TEMP,DTEMP,PROPS,NPROPS,PREDEF,DPRED,
			    STATEV,NSTATV,STRESS,sfeh);
      } else if(*NTENS==6){
	return DimensionDispatch<3u>::exe(NTENS,DTIME,DROT,DDSOE,STRAN,DSTRAN,
					  TEMP,DTEMP,PROPS,NPROPS,PREDEF,DPRED,
					  STATEV,NSTATV,STRESS,sfeh);
      } else {
	AsterInterfaceBase::displayInvalidNTENSValueErrorMessage();
	return -2;
      }
    }

  private:

    template<unsigned short N>
    struct BehaviourWrapper
    {
      TFEL_ASTER_INLINE2 static
      int exe(const AsterInt  *const, const AsterReal *const DTIME,
	      const AsterReal *const DROT,  AsterReal *const DDSOE,
	      const AsterReal *const STRAN, const AsterReal *const DSTRAN,
	      const AsterReal *const TEMP,  const AsterReal *const DTEMP,
	      const AsterReal *const PROPS, const AsterInt  *const NPROPS,
	      const AsterReal *const PREDEF,const AsterReal *const DPRED,
	      AsterReal *const STATEV,const AsterInt  *const NSTATV,
	      AsterReal *const STRESS,
	      const StressFreeExpansionHandler& sfeh)
      {
	using namespace tfel::meta;
	using namespace tfel::math;
	using namespace tfel::utilities;
	using namespace tfel::fsalgo;
	AsterInt NTENS   = 6u;
	AsterReal s[6u]  = {0.,0.,0.,0.,0.,0.};
	AsterReal e[6u]  = {0.,0.,0.,0.,0.,0.};
	AsterReal de[6u] = {0.,0.,0.,0.,0.,0.};
	AsterReal K[36u] = {0.,0.,0.,0.,0.,0.,
			    0.,0.,0.,0.,0.,0.,
			    0.,0.,0.,0.,0.,0.,
			    0.,0.,0.,0.,0.,0.,
			    0.,0.,0.,0.,0.,0.,
			    0.,0.,0.,0.,0.,0.};
	K[0u] = DDSOE[0];
	copy<StensorDimeToSize<N>::value>::exe(STRESS,s);
	copy<StensorDimeToSize<N>::value>::exe(STRAN,e);
	copy<StensorDimeToSize<N>::value>::exe(DSTRAN,de);
	int r = DimensionDispatch<3u>::exe(&NTENS,DTIME,DROT,K,e,de,
					   TEMP,DTEMP,PROPS,NPROPS,PREDEF,DPRED,
					   STATEV,NSTATV,s,sfeh);
	if(r==0){
	  AsterReduceTangentOperator<N>::exe(DDSOE,K);
	  copy<StensorDimeToSize<N>::value>::exe(s,STRESS);
	}
	return r;
      }
    };

    template<unsigned short N>
    struct DimensionDispatch
    {
      TFEL_ASTER_INLINE2 static
      int exe(const AsterInt  *const, const AsterReal *const DTIME,
	      const AsterReal *const DROT,  AsterReal *const DDSOE,
	      const AsterReal *const STRAN, const AsterReal *const DSTRAN,
	      const AsterReal *const TEMP,  const AsterReal *const DTEMP,
	      const AsterReal *const PROPS, const AsterInt  *const NPROPS,
	      const AsterReal *const PREDEF,const AsterReal *const DPRED,
	      AsterReal *const STATEV,const AsterInt  *const NSTATV,
	      AsterReal *const STRESS,
	      const StressFreeExpansionHandler& sfeh)
      {
	using namespace std;
	using namespace tfel::meta;
	using namespace tfel::utilities;
	typedef Behaviour<AsterModellingHypothesis<N>::value,AsterReal,false> BV;
	typedef AsterTraits<BV> Traits;
	try {
	  typedef typename IF<Traits::type==aster::ISOTROPIC,
			      AsterIsotropicBehaviourHandler<N,Behaviour>,
			      AsterOrthotropicBehaviourHandler<N,Behaviour> >::type Handler;
	  Handler::exe(DTIME,DROT,DDSOE,STRAN,DSTRAN,TEMP,DTEMP,
		       PROPS,NPROPS,PREDEF,DPRED,STATEV,NSTATV,
		       STRESS,sfeh);
	} 
	catch(const AsterException& e){
	  if(Traits::errorReportPolicy!=ASTER_NOERRORREPORT){
	    AsterInterfaceBase::treatAsterException(Name<BV>::getName(),e);
	  }
	  return -2;
	}
	catch(const tfel::material::MaterialException& e){
	  if(Traits::errorReportPolicy!=ASTER_NOERRORREPORT){
	    AsterInterfaceBase::treatMaterialException(Name<BV>::getName(),e);
	  }
	  return -3;
	}
	catch(const tfel::exception::TFELException& e){
	  if(Traits::errorReportPolicy!=ASTER_NOERRORREPORT){
	    AsterInterfaceBase::treatTFELException(Name<BV>::getName(),e);
	  }
	  return -4;
	}
	catch(const std::exception& e){
	  if(Traits::errorReportPolicy!=ASTER_NOERRORREPORT){
	    AsterInterfaceBase::treatStandardException(Name<BV>::getName(),e);
	  }
	  return -5;
	}
	catch(...){
	  if(Traits::errorReportPolicy!=ASTER_NOERRORREPORT){
	    AsterInterfaceBase::treatUnknownException(Name<BV>::getName());
	  }
	  return -6;
	}
	return 0;
      } // end of DimensionDispatch::exe
    }; // end of struct DimensionDispatch
  }; // end of struct AsterInterface
  
} // end of namespace aster

#include"MFront/Aster/AsterBehaviourHandler.hxx"

#endif /* _LIB_MFRONT_ASTER_CALL_H */

