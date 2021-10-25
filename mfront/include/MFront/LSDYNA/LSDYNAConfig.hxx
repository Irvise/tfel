/*!
 * \file  mfront/include/MFront/LSDYNA/LSDYNAConfig.hxx
 * \brief
 * \author Thomas Helfer
 * \brief 10 janv. 2013
 * \copyright Copyright (C) 2006-2018 CEA/DEN, EDF R&D. All rights
 * reserved.
 * This project is publicly released under either the GNU GPL Licence
 * or the CECILL-A licence. A copy of thoses licences are delivered
 * with the sources of TFEL. CEA or EDF may also distribute this
 * project under specific licensing conditions.
 */

#ifndef LIB_MFRONT_LSDYNA_LSDYNACONFIG_HXX
#define LIB_MFRONT_LSDYNA_LSDYNACONFIG_HXX

#include "MFront/MFrontConfig.hxx"

#if defined _WIN32 || defined _WIN64 || defined __CYGWIN__
#if defined LSDYNAInterface_EXPORTS
#define MFRONT_LSDYNA_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#else
#ifndef TFEL_STATIC_BUILD
#define MFRONT_LSDYNA_VISIBILITY_EXPORT TFEL_VISIBILITY_IMPORT
#else
#define MFRONT_LSDYNA_VISIBILITY_EXPORT
#endif
#endif
#else
#define MFRONT_LSDYNA_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#endif /* LIB_MFRONT_LSDYNA_LSDYNACONFIG_HXX */

#endif /* LIB_MFRONT_LSDYNA_LSDYNACONFIG_HXX */
