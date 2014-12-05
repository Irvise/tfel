/*! 
 * \file  src/Math/LevenbergMarquardtEvaluatorWrapper.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 24 f�v 2010
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#include<string>
#include<stdexcept>

#include"TFEL/Math/LevenbergMarquardt/LevenbergMarquardtEvaluatorWrapper.hxx"

namespace tfel
{

  namespace math
  {
    
    LevenbergMarquardtEvaluatorWrapper::LevenbergMarquardtEvaluatorWrapper(std::shared_ptr<tfel::math::Evaluator> ev_,
									   const LevenbergMarquardtEvaluatorWrapper::size_type nv_,
									   const LevenbergMarquardtEvaluatorWrapper::size_type np_)
      : ev(ev_),
	dev(np_),
	nv(nv_),
	np(np_)
    {
      using namespace std;
      using namespace tfel::math;
      using tfel::math::vector;
      vector<double>::size_type i;
      if(this->ev->getNumberOfVariables()!=this->getNumberOfVariables()+this->getNumberOfParameters()){
	string msg("LevenbergMarquardtEvaluatorWrapper::LevenbergMarquardtEvaluatorWrapper : ");
	msg += "the number of variables declared in the specified evaluator is not equal ";
	msg += "to the sum of the number of variables and the number of paramters";
	throw(runtime_error(msg));
      }
      if(this->getNumberOfVariables()==0){
	string msg("LevenbergMarquardtEvaluatorWrapper::LevenbergMarquardtEvaluatorWrapper : ");
	msg += "null variable number";
	throw(runtime_error(msg));
      }
      if(this->getNumberOfParameters()==0){
	string msg("LevenbergMarquardtEvaluatorWrapper::LevenbergMarquardtEvaluatorWrapper : ");
	msg += "null parameters number";
	throw(runtime_error(msg));
      }
      for(i=0;i!=this->getNumberOfParameters();++i){
	dev[i]=this->ev->differentiate(this->getNumberOfVariables()+i);
      }
    } // end of LevenbergMarquardtEvaluatorWrapper::LevenbergMarquardtEvaluatorWrapper

    LevenbergMarquardtEvaluatorWrapper::size_type
    LevenbergMarquardtEvaluatorWrapper::getNumberOfVariables(void) const
    {
      return this->nv;
    } // end of LevenbergMarquardtEvaluatorWrapper::getNumberOfVariables()

    LevenbergMarquardtEvaluatorWrapper::size_type
    LevenbergMarquardtEvaluatorWrapper::getNumberOfParameters(void) const
    {
      return this->np;
    } // end of LevenbergMarquardtEvaluatorWrapper::getNumberOfParameters()

    void
    LevenbergMarquardtEvaluatorWrapper::operator()(double& r,
						   tfel::math::vector<double>& g,
						   const tfel::math::vector<double>& variables,
						   const tfel::math::vector<double>& params)
    {
      using namespace std;
      using namespace tfel::math;
      using namespace tfel::math::parser;
      using tfel::math::vector;
      vector<double>::const_iterator p;
      vector<shared_ptr<ExternalFunction> >::iterator pdev;
      vector<double>::size_type i;
      if(variables.size()!=this->getNumberOfVariables()){
	string msg("LevenbergMarquardtEvaluatorWrapper::operator() : ");
	msg += "invalid number of variables";
	throw(runtime_error(msg));
	
      }
      if(params.size()!=this->getNumberOfParameters()){
	string msg("LevenbergMarquardtEvaluatorWrapper::operator() : ");
	msg += "invalid number of parameters";
	throw(runtime_error(msg));
	
      }
      i=0;
      g.resize(this->getNumberOfParameters());
      for(p=variables.begin();p!=variables.end();++p,++i){
	this->ev->setVariableValue(i,*p);
	for(pdev=dev.begin();pdev!=dev.end();++pdev){
	  (*pdev)->setVariableValue(i,*p);
	}
      }
      for(p=params.begin();p!=params.end();++p,++i){
	this->ev->setVariableValue(i,*p);
	for(pdev=dev.begin();pdev!=dev.end();++pdev){
	  (*pdev)->setVariableValue(i,*p);
	}
      }
      r = this->ev->getValue();
      for(i=0;i!=dev.size();++i){
	g[i] = dev[i]->getValue();
      }
    } // end of LevenbergMarquardtEvaluatorWrapper::operator()

  } // end of namespace math

} // end of namespace tfel
