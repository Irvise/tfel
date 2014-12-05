/*! 
 * \file  include/TFEL/Math/Parser/ConstantExternalFunction.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 18 mars 2013
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef _LIB_TFEL_MATH_PARSER_CONSTANTEXTERNALFUNCTION_H_
#define _LIB_TFEL_MATH_PARSER_CONSTANTEXTERNALFUNCTION_H_ 

#include<sstream>
#include<vector>
#include<stdexcept>

#include"TFEL/Config/TFELConfig.hxx"
#include"TFEL/Math/Parser/ExternalFunction.hxx"

namespace tfel
{

  namespace math
  {

    namespace parser
    {

      struct TFELMATHPARSER_VISIBILITY_EXPORT ConstantExternalFunction
	: public ExternalFunction
      {
	ConstantExternalFunction(const double);
      	virtual double
	getValue(void) const;
	virtual void
	setVariableValue(const std::vector<double>::size_type,
			 const double);
	virtual std::vector<double>::size_type
	getNumberOfVariables(void) const;
	virtual void
	checkCyclicDependency(const std::string&) const;
	virtual void
	checkCyclicDependency(std::vector<std::string>&) const;
	virtual std::shared_ptr<ExternalFunction>
	differentiate(const std::vector<double>::size_type) const;
	virtual	std::shared_ptr<ExternalFunction>
	differentiate(const std::string&) const;
	virtual	std::shared_ptr<ExternalFunction>
	resolveDependencies(void) const;
	virtual std::shared_ptr<ExternalFunction>
	createFunctionByChangingParametersIntoVariables(const std::vector<std::string>&) const;
	virtual std::shared_ptr<ExternalFunction>
	createFunctionByChangingParametersIntoVariables(std::vector<std::string>&,
							const std::vector<double>&,
							const std::vector<std::string>&,
							const std::map<std::string,
							std::vector<double>::size_type>&) const;
	virtual void
	getParametersNames(std::set<std::string>&) const;
	virtual ~ConstantExternalFunction();
      protected:
	const double value;
      }; // end of struct ConstantExternalFunction
      
    } // end of namespace parser

  } // end of namespace math

} // end of namespace tfel

#endif /* _LIB_TFEL_MATH_PARSER_CONSTANTEXTERNALFUNCTION_H */

