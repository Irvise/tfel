/*! 
 * \file   mfront/mtest/AbaqusSmallStrainBehaviour.cxx
 * \brief
 * \author Helfer Thomas
 * \brief  07 avril 2013
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#include<cmath>
#include<algorithm>

#include"TFEL/Math/tmatrix.hxx"
#include"TFEL/Math/stensor.hxx"
#include"TFEL/Math/st2tost2.hxx"
#include"TFEL/System/ExternalLibraryManager.hxx"
#include"MFront/Abaqus/Abaqus.hxx"
#include"MFront/Abaqus/AbaqusComputeStiffnessTensor.hxx"

#include"MTest/UmatNormaliseTangentOperator.hxx"
#include"MTest/AbaqusSmallStrainBehaviour.hxx"

namespace mtest
{

  AbaqusSmallStrainBehaviour::AbaqusSmallStrainBehaviour(const tfel::material::ModellingHypothesis::Hypothesis h,
								 const std::string& l,
								 const std::string& b)
    : AbaqusStandardBehaviour(h,l,b)
  {}

  void
  AbaqusSmallStrainBehaviour::getDrivingVariablesDefaultInitialValues(tfel::math::vector<real>& v) const
  {
    std::fill(v.begin(),v.end(),real(0));
  }

  bool
  AbaqusSmallStrainBehaviour::call_behaviour(tfel::math::matrix<real>& Kt,
					    tfel::math::vector<real>& s1,
					    tfel::math::vector<real>& iv1,
					    const tfel::math::tmatrix<3u,3u,real>& r,
					    const tfel::math::vector<real>& e0,
					    const tfel::math::vector<real>& e1,
					    const tfel::math::vector<real>& s0,
					    const tfel::math::vector<real>& mp0,
					    const tfel::math::vector<real>& iv0,
					    const tfel::math::vector<real>& ev0,
					    const tfel::math::vector<real>& dev,
					    const tfel::material::ModellingHypothesis::Hypothesis h,
					    const real dt,
					    const StiffnessMatrixType::mtype ktype,
					    const bool b) const
  {
    using namespace std;
    using namespace tfel::math;
    using namespace abaqus;
    typedef tfel::material::ModellingHypothesis MH;
    using tfel::math::vector;
    static const real sqrt2 = std::sqrt(real(2));
    if(ktype!=StiffnessMatrixType::CONSISTENTTANGENTOPERATOR){
      throw(std::runtime_error("AbaqusSmallStrainBehaviour::call_beahviour : "
			       "abaqus behaviours may only provide the "
			       "consistent tangent operator"));
    }
    const AbaqusInt nprops = mp0.size() == 0 ? 1 : static_cast<AbaqusInt>(mp0.size());
    const AbaqusInt ntens = [&h](){
      if (h==MH::AXISYMMETRICAL){
	return 4;
      } else if (h==MH::PLANESTRESS){
	return 3;
      } else if (h==MH::TRIDIMENSIONAL){
	return 6;
      } 
      throw(std::runtime_error("AbaqusSmallStrainBehaviour::call_beahviour : "
			       "unsupported hypothesis"));      
    }();
    fill(this->D.begin(),this->D.end(),0.);
    // using a local copy of material properties to handle the
    // case where mp0 is empty
    copy(mp0.begin(),mp0.end(),this->mps.begin());
    if(mp0.empty()){
      this->mps[0] = real(0);
    }
    // using a local copy of internal state variables to handle the
    // case where iv0 is empty
    copy(iv0.begin(),iv0.end(),this->ivs.begin());
    if(iv0.empty()){
      this->ivs[0] = real(0);
    }
    const auto nstatv = static_cast<AbaqusInt>(this->ivs.size());
    // rotation matrix, set to identity
    const tmatrix<3u,3u,real> drot = {1,0,0,
				      0,1,0,
				      0,0,1};
    stensor<3u,real> ue0(real(0));
    stensor<3u,real> ude(real(0));
    stensor<3u,real> us(real(0));
    copy(e0.begin(),e0.end(),ue0.begin());
    copy(s0.begin(),s0.end(),us.begin());
    for(AbaqusInt i=0;i!=e1.size();++i){
      ude(i) = e1(i)-e0(i);
    }
    ue0.changeBasis(r);
    ude.changeBasis(r);
    us.changeBasis(r);
    // abaqus standard convention
    for(AbaqusInt i=3;i!=e1.size();++i){
      ue0(i) *= sqrt2;
      ude(i) *= sqrt2;
    }
    if (h==MH::PLANESTRESS){
      us[2] = us[3]/sqrt2;
      us[3] = 0;
    } else {
      for(AbaqusInt i=3;i!=static_cast<unsigned short>(ntens);++i){
	us(i) /= sqrt2;
      }
    }
    AbaqusReal ndt(1.);
    (this->fct)(&us(0),&this->ivs(0),&D(0,0),
		nullptr,nullptr,nullptr,nullptr,
		nullptr,nullptr,nullptr,
		&ue0(0),&ude(0),nullptr,&dt,
		&ev0(0),&dev(0),
		&ev0(0)+1,&dev(0)+1,
		nullptr,nullptr,nullptr,&ntens,&nstatv,&mps(0),
		&nprops,nullptr,&drot(0,0),&ndt,
		nullptr,nullptr,nullptr,nullptr,nullptr,
		nullptr,nullptr,nullptr,nullptr,0);
    if(ndt<1.){
      return false;
    }
    const auto rb = transpose(r);
    // treating the consistent tangent operator
    if(h==MH::TRIDIMENSIONAL){
      UmatNormaliseTangentOperator::exe(Kt,D,3u);
      st2tost2<3u,AbaqusReal> K;
      for(unsigned short i=0;i!=6u;++i){
	for(unsigned short j=0;j!=6u;++j){
	  K(i,j)=Kt(i,j);
	}
      }
      const auto nK = change_basis(K,rb);
      for(unsigned short i=0;i!=6u;++i){
	for(unsigned short j=0;j!=6u;++j){
	  Kt(i,j)=nK(i,j);
	}
      }
    } else if (h==MH::AXISYMMETRICAL){
      UmatNormaliseTangentOperator::exe(Kt,D,2u);
      st2tost2<2u,AbaqusReal> K;
      for(unsigned short i=0;i!=4u;++i){
	for(unsigned short j=0;j!=4u;++j){
	  K(i,j)=Kt(i,j);
	}
      }
      const auto nK = change_basis(K,rb);
      for(unsigned short i=0;i!=4u;++i){
	for(unsigned short j=0;j!=4u;++j){
	  Kt(i,j)=nK(i,j);
	}
      }
    } else if (h==MH::PLANESTRESS){
      constexpr const auto zero = AbaqusReal{0};
      // D has been as a 3*3 fortran matrix. The terms associated with
      // the 2 indices are omitted.
      // D = D00 D10 D30 D01 D11 D31 D03 D13 D33
      double D2[9u];
      std::copy(D.begin(),D.begin()+9,D2);
      // Let us add the missing term
      // We want D00 D10 D20 D30 D01 D11 D21 D31 D02 D12 D22 D32 D03 D13 D23 D33 
      auto p = D.begin();
      // D00 D10 D20 D30
      *p     = D2[0];
      *(p+1) = D2[1];
      *(p+2) = zero;
      *(p+3) = D2[2];
      // D01 D11 D21 D31
      *(p+4) = D2[3];
      *(p+5) = D2[4];
      *(p+6) = zero;
      *(p+7) = D2[5];
      // D02 D12 D22 D32
      *(p+8)  = zero;
      *(p+9)  = zero;
      *(p+10) = zero;
      *(p+11) = zero;
      // D03 D13 D23 D33
      *(p+12) = D2[6];
      *(p+13) = D2[7];
      *(p+14) = zero;
      *(p+15) = D2[8];
      // so now we have D in a conventional fortan form, so we can
      // normalise it (transpose and TFEL storage conventions !)
      UmatNormaliseTangentOperator::exe(Kt,D,2u);
      // the last step: rotation in the global frame
      st2tost2<2u,AbaqusReal> K;
      for(unsigned short i=0;i!=4u;++i){
	for(unsigned short j=0;j!=4u;++j){
	  K(i,j)=Kt(i,j);
	}
      }
      const auto nK = change_basis(K,rb);
      for(unsigned short i=0;i!=4u;++i){
	for(unsigned short j=0;j!=4u;++j){
	  Kt(i,j)=nK(i,j);
	}
      }
    } else {
      throw(std::runtime_error("AbaqusSmallStrainBehaviour::"
			       "call_behaviour: normalise, "
			       "unsupported modelling hypothesis"));
    }
    if(b){
      // treating internal state variables
      if(!iv0.empty()){
	copy_n(this->ivs.begin(), iv1.size(),iv1.begin());
      }
      // treating stresses
      if (h==MH::PLANESTRESS){
	us[3] = us[2]*sqrt2;
	us[2] = real(0);
      } else {
	// turning stresses in TFEL conventions
	for(AbaqusInt i=3;i!=static_cast<unsigned short>(ntens);++i){
	  us[i] *= sqrt2;
	}
      }
      us.changeBasis(rb);
      copy(us.begin(),us.begin()+s1.size(),s1.begin());
    }
    return true;
  }

  AbaqusSmallStrainBehaviour::~AbaqusSmallStrainBehaviour()
  {}
  
} // end of namespace mtest
