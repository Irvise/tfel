/*!
 * \file   include/TFEL/Math/Parser/ExternalFunctionManager.hxx
 * \brief  
 * 
 * \author Helfer Thomas
 * \date   02 oct 2007
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef _LIB_TFEL_MATH_PARSER_EXTERNALFUNCTIONMANAGER_HXX_
#define _LIB_TFEL_MATH_PARSER_EXTERNALFUNCTIONMANAGER_HXX_ 

#include<string>
#include<map>

#include<memory>
#include"TFEL/Math/Parser/ExternalFunction.hxx"

namespace tfel
{

  namespace math
  {

    namespace parser
    {

      struct ExternalFunctionManager
	: public std::map<std::string,
			  std::shared_ptr<ExternalFunction> >
      {}; // end of struct ExternalFunctionManager
      
    } // end of namespace parser

  } // end of namespace math
  
} // end of namespace tfel

#endif /* _LIB_TFEL_MATH_PARSER_EXTERNALFUNCTIONMANAGER_HXX */

