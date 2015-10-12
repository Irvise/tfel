/*! 
 * \file  mfront/src/UMATInterfaceBase.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 10 juil. 2013
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#include<sstream>
#include<fstream>
#include<stdexcept>
#include<algorithm>

#include"TFEL/System/System.hxx"
#include"TFEL/Utilities/StringAlgorithms.hxx"

#include"MFront/DSLUtilities.hxx"
#include"MFront/MFrontLogStream.hxx"
#include"MFront/FileDescription.hxx"
#include"MFront/UMATInterfaceBase.hxx"

namespace mfront
{

  UMATInterfaceBase::UMATMaterialProperty::UMATMaterialProperty(const std::string& t,
								const std::string& n,
								const std::string& v,
								const unsigned short a,
								const SupportedTypes::TypeSize o,
								const bool d)
    : type(t),
      name(n),
      var_name(v),
      arraySize(a),
      offset(o),
      dummy(d)
  {} // end olf UMATMaterialProperty::UMATMaterialProperty
  
  UMATInterfaceBase::UMATMaterialProperty::~UMATMaterialProperty() = default;

  const UMATInterfaceBase::UMATMaterialProperty&
  UMATInterfaceBase::findUMATMaterialProperty(const std::vector<UMATMaterialProperty>& mprops,
						    const std::string& n)
  {
    for(const auto& m : mprops){
      if((m.name==n)&&(!m.dummy)){
	return m;
      }
    }
    throw(std::runtime_error("UMATInterfaceBase::findUMATMaterialProperty : "
			     "no material property associated with the "
			     "glossary name '"+n+"'"));
  } // end of UMATInterfaceBase::findUMATMaterialProperty


  UMATInterfaceBase::UMATInterfaceBase()
    : generateMTestFile(false)
  {} // end of UMATInterfaceBase::UMATInterfaceBase

  bool
  UMATInterfaceBase::isModellingHypothesisHandled(const Hypothesis h,
							const BehaviourDescription& mb) const
  {
    const auto ih = this->getModellingHypothesesToBeTreated(mb);
    if(h==ModellingHypothesis::UNDEFINEDHYPOTHESIS){
      return !mb.areAllMechanicalDataSpecialised(ih);
    }
    return ih.find(h)!=ih.end();
  }

  std::string
  UMATInterfaceBase::getSymbolName(const std::string& n,
					 const Hypothesis h) const
  {
    if(h!=ModellingHypothesis::UNDEFINEDHYPOTHESIS){
      return this->getFunctionName(n)+"_"+ModellingHypothesis::toString(h);
    }
    return this->getFunctionName(n);
  } // end of UMATInterfaceBase::getLibraryName

  void
  UMATInterfaceBase::allowDynamicallyAllocatedArrays(const bool b)
  {
    this->areDynamicallyAllocatedVectorsAllowed_ = b;
  } // end of UMATInterfaceBase::allowDynamicallyAllocatedArrays

  void
  UMATInterfaceBase::appendToMaterialPropertiesList(std::vector<UMATMaterialProperty>& l,
						    const std::string& t,
						    const std::string& n,
						    const std::string& v,
						    const bool b) const
  {
    using namespace std;
    const auto flag = this->getTypeFlag(t);
    if(flag!=SupportedTypes::Scalar){
      string msg("UMATMaterialProperty::UMATMaterialProperty : "
		 "material properties shall be scalars");
      throw(runtime_error(msg));
    }
    SupportedTypes::TypeSize o;
    if(!l.empty()){
      const auto& m = l.back();
      o  = m.offset;
      o += this->getTypeSize(t,1u);
    }
    l.push_back(UMATMaterialProperty(t,n,v,1u,o,b));
  } // end of UMATInterfaceBase::appendToMaterialPropertiesList

  void
  UMATInterfaceBase::completeMaterialPropertiesList(std::vector<UMATMaterialProperty>& mprops,
							  const BehaviourDescription& mb,
							  const Hypothesis h) const
  {
    using namespace std;
    const auto& d = mb.getBehaviourData(h);
    const auto& mp = d.getMaterialProperties();
    for(auto p=mp.begin();p!=mp.end();++p){
      const auto& n = mb.getExternalName(h,p->name);
      bool found = false;
      const auto flag = this->getTypeFlag(p->type);
      if(flag!=SupportedTypes::Scalar){
	throw(runtime_error("UMATMaterialProperty::UMATMaterialProperty : "
			    "Invalid type for material property '"+p->name+"' ("+p->type+").\n"
			    "Material properties shall be scalars"));
      }
      for(auto pum=mprops.begin();(pum!=mprops.end())&&(!found);++pum){
	if(!pum->dummy){
	  if(pum->name==n){
	    // type check
	    if(mb.useQt()){
	      if(p->type!=pum->type){
		string msg("UMATInterfaceBase::completeMaterialPropertiesList : "
			   "incompatible type for variable '"+n+
			   "' ('"+p->type+"' vs '"+pum->type+"')");
		throw(runtime_error(msg));
	      }
	    } else {
	      // don't use quantity
	      if(this->getTypeFlag(p->type)!=this->getTypeFlag(pum->type)){;
		string msg("UMATInterfaceBase::completeMaterialPropertiesList : "
			   "incompatible type for variable '"+n+
			   "' ('"+p->type+"' vs '"+pum->type+"')");
		throw(runtime_error(msg));
	      }
	      if(p->type!=pum->type){
		auto& log = getLogStream();
		log << "UMATInterfaceBase::completeMaterialPropertiesList : "
		    << "inconsistent type for variable '" << n
		    << "' ('" << p->type << "' vs '" << pum->type << "')" << endl;
	      }
	    }
	    if(p->arraySize!=pum->arraySize){
	      string msg("UMATInterfaceBase::completeMaterialPropertiesList : "
			 "incompatible array size for variable '"+n+
			 "' ('"+p->type+"' vs '"+pum->type+"')");
	      throw(runtime_error(msg));
	    }
	    found = true;
	  }
	}
      }
      if(!found){
	SupportedTypes::TypeSize o;
	if(!mprops.empty()){
	  const auto& m = mprops.back();
	  o  = m.offset;
	  o += this->getTypeSize(m.type,m.arraySize);
	}
	mprops.push_back(UMATMaterialProperty(p->type,n,p->name,
					      p->arraySize,o,false));
      }
    }
  } // end of UMATInterfaceBase::completeMaterialPropertiesList

  void 
  UMATInterfaceBase::exportMechanicalData(std::ofstream& behaviourDataFile,
						const Hypothesis h,
						const BehaviourDescription& mb) const
  {
    const auto& d = mb.getBehaviourData(h);
    const auto& persistentVarsHolder = d.getPersistentVariables();
    const auto iprefix = makeUpperCase(this->getInterfaceName());
    if(!persistentVarsHolder.empty()){
      behaviourDataFile << "void\n"
			<< iprefix+"exportStateData("
			<< "Type * const "+iprefix+"stress_,Type * const "+iprefix+"statev) const\n";
    } else {
      behaviourDataFile << "void\n"
			<< iprefix+"exportStateData("
			<< "Type * const "+iprefix+"stress_,Type * const) const\n";
    }
    behaviourDataFile << "{\n";
    behaviourDataFile << "using namespace tfel::math;\n";
    SupportedTypes::TypeSize of;
    for(auto pm = mb.getMainVariables().begin();
	pm!=mb.getMainVariables().end();++pm){
      const auto& f = pm->second;
      const auto flag = this->getTypeFlag(f.type);
      if(flag==SupportedTypes::Scalar){
	if(pm!=mb.getMainVariables().begin()){
	  behaviourDataFile << "*("+iprefix+"stress_+" << of << ") = this->" << f.name << ";\n";
	} else {
	  behaviourDataFile << "*("+iprefix+"stress_) = this->" << f.name << ";\n";
	}
      } else if(flag==SupportedTypes::Stensor){
	if(pm!=mb.getMainVariables().begin()){
	  behaviourDataFile << "this->sig.exportTab("+iprefix+"stress_+" << of << ");\n";
	} else {
	  behaviourDataFile << "this->sig.exportTab("+iprefix+"stress_"");\n";
	}
      } else if((flag==SupportedTypes::TVector)||
		(flag==SupportedTypes::Tensor)){
	if(pm!=mb.getMainVariables().begin()){
	  behaviourDataFile << "exportToBaseTypeArray(this->" << f.name << ","+iprefix+"stress_+"
			    << of << ");\n";	
	} else {
	  behaviourDataFile << "exportToBaseTypeArray(this->" << f.name << ","+iprefix+"stress_);\n";	
	}
      } else {
	throw(std::runtime_error("UMATInterfaceBase::exportMechanicalData : "
				 "unsupported forces type"));
      }
      of += this->getTypeSize(f.type,1u);
    }
    if(!persistentVarsHolder.empty()){
      this->exportResults(behaviourDataFile,
			  persistentVarsHolder,
			  iprefix+"statev",
			  mb.useQt());
    }
    behaviourDataFile << "} // end of "+iprefix+"ExportStateData\n\n";
  }
  
  void
  UMATInterfaceBase::writeBehaviourConstructor(std::ofstream& behaviourFile,
						     const BehaviourDescription& mb,
						     const std::string& initStateVarsIncrements) const
  {
    const auto iprefix = makeUpperCase(this->getInterfaceName());
    behaviourFile << "/*\n";
    behaviourFile << " * \\brief constructor for the umat interface\n";
    behaviourFile << " *\n";
    behaviourFile << " * \\param const Type *const "+iprefix+"dt_, time increment\n";
    behaviourFile << " * \\param const Type *const "+iprefix+"T_, temperature\n";
    behaviourFile << " * \\param const Type *const "+iprefix+"dT_, temperature increment\n";
    behaviourFile << " * \\param const Type *const "+iprefix+"mat, material properties\n";
    behaviourFile << " * \\param const Type *const "+iprefix+"int_vars, state variables\n"; 
    behaviourFile << " * \\param const Type *const "+iprefix+"ext_vars, external state variables\n";
    behaviourFile << " * \\param const Type *const "+iprefix+"dext_vars,";
    behaviourFile << " external state variables increments\n";
    behaviourFile << " */\n";
    behaviourFile << mb.getClassName() 
		  << "(const Type* const "+iprefix+"dt_,\n" 
		  <<  "const Type* const "+iprefix+"T_,const Type* const "+iprefix+"dT_,\n"
		  <<  "const Type* const "+iprefix+"mat,const Type* const "+iprefix+"int_vars,\n"
		  <<  "const Type* const "+iprefix+"ext_vars,const Type* const "+iprefix+"dext_vars)\n";
    if(mb.useQt()){
      behaviourFile << ": " << mb.getClassName() 
		    << "BehaviourData<hypothesis,Type,use_qt>("+iprefix+"T_,"+iprefix+"mat,\n"
		    << iprefix+"int_vars,"+iprefix+"ext_vars),\n";
      behaviourFile << mb.getClassName() 
		    << "IntegrationData<hypothesis,Type,use_qt>("+iprefix+"dt_,"+iprefix+"dT_,"+iprefix+"dext_vars)";
    } else {
      behaviourFile << ": " << mb.getClassName() 
		    << "BehaviourData<hypothesis,Type,false>("+iprefix+"T_,"+iprefix+"mat,\n"
		    << iprefix+"int_vars,"+iprefix+"ext_vars),\n";
      behaviourFile << mb.getClassName() 
		    << "IntegrationData<hypothesis,Type,false>("+iprefix+"dt_,"+iprefix+"dT_,"+iprefix+"dext_vars)";
    }
    if(!initStateVarsIncrements.empty()){
      behaviourFile << ",\n" << initStateVarsIncrements;
    }
  }

  void
  UMATInterfaceBase::writeMaterialPropertiesInitializersInBehaviourDataConstructorI(std::ostream& f,
										    const Hypothesis h,
										    const BehaviourDescription& mb,
										    const std::vector<UMATMaterialProperty>& i,
										    const SupportedTypes::TypeSize ioffset,
										    const std::string& src,
										    const std::string& prefix,
										    const std::string& suffix) const
  {
    using namespace std;
    const auto& d = mb.getBehaviourData(h);
    const auto& v = d.getMaterialProperties();
    VariableDescriptionContainer::const_iterator p;
    if(!v.empty()){
      for(p=v.begin();p!=v.end();++p){
	if(p->arraySize==1u){
	  const string n = prefix+p->name+suffix;
	  const UMATMaterialProperty& m = 
	    UMATInterfaceBase::findUMATMaterialProperty(i,mb.getExternalName(h,p->name));
	  SupportedTypes::TypeSize offset = m.offset;
	  offset -= ioffset;
	  f << "," << endl;
	  auto flag = this->getTypeFlag(p->type);
	  if(flag==SupportedTypes::Scalar){
	    f << n << "("+src+"[" 
	      << offset << "])";  
	  } else if((flag==SupportedTypes::TVector)||
		    (flag==SupportedTypes::Stensor)||
		    (flag==SupportedTypes::Tensor)){
	    f << n << "(&"+src+"[" 
	      << offset << "])";  
	  } else {
	    string msg("SupportedTypes::");
	    msg += "writeVariableInitializersInBehaviourDataConstructorI : ";
	    msg += "internal error, tag unsupported";
	    throw(runtime_error(msg));
	  }
	}
      }
    }
    
  } // end of UMATInterfaceBase::writeVariableInitializersInBehaviourDataConstructorI

  void
  UMATInterfaceBase::writeMaterialPropertiesInitializersInBehaviourDataConstructorII(std::ostream& f,
											   const Hypothesis h,
											   const BehaviourDescription& mb,
											   const std::vector<UMATMaterialProperty>& i,
											   const SupportedTypes::TypeSize ioffset,
											   const std::string& src,
											   const std::string& prefix,
											   const std::string& suffix) const
  {
    using namespace std;
    const auto& d = mb.getBehaviourData(h);
    const auto& v = d.getMaterialProperties();
    VariableDescriptionContainer::const_iterator p;
    if(!v.empty()){
      for(p=v.begin();p!=v.end();++p){
	if(p->arraySize!=1u){
	  const UMATMaterialProperty& m =
	    UMATInterfaceBase::findUMATMaterialProperty(i,mb.getExternalName(h,p->name));	  
	  const auto flag = this->getTypeFlag(p->type);
	  SupportedTypes::TypeSize offset = m.offset;
	  offset -= ioffset;
	  const string n = prefix+p->name+suffix;
	  if(this->useDynamicallyAllocatedVector(p->arraySize)){
	    f << n << ".resize(" << p->arraySize << ");" << endl;
	    f << "for(unsigned short idx=0;idx!=" << p->arraySize << ";++idx){" << endl;
	    switch(flag){
	    case SupportedTypes::Scalar : 
	      f << n << "[idx] = "+src+"[" 
		<< offset << "+idx];" << endl;  
	      break;
	    case SupportedTypes::TVector :
	      f << "tfel::fsalgo::copy<TVectorSize>::exe(&"+src+"[" 
		<< offset << "+idx*TVectorSize],"
		<< n << "[idx].begin());" << endl;  
	      break;
	    case SupportedTypes::Stensor :
	      f << n << "[idx].import(&"+src+"[" 
		  << offset << "+idx*StensorSize]);" << endl;  
	      break;
	    case SupportedTypes::Tensor :
	      f << "tfel::fsalgo::copy<TensorSize>::exe(&"+src+"[" 
		<< offset << "+idx*TensorSize],"
		<< n << "[idx].begin());" << endl;  
	      break;
	    default : 
	      string msg("SupportedTypes::");
	      msg += "writeVariableInitializersInBehaviourDataConstructorII : ";
	      msg += "internal error, tag unsupported";
	      throw(runtime_error(msg));
	    }
	    f << "}" << endl;
	  } else {
	    for(int index=0;index!=p->arraySize;++index){
	      switch(flag){
	      case SupportedTypes::Scalar : 
		f << n << "[" << index << "] = "+src+"[" 
		  << offset << "];" << endl;  
		break;
	      case SupportedTypes::TVector :
		f << "tfel::fsalgo::copy<TVectorSize>::exe(&"+src+"[" 
		  << offset << "]," << n << "[" << index << "].begin());" << endl;  
		break;
	      case SupportedTypes::Stensor :
		f << n << "["<< index << "].import(&"+src+"[" 
		  << offset << "]);" << endl;  
		break;
	      case SupportedTypes::Tensor :
		f << "tfel::fsalgo::copy<TensorSize>::exe(&"+src+"[" 
		  << offset << "]," << n << "[" << index << "].begin());" << endl;  
		break;
	      default : 
		string msg("SupportedTypes::");
		msg += "writeVariableInitializersInBehaviourDataConstructorII : ";
		msg += "internal error, tag unsupported";
		throw(runtime_error(msg));
	      }
	      offset+=this->getTypeSize(p->type,1u);
	    }
	  }
	}
      }
    }
  } // end of UMATInterfaceBase::writeVariableInitializersInBehaviourDataConstructorII
  
  void 
  UMATInterfaceBase::writeBehaviourDataConstructor(std::ofstream& behaviourDataFile,
							 const Hypothesis h,
							 const BehaviourDescription& mb) const
  {
    using namespace std;
    const auto& d = mb.getBehaviourData(h);
    const std::string iprefix = makeUpperCase(this->getInterfaceName());
    pair<vector<UMATMaterialProperty>,
	 SupportedTypes::TypeSize> mprops = this->buildMaterialPropertiesList(mb,h);
    const auto& mp = d.getMaterialProperties();
    const auto& persistentVarsHolder = d.getPersistentVariables();
    const auto& externalStateVarsHolder = d.getExternalStateVariables();
    behaviourDataFile << "/*" << endl;
    behaviourDataFile << " * \\brief constructor for the umat interface" << endl;
    behaviourDataFile << " *" << endl;
    behaviourDataFile << " * \\param const Type *const "+iprefix+"T_, temperature" << endl;
    behaviourDataFile << " * \\param const Type *const "+iprefix+"mat, material properties" << endl;
    behaviourDataFile << " * \\param const Type *const "+iprefix+"int_vars, state variables" << endl; 
    behaviourDataFile << " * \\param const Type *const "+iprefix+"ext_vars, external state variables" << endl;
    behaviourDataFile << " */" << endl;
    behaviourDataFile << mb.getClassName() << "BehaviourData"
		      << "(const Type* const "+iprefix+"T_,const Type* const";
    if(!mp.empty()){
      behaviourDataFile << " "+iprefix+"mat," << endl;
    } else {
      behaviourDataFile << "," << endl;
    }
    behaviourDataFile <<  "const Type* const";
    if(!persistentVarsHolder.empty()){
      behaviourDataFile << " "+iprefix+"int_vars," << endl;
    } else {
      behaviourDataFile << "," << endl;
    }
    behaviourDataFile << "const Type* const";
    if(!externalStateVarsHolder.empty()){
      behaviourDataFile << " "+iprefix+"ext_vars";
    }
    behaviourDataFile << ")" << endl;
    behaviourDataFile << ": T(*"+iprefix+"T_)";
    this->writeMaterialPropertiesInitializersInBehaviourDataConstructorI(behaviourDataFile,
									 h,mb,mprops.first,
									 mprops.second,
									 iprefix+"mat","","");
    this->writeVariableInitializersInBehaviourDataConstructorI(behaviourDataFile,
							       persistentVarsHolder,
							       iprefix+"int_vars","","");
    this->writeVariableInitializersInBehaviourDataConstructorI(behaviourDataFile,
							       externalStateVarsHolder,
							       iprefix+"ext_vars","","");
    behaviourDataFile << "\n{" << endl;
    this->writeMaterialPropertiesInitializersInBehaviourDataConstructorII(behaviourDataFile,
									  h,mb,mprops.first,
									  mprops.second,
									  iprefix+"mat","","");
    this->writeVariableInitializersInBehaviourDataConstructorII(behaviourDataFile,
								persistentVarsHolder,
								iprefix+"int_vars","","");
    this->writeVariableInitializersInBehaviourDataConstructorII(behaviourDataFile,
								externalStateVarsHolder,
								iprefix+"ext_vars","","");
    behaviourDataFile << "}" << endl << endl;
  }
  
  void 
  UMATInterfaceBase::writeIntegrationDataConstructor(std::ofstream& behaviourIntegrationFile,
							   const Hypothesis h,
							   const BehaviourDescription& mb) const
  {
    using namespace std;
    const auto& d = mb.getBehaviourData(h);
    const std::string iprefix = makeUpperCase(this->getInterfaceName());
    const auto& externalStateVarsHolder = d.getExternalStateVariables();
    behaviourIntegrationFile << "/*" << endl;
    behaviourIntegrationFile << " * \\brief constructor for the umat interface" << endl;
    behaviourIntegrationFile << " * \\param const Type *const "+iprefix+"dt_, time increment" << endl;
    behaviourIntegrationFile << " * \\param const Type *const "+iprefix+"dT_, temperature increment" << endl;
    behaviourIntegrationFile << " * \\param const Type *const "+iprefix+"dext_vars,";
    behaviourIntegrationFile << " external state variables increments" << endl;
    behaviourIntegrationFile << " *" << endl;
    behaviourIntegrationFile << " */" << endl;
    behaviourIntegrationFile << mb.getClassName() << "IntegrationData"
			     << "(const Type* const "+iprefix+"dt_," << endl 
			     <<  "const Type* const "+iprefix+"dT_,const Type* const";
    if(!externalStateVarsHolder.empty()){
      behaviourIntegrationFile << " "+iprefix+"dext_vars)" << endl;
    } else {
      behaviourIntegrationFile << ")" << endl;
    }
    behaviourIntegrationFile << ": dt(*"+iprefix+"dt_),dT(*"+iprefix+"dT_)";
    if(!externalStateVarsHolder.empty()){
      this->writeVariableInitializersInBehaviourDataConstructorI(behaviourIntegrationFile,
								 externalStateVarsHolder,
								 iprefix+"dext_vars","d","");
    }
    behaviourIntegrationFile << "\n{" << endl;
    if(!externalStateVarsHolder.empty()){
      this->writeVariableInitializersInBehaviourDataConstructorII(behaviourIntegrationFile,
								  externalStateVarsHolder,
								  iprefix+"dext_vars","d","");
    }
    behaviourIntegrationFile << "}" << endl << endl;
  }

  void 
  UMATInterfaceBase::writeBehaviourDataMainVariablesSetters(std::ofstream& os,
								  const BehaviourDescription& mb) const
  {
    using namespace std;
    const auto iprefix = makeUpperCase(this->getInterfaceName());
    map<DrivingVariable,
	ThermodynamicForce>::const_iterator pm;
    SupportedTypes::TypeSize ov;
    os << "void set"+iprefix+"BehaviourDataDrivingVariables(const Type* const "+iprefix+"stran)" << endl
       << "{" << endl;
    for(pm=mb.getMainVariables().begin();pm!=mb.getMainVariables().end();++pm){
      const auto& v = pm->first;
      if(!v.increment_known){
	if(this->getTypeFlag(v.type)==SupportedTypes::TVector){
	  if(pm!=mb.getMainVariables().begin()){
	    os << "tfel::fsalgo::copy<N>::exe("+iprefix+"stran+"  << ov <<",this->" << v.name << "0.begin());" << endl;
	  } else {
	    os << "tfel::fsalgo::copy<N>::exe("+iprefix+"stran,this->" << v.name << "0.begin());" << endl;
	  }
	} else if(this->getTypeFlag(v.type)==SupportedTypes::Stensor){
	  if(pm!=mb.getMainVariables().begin()){
	    os << "this->" << v.name << "0.importVoigt("+iprefix+"stran+" << ov <<");" << endl;
	  } else {
	    os << "this->" << v.name << "0.importVoigt("+iprefix+"stran);" << endl;
	  }
	} else if(this->getTypeFlag(v.type)==SupportedTypes::Tensor){
	  if(pm!=mb.getMainVariables().begin()){
	    os << v.type << "::buildFromFortranMatrix(this->" << v.name << "0,"
			      << iprefix << "stran+" << ov <<");" << endl;
	  } else {
	    os << v.type << "::buildFromFortranMatrix(this->" << v.name << "0,"
			      << iprefix << "stran);" << endl;
	  }
	} else {
	  string msg("UMATInterfaceBase::writeBehaviourDataMainVariablesSetters : ");
	  msg += "unsupported driving variable type";
	  throw(runtime_error(msg));
	}
      } else {
	if(this->getTypeFlag(v.type)==SupportedTypes::TVector){
	  if(pm!=mb.getMainVariables().begin()){
	    os << "tfel::fsalgo::copy<N>::exe("+iprefix+"stran+"  << ov << ",this->"  << v.name << ".begin());" << endl;
	  } else {
	    os << "tfel::fsalgo::copy<N>::exe("+iprefix+"stran,this->"   << v.name << ".begin());" << endl;
	  }
	} else if(this->getTypeFlag(v.type)==SupportedTypes::Stensor){
	  if(pm!=mb.getMainVariables().begin()){
	    os << "this->" << v.name << ".importVoigt("+iprefix+"stran+" << ov <<");" << endl;
	  } else {
	    os << "this->"  << v.name << ".importVoigt("+iprefix+"stran);" << endl;
	  }
	} else if(this->getTypeFlag(v.type)==SupportedTypes::Tensor){
	  if(pm!=mb.getMainVariables().begin()){
	    os << v.type << "::buildFromFortranMatrix(this->" << v.name << ","
			      << iprefix << "stran+" << ov <<");" << endl;
	  } else {
	    os << v.type << "::buildFromFortranMatrix(this->" << v.name << ","
			      << iprefix+"stran);" << endl;
	  }
	} else {
	  string msg("UMATInterfaceBase::writeBehaviourDataMainVariablesSetters : ");
	  msg += "unsupported driving variable type";
	  throw(runtime_error(msg));
	}
      }
      ov += this->getTypeSize(v.type,1u);
    }
    os << "}" << endl << endl;
    os << "void set"+iprefix+"BehaviourDataThermodynamicForces(const Type* const "+iprefix+"stress_)" << endl
      << "{" << endl;
    SupportedTypes::TypeSize of;
    for(pm=mb.getMainVariables().begin();pm!=mb.getMainVariables().end();++pm){
      const auto& f = pm->second;
      if(this->getTypeFlag(f.type)==SupportedTypes::TVector){
	if(pm!=mb.getMainVariables().begin()){
	  os << "tfel::fsalgo::copy<N>::exe("+iprefix+"stress_+" << of <<",this->" << f.name << ".begin());" << endl;
	} else {
	  os << "tfel::fsalgo::copy<N>::exe("+iprefix+"stress_,this->" << f.name << ".begin());" << endl;
	}
      } else if(this->getTypeFlag(f.type)==SupportedTypes::Stensor){
	if(pm!=mb.getMainVariables().begin()){
	  os << "this->" << f.name << ".importTab("+iprefix+"stress_+" << of <<");" << endl;
	} else {
	  os << "this->" << f.name << ".importTab("+iprefix+"stress_);" << endl;
	}
      } else if(this->getTypeFlag(f.type)==SupportedTypes::Tensor){
	if(pm!=mb.getMainVariables().begin()){
	  os << f.type << "::buildFromFortranMatrix(this->" << f.name << ","
			    << iprefix << "stress_+" << of <<");" << endl;
	} else {
	  os << f.type << "::buildFromFortranMatrix(this->" << f.name << ","
			    << iprefix << "stress_);" << endl;
	}
      } else {
	string msg("UMATInterfaceBase::writeBehaviourDataMainVariablesSetters : ");
	msg += "unsupported forces type";
	throw(runtime_error(msg));
      }
      of += this->getTypeSize(f.type,1u);
    }
    os << "}" << endl << endl;
  }

  void 
  UMATInterfaceBase::writeIntegrationDataMainVariablesSetters(std::ofstream& os,
								    const BehaviourDescription& mb) const
  {
    using namespace std;
    const std::string iprefix = makeUpperCase(this->getInterfaceName());
    map<DrivingVariable,
	ThermodynamicForce>::const_iterator pm;
    SupportedTypes::TypeSize ov;
    os << "void set"+iprefix+"IntegrationDataDrivingVariables(const Type* const "+iprefix+"dstran)" << endl
       << "{" << endl;
    for(pm=mb.getMainVariables().begin();pm!=mb.getMainVariables().end();++pm){
      const auto& v = pm->first;
      if(!v.increment_known){
	if(this->getTypeFlag(v.type)==SupportedTypes::TVector){
	  if(pm!=mb.getMainVariables().begin()){
	    os << "tfel::fsalgo::copy<N>::exe("+iprefix+"dstran+" << ov <<",this->" << v.name << "1.begin());" << endl;
	  } else {
	    os << "tfel::fsalgo::copy<N>::exe("+iprefix+"dstran,this->" << v.name << "1.begin());" << endl;
	  }
	} else if(this->getTypeFlag(v.type)==SupportedTypes::Stensor){
	  if(pm!=mb.getMainVariables().begin()){
	    os << "this->" << v.name << "1.importVoigt("+iprefix+"dstran+" << ov <<");" << endl;
	  } else {
	    os << "this->" << v.name << "1.importVoigt("+iprefix+"dstran);" << endl;
	  }
	} else if(this->getTypeFlag(v.type)==SupportedTypes::Tensor){
	  if(pm!=mb.getMainVariables().begin()){
	    os << v.type << "::buildFromFortranMatrix(this->" << v.name << "1,"
			      << iprefix << "dstran+" << ov <<");" << endl;
	  } else {
	    os << v.type << "::buildFromFortranMatrix(this->" << v.name << "1,"
			      << iprefix << "dstran);" << endl;
	  }
	} else {
	  string msg("UMATInterfaceBase::writeIntegrationDataMainVariablesSetters : ");
	  msg += "unsupported driving variable type";
	  throw(runtime_error(msg));
	}
      } else {
	if(this->getTypeFlag(v.type)==SupportedTypes::TVector){
	  if(pm!=mb.getMainVariables().begin()){
	    os << "tfel::fsalgo::copy<N>::exe("+iprefix+"dstran+" << ov << ",this->d" << v.name << ".begin());" << endl;
	  } else {
	    os << "tfel::fsalgo::copy<N>::exe("+iprefix+"dstran,this->d" << v.name << ".begin());" << endl;
	  }
	} else if(this->getTypeFlag(v.type)==SupportedTypes::Stensor){
	  if(pm!=mb.getMainVariables().begin()){
	    os << "this->d" << v.name << ".importVoigt("+iprefix+"dstran+" << ov <<");" << endl;
	  } else {
	    os << "this->d" << v.name << ".importVoigt("+iprefix+"dstran);" << endl;
	  }
	} else if(this->getTypeFlag(v.type)==SupportedTypes::Tensor){
	  if(pm!=mb.getMainVariables().begin()){
	    os << v.type << "::buildFromFortranMatrix(this->d" << v.name << ","
			      << iprefix << "dstran+" << ov <<");" << endl;
	  } else {
	    os << v.type << "::buildFromFortranMatrix(this->d" << v.name << ","
			      << iprefix+"dstran);" << endl;
	  }
	} else {
	  string msg("UMATInterfaceBase::writeIntegrationDataMainVariablesSetters : ");
	  msg += "unsupported driving variable type";
	  throw(runtime_error(msg));
	}
      }
      ov += this->getTypeSize(v.type,1u);
    }
    os << "}" << endl << endl;
  }

  void
  UMATInterfaceBase::writeExternalNames(std::ostream& f,
					      const std::string& name,
					      const Hypothesis& h,
					      const std::vector<std::string>& v,
					      const std::string& t) const
  {
    using namespace std;
    if(v.empty()){
      f << "MFRONT_SHAREDOBJ const char * const * "  << this->getSymbolName(name,h)
	<< "_" << t << " = nullptr;" << endl << endl;
    } else {
      vector<string>::size_type s = 0u;
      vector<string>::const_iterator p = v.begin();      
      f << "MFRONT_SHAREDOBJ const char * " << this->getSymbolName(name,h)
	<< "_" << t << "[" << v.size() <<  "] = {";
      while(p!=v.end()){
	f << '"' << *p << '"';
	if(++p!=v.end()){
	  if(s%5==0){
	    f << "," << endl;
	  } else {
	    f << ",";
	  }
	}
	++s;
      }
      f << "};" << endl;
    }
  } // end of UMATInterfaceBase::writeExternalNames

  bool
  UMATInterfaceBase::readBooleanValue(const std::string& key,
					    tfel::utilities::CxxTokenizer::TokensContainer::const_iterator& current,
					    const tfel::utilities::CxxTokenizer::TokensContainer::const_iterator end) const
  {
    using namespace std;
    bool b = true;
    if(current==end){
      string msg("MFrontUmatInterfaceBase::readBooleanValue ("+key+") : ");
      msg += "unexpected end of file";
      throw(runtime_error(msg));
    }
    if(current->value=="true"){
      b = true;
    } else if(current->value=="false"){
      b = false;
    } else {
      string msg("MFrontUmatInterfaceBase::readBooleanValue ("+key+") :");
      msg += "expected 'true' or 'false'";
      throw(runtime_error(msg));
    }
    ++(current); 
    if(current==end){
      string msg("MFrontUmatInterfaceBase::readBooleanValue ("+key+") : ");
      msg += "unexpected end of file";
      throw(runtime_error(msg));
    }    
    if(current->value!=";"){
      string msg("MFrontUmatInterfaceBase::readBooleanValue : expected ';', read ");
      msg += current->value;
      throw(runtime_error(msg));
    }
    ++(current);
    return b;
  }

  void
  UMATInterfaceBase::writeVisibilityDefines(std::ostream& out) const
  {
    out << "#ifdef _WIN32\n"
	<< "#ifndef NOMINMAX\n"
	<< "#define NOMINMAX\n"
	<< "#endif /* NOMINMAX */\n"
	<< "#include <windows.h>\n"
	<< "#endif /* _WIN32 */\n\n"
	<< "#ifndef MFRONT_SHAREDOBJ\n"
	<< "#define MFRONT_SHAREDOBJ TFEL_VISIBILITY_EXPORT\n" 
	<< "#endif /* MFRONT_SHAREDOBJ */\n\n"; 
  } // end of UMATInterfaceBase::writeVisibilityDefines

  void
  UMATInterfaceBase::checkParametersType(bool& rp,
					       bool& ip,
					       bool& up,
					       const VariableDescriptionContainer& pc) const
  {
    using namespace std;
    VariableDescriptionContainer::const_iterator pp;
    rp = false;
    ip = false;
    up = false;
    for(pp=pc.begin();pp!=pc.end();++pp){
      if(pp->type=="real"){
	rp = true;
      } else if(pp->type=="int"){
	ip = true;
      } else if(pp->type=="ushort"){
	up = true;
      } else {
	string msg("UMATInterfaceBase::checkParametersType : ");
	msg += "unsupport parameter type '"+pp->type+"'.";
	throw(runtime_error(msg));
      } 
    }
  }

  void
  UMATInterfaceBase::writeSetOutOfBoundsPolicyFunctionDeclaration(std::ostream& out,
									const std::string& name) const
  {
    out << "MFRONT_SHAREDOBJ void\n"
	<< getFunctionName(name) << "_setOutOfBoundsPolicy(const int);\n\n";
  }

  void
  UMATInterfaceBase::writeGetOutOfBoundsPolicyFunctionImplementation(std::ostream& out,
									   const std::string& name) const
  {
    out << "static tfel::material::OutOfBoundsPolicy&\n"
	<< getFunctionName(name) << "_getOutOfBoundsPolicy(void){\n"
	<< "using namespace tfel::material;\n"
	<< "static OutOfBoundsPolicy policy = None;\n"
	<< "return policy;\n"
	<< "}\n\n";
  } // end of UMATInterfaceBase::writeGetOutOfBoundsPolicyFunctionImplementation    
  
  void
  UMATInterfaceBase::writeSetOutOfBoundsPolicyFunctionImplementation(std::ostream& out,
							             const std::string& name) const
  {
    out << "MFRONT_SHAREDOBJ void\n"
	<< getFunctionName(name) << "_setOutOfBoundsPolicy(const int p){\n"
	<< "if(p==0){\n"
	<< getFunctionName(name) << "_getOutOfBoundsPolicy() = tfel::material::None;\n"
	<< "} else if(p==1){\n"
	<< getFunctionName(name) << "_getOutOfBoundsPolicy() = tfel::material::Warning;\n"
	<< "} else if(p==2){\n"
	<< getFunctionName(name) << "_getOutOfBoundsPolicy() = tfel::material::Strict;\n"
	<< "} else {\n"
      	<< "std::cerr << \"" << getFunctionName(name)
	<< "_setOutOfBoundsPolicy : invalid argument\\n\";\n"
	<< "}\n"
	<< "}\n\n";
  }
  
  void
  UMATInterfaceBase::writeSetParametersFunctionsDeclarations(std::ostream& out,
								   const std::string& name,
								   const BehaviourDescription& mb) const
  {
    using namespace std;
    const set<Hypothesis> h  = mb.getDistinctModellingHypotheses();
    const set<Hypothesis> h2 = this->getModellingHypothesesToBeTreated(mb);
    set<Hypothesis>::const_iterator p;
    for(p=h.begin();p!=h.end();++p){
      if((*p==ModellingHypothesis::UNDEFINEDHYPOTHESIS)||
	 (h2.find(*p)!=h2.end())){
	bool rp,ip,up;
	const auto& d = mb.getBehaviourData(*p);
	const auto& pc = d.getParameters();
	this->checkParametersType(rp,ip,up,pc);
	string fctName = this->getFunctionName(name);
	string suffix;
	if(*p!=ModellingHypothesis::UNDEFINEDHYPOTHESIS){
	  suffix += ModellingHypothesis::toString(*p);
	  fctName += "_"+suffix;
	}
	if(rp){
	  out << "MFRONT_SHAREDOBJ int" << endl
	      << fctName << "_setParameter(const char *const,const double);" << endl << endl;
	}
	if(ip){
	  out << "MFRONT_SHAREDOBJ int" << endl
	      << fctName << "_setIntegerParameter(const char *const,const int);" << endl << endl;
	}
	if(up){
	  out << "MFRONT_SHAREDOBJ int" << endl
	      << fctName << "_setUnsignedShortParameter(const char *const,const unsigned short);" << endl << endl;
	}
      }
    }
  } // end of UMATInterfaceBase::writeSetParametersFunctionsDeclarations

  void
  UMATInterfaceBase::writeSetParametersFunctionsImplementations(std::ostream& out,
								      const std::string& name,
								      const BehaviourDescription& mb) const
  {
    using namespace std;
    set<Hypothesis> h  = mb.getDistinctModellingHypotheses();
    set<Hypothesis> h2 = this->getModellingHypothesesToBeTreated(mb);
    set<Hypothesis>::const_iterator p;
    for(p=h.begin();p!=h.end();++p){
      if((*p==ModellingHypothesis::UNDEFINEDHYPOTHESIS)||
	 (h2.find(*p)!=h2.end())){
	const auto& d = mb.getBehaviourData(*p);
	const auto& pc = d.getParameters();
	bool rp,ip,up;
	this->checkParametersType(rp,ip,up,pc);
	string fctName = this->getFunctionName(name);
	string suffix;
	if(*p!=ModellingHypothesis::UNDEFINEDHYPOTHESIS){
	  suffix  += ModellingHypothesis::toString(*p);
	  fctName += "_"+suffix;
	}
	string cname = mb.getClassName() + suffix + "ParametersInitializer";
	if(rp){
	  out << "MFRONT_SHAREDOBJ int" << endl
	      << fctName << "_setParameter(const char *const key,const double value){" << endl
	      << "using namespace std;" << endl
	      << "using namespace tfel::material;" << endl
	      << cname << "& i = " << cname << "::get();" << endl
	      << "try{" << endl
	      << "i.set(key,value);" << endl
	      << "} catch(runtime_error& e){" << endl
	      << "cerr << e.what() << endl;" << endl
	      << "return 0;" << endl
	      << "}" << endl
	      << "return 1;" << endl
	      << "}" << endl << endl;
	}
	if(ip){
	  out << "MFRONT_SHAREDOBJ int" << endl
	      << fctName << "_setIntegerParameter(const char *const key,const int value){" << endl
	      << "using namespace std;" << endl
	      << "using namespace tfel::material;" << endl
	      << cname << "& i = " << cname << "::get();" << endl
	      << "try{" << endl
	      << "i.set(key,value);" << endl
	      << "} catch(runtime_error& e){" << endl
	      << "cerr << e.what() << endl;" << endl
	      << "return 0;" << endl
	      << "}" << endl
	      << "return 1;" << endl
	      << "}" << endl << endl;
	}
	if(up){
	  out << "MFRONT_SHAREDOBJ int" << endl
	      << fctName << "_setUnsignedShortParameter(const char *const key,const unsigned short value){" << endl
	      << "using namespace std;" << endl
	      << "using namespace tfel::material;" << endl
	      << cname << "& i = " << cname << "::get();" << endl
	      << "try{" << endl
	      << "i.set(key,value);" << endl
	      << "} catch(runtime_error& e){" << endl
	      << "cerr << e.what() << endl;" << endl
	      << "return 0;" << endl
	      << "}" << endl
	      << "return 1;" << endl
	      << "}" << endl << endl;
	}
      }
    }
  }

  std::string
  UMATInterfaceBase::getHeaderDefine(const BehaviourDescription& mb) const
  {
    using namespace std;
    const auto& m = mb.getMaterialName();
    string header = "LIB_"+makeUpperCase(this->getInterfaceName());
    if(!mb.getLibrary().empty()){
      header += "_";
      header += makeUpperCase(mb.getLibrary());
    }
    if(!m.empty()){
      header += "_";
      header += makeUpperCase(m);
    }
    header += "_";
    header += makeUpperCase(mb.getClassName());
    header += "_HXX";
    return header;
  }

  void
  UMATInterfaceBase::getExtraSrcIncludes(std::ostream& out,
					       const BehaviourDescription& mb) const
  {
    using namespace std;
    if(mb.hasParameters()){
      out << "#include<iostream>" << endl;
      out << "#include<stdexcept>" << endl;
    }
    if(this->generateMTestFile){
      out << "#include<vector>" << endl;
    }
    if(this->generateMTestFile){
      out << "#include<sstream>" << endl;
    }
    if(this->generateMTestFile){
      out << "#include\"TFEL/Material/ModellingHypothesis.hxx\"" << endl;
      out << "#include\"MFront/SupportedTypes.hxx\"" << endl;
      out << "#include\"MFront/UmatSmallStrainMTestFileGenerator.hxx\"" << endl;
      out << "#include\"MFront/UmatFiniteStrainMTestFileGenerator.hxx\"" << endl;
    }
  } // end of UMATInterfaceBase::getExtraSrcIncludes
  
  void
  UMATInterfaceBase::generateMTestFile1(std::ostream& out) const
  {
    using namespace std;
    if(this->generateMTestFile){
      out << "using namespace std;" << endl;
      out << "using tfel::material::ModellingHypothesis;" << endl;
      out << "using mfront::SupportedTypes;" << endl;
      out << "// double is used by MTestFileGeneratorBase" << endl;
      out << "vector<double> mg_STRESS(*NTENS);" << endl;
      out << "vector<double> mg_STATEV(*NSTATV);" << endl;
      out << "copy(STRESS,STRESS+*NTENS,mg_STRESS.begin());" << endl;
      out << "copy(STATEV,STATEV+*NSTATV,mg_STATEV.begin());" << endl;
    }
  } // end of UMATInterfaceBase::generateMTestFile1

  void
  UMATInterfaceBase::generateMTestFile2(std::ostream& out,
					      const BehaviourDescription::BehaviourType type,
					      const std::string& name,
					      const std::string& suffix,
					      const BehaviourDescription& mb) const
  {
    using namespace std;
    if(this->generateMTestFile){
      string fname = name;
      if(!suffix.empty()){
    	fname += "_"+suffix;
      }
      if(type==BehaviourDescription::SMALLSTRAINSTANDARDBEHAVIOUR){
    	out << "mfront::UmatSmallStrainMTestFileGenerator mg(\""
    	    << makeLowerCase(this->getInterfaceName()) << "\",\""
    	    << this->getLibraryName(mb) <<"\",\"" << this->getFunctionName(fname)
    	    <<  "\");" << endl;
      } else if(type==BehaviourDescription::FINITESTRAINSTANDARDBEHAVIOUR){
    	out << "mfront::UmatFiniteStrainMTestFileGenerator mg(\""
    	    << makeLowerCase(this->getInterfaceName()) << "\",\""
    	    << this->getLibraryName(mb) <<"\",\"" << this->getFunctionName(fname)
    	    <<  "\");" << endl;
      } else {
    	string msg("UMATInterfaceBase::generateMTestFile2 : ");
    	msg += "only small strain or finite strain behaviours are supported";
    	throw(runtime_error(msg));
      }
      this->writeMTestFileGeneratorSetModellingHypothesis(out);
      this->writeMTestFileGeneratorSetRotationMatrix(out,mb);
      out << "const unsigned short TVectorSize = mg.getTVectorSize();" << endl;
      out << "const unsigned short StensorSize = mg.getStensorSize();" << endl;
      out << "const unsigned short TensorSize  = mg.getTensorSize();" << endl;
      out << "mg.setHandleThermalExpansion(false);" << endl;
      out << "mg.addTime(0.);" << endl;
      out << "mg.addTime(*DTIME>0 ? *DTIME : 1.e-50);" << endl;
      if(type==BehaviourDescription::SMALLSTRAINSTANDARDBEHAVIOUR){
    	out << "mg.setStrainTensor(STRAN);" << endl;
    	out << "mg.setStrainTensorIncrement(DSTRAN);" << endl;
    	out << "mg.setStressTensor(&mg_STRESS[0]);" << endl;
      } else if(type==BehaviourDescription::FINITESTRAINSTANDARDBEHAVIOUR){
    	out << "mg.setDeformationGradientAtTheBeginningOfTheStimeStep(F0);" << endl;
    	out << "mg.setDeformationGradientAtTheEndOfTheStimeStep(F1);" << endl;
    	out << "mg.setStressTensor(&mg_STRESS[0]);" << endl;
      } else {
    	string msg("UMATInterfaceBase::generateMTestFile2 : ");
    	msg += "only small strain or finite strain behaviours are supported";
    	throw(runtime_error(msg));
      }
      const auto& gh = this->gatherModellingHypothesesAndTests(mb);
      for(const auto & elem : gh){
	const auto& d = mb.getBehaviourData(elem.first);
	const auto& persistentVarsHolder = d.getPersistentVariables();
	const auto& externalStateVarsHolder = d.getExternalStateVariables();
	pair<vector<UMATMaterialProperty>,
	     SupportedTypes::TypeSize> mprops = this->buildMaterialPropertiesList(mb,elem.first);
	VariableDescriptionContainer::const_iterator p;
	unsigned short i;
	unsigned int offset;
	out << "if(" << elem.second << "){" << endl;
	offset=0;
	for(const auto& m : mprops.first){
	  auto flag = this->getTypeFlag(m.type);
	  if(flag!=SupportedTypes::Scalar){
	    throw(runtime_error("UMATInterfaceBase::generateFile2 : "
				"unsupported external state variable type "
				"in mtest file generation"));
	  }
	  if(m.arraySize==1u){
	    if(offset==0){
	      out << "mg.addMaterialProperty(\"" << m.name << "\",*(PROPS));" << endl;	    
	    } else {
	      out << "mg.addMaterialProperty(\"" << m.name << "\",*(PROPS+" << offset << "));" << endl;	    
	    }
	    ++offset;
	  } else {
	    for(unsigned short s=0;s!=m.arraySize;++s,++offset){
	      if(offset==0){
		out << "mg.addMaterialProperty(\"" << m.name << "[" << s << "]\",*(PROPS));" << endl;	    
	      } else {
		out << "mg.addMaterialProperty(\"" << m.name << "[" << s << "]\","
		    << "*(PROPS+" << offset << "));" << endl;	    
	      }
	    }
	  }
	}
	SupportedTypes::TypeSize ivoffset;
	for(p=persistentVarsHolder.begin();p!=persistentVarsHolder.end();++p){
	  auto flag = this->getTypeFlag(p->type);
	  const auto& ivname = d.getExternalName(p->name);
	  if(p->arraySize==1u){
	    if(flag==SupportedTypes::Scalar){
	      out << "mg.addInternalStateVariable(\"" << ivname << "\",SupportedTypes::Scalar,&mg_STATEV[" << ivoffset<< "]);" << endl;
	      ivoffset += SupportedTypes::TypeSize(1u,0u,0u,0u);
	    } else {
	      out << "mg.addInternalStateVariable(\"" << ivname << "\",SupportedTypes::Stensor,&mg_STATEV[" << ivoffset<< "]);" << endl;
	      ivoffset += SupportedTypes::TypeSize(0u,0u,1u,0u);
	    }
	  } else {
	    if(p->arraySize>=SupportedTypes::ArraySizeLimit){
	      out << "for(unsigned short i=0;i!=" << p->arraySize << ";++i){" << endl;
	      out << "ostringstream name;" << endl;
	      out << "name << \"" << ivname << "[\" << i << \"]\";" << endl;
	      if(flag==SupportedTypes::Scalar){
		out << "mg.addInternalStateVariable(name.str(),SupportedTypes::Scalar,&mg_STATEV[" << ivoffset<< "]+i);" << endl;
	      } else {
		out << "mg.addInternalStateVariable(name.str(),SupportedTypes::Stensor,&mg_STATEV[" << ivoffset<< "]+i);" << endl;
	      }
	      out << "}" << endl;
	      if(flag==SupportedTypes::Scalar){
		ivoffset += SupportedTypes::TypeSize(p->arraySize,0u,0u,0u);
	      } else {
		ivoffset += SupportedTypes::TypeSize(0u,0u,p->arraySize,0u);
	      }
	    } else {
	      for(i=0;i!=p->arraySize;++i){
		if(flag==SupportedTypes::Scalar){
		  out << "mg.addInternalStateVariable(\""
		      << ivname << "[" << i << "]\",SupportedTypes::Scalar,&mg_STATEV[" << ivoffset<< "]);" << endl;
		  ivoffset += SupportedTypes::TypeSize(1u,0u,0u,0u);
		} else {
		  out << "mg.addInternalStateVariable(\""
		      << ivname << "[" << i << "]\",SupportedTypes::Stensor,&mg_STATEV[" << ivoffset<< "]);" << endl;
		  ivoffset += SupportedTypes::TypeSize(0u,0u,1u,0u);
		}
	      }
	    }
	  }
	}
	out << "mg.addExternalStateVariableValue(\"Temperature\",0.,*TEMP);" << endl;
	out << "mg.addExternalStateVariableValue(\"Temperature\",*DTIME,*TEMP+*DTEMP);" << endl;
	for(p=externalStateVarsHolder.begin(),offset=0;p!=externalStateVarsHolder.end();++p){
	  auto flag = this->getTypeFlag(p->type);
	  if(flag!=SupportedTypes::Scalar){
	    string msg("UMATInterfaceBase::generateFile2 : "
		       "unsupported external state variable type "
		       "in mtest file generation");
	    throw(runtime_error(msg));
	  }
	  const auto& evname = d.getExternalName(p->name);
	  if(p->arraySize==1u){
	    if(offset==0){
	      out << "mg.addExternalStateVariableValue(\"" << evname 
		  << "\",0,*PREDEF);" << endl;
	      out << "mg.addExternalStateVariableValue(\"" << evname 
		  << "\",*DTIME,*PREDEF+*DPRED);" << endl;
	    } else {
	      out << "mg.addExternalStateVariableValue(\"" << evname 
		  << "\",0,*(PREDEF+" << offset<< "));" << endl;
	      out << "mg.addExternalStateVariableValue(\"" << evname 
		  << "\",*DTIME,*(PREDEF+" << offset<< ")+*(DPRED+" << offset<< "));" << endl;
	    }
	    ++offset;
	  } else {
	    if(p->arraySize>=SupportedTypes::ArraySizeLimit){
	      out << "for(unsigned short i=0;i!=" << p->arraySize << ";++i){" << endl;
	      out << "ostringstream name;" << endl;
	      out << "name << \"" << evname << "[\" << i << \"]\";" << endl;
	      if(offset==0){
		out << "mg.addExternalStateVariableValue(name.str(),0,*(PREDEF+i));" << endl;
		out << "mg.addExternalStateVariableValue(name.str(),"
		  "*DTIME,*(PREDEF+i)+*(DPRED+i));" << endl;
	      } else {
		out << "mg.addExternalStateVariableValue(name.str(),"
		  "0,*(PREDEF+" << offset<< "+i));" << endl;
		out << "mg.addExternalStateVariableValue(name.str(),"
		  "*DTIME,*(PREDEF+" << offset << "+i)+*(DPRED+" << offset<< "+i));" << endl;
	      }
	      out << "}" << endl;
	      offset += p->arraySize;
	    } else {
	      for(i=0;i!=p->arraySize;++i,++offset){
		if(offset==0){
		  out << "mg.addExternalStateVariableValue(\"" << evname
		      << "[" << i << "]\",0,*PREDEF);" << endl;
		  out << "mg.addExternalStateVariableValue(\"" << evname
		      << "[" << i << "]\",*DTIME,*PREDEF+*DPRED);" << endl;
		} else {
		  out << "mg.addExternalStateVariableValue(\""
		      << evname << "[" << i << "]\","
		    "0,*(PREDEF+" << offset<< "));" << endl;
		  out << "mg.addExternalStateVariableValue(\""
		      << evname << "[" << i << "]\","
		    "*DTIME,*(PREDEF+" << offset<< ")+*(DPRED+" << offset<< "));" << endl;
		}
	      }
	    }
	  }
	}
	out << "}" << endl;
      }
      out << "mg.generate(\""+name+"\");" << endl;
      out << "static_cast<void>(TVectorSize); // remove gcc warning" << endl;
      out << "static_cast<void>(StensorSize); // remove gcc warning" << endl;
      out << "static_cast<void>(TensorSize);  // remove gcc warning" << endl;
    }
  }

  void
  UMATInterfaceBase::writeMTestFileGeneratorSetRotationMatrix(std::ostream& out,
								    const BehaviourDescription& mb) const
  {
    using namespace std;
    if(mb.getSymmetryType()==mfront::ORTHOTROPIC){
      out << "mg.setRotationMatrix("
	  << "DROT[0],DROT[3],DROT[6],"
	  << "DROT[1],DROT[4],DROT[7],"
	  << "DROT[2],DROT[5],DROT[8]);" << endl;
    }
  } // end of UMATInterfaceBase::writeMTestFileGeneratorSetRotationMatrix

  void
  UMATInterfaceBase::generateUMATxxGeneralSymbols(std::ostream& out,
							const std::string& name,
							const BehaviourDescription& mb,
							const FileDescription & fd) const
  {
    this->writeUMATxxSourceFileSymbols(out,name,mb,fd);
    this->writeUMATxxSupportedModellingHypothesis(out,name,mb);
    this->writeUMATxxBehaviourTypeSymbols(out,name,mb);
    this->writeUMATxxSymmetryTypeSymbols(out,name,mb);
    this->writeUMATxxElasticSymmetryTypeSymbols(out,name,mb);
    this->writeUMATxxSpecificSymbols(out,name,mb,fd);
  }

  void
  UMATInterfaceBase::generateUMATxxSymbols(std::ostream& out,
						 const std::string& name,
						 const Hypothesis h,
						 const BehaviourDescription& mb,
						 const FileDescription & fd) const
  {
    this->writeUMATxxIsUsableInPurelyImplicitResolutionSymbols(out,name,h,mb);
    this->writeUMATxxMaterialPropertiesSymbols(out,name,h,mb);
    this->writeUMATxxStateVariablesSymbols(out,name,h,mb);
    this->writeUMATxxExternalStateVariablesSymbols(out,name,h,mb);
    this->writeUMATxxRequirementsSymbols(out,name,h,mb);
    this->writeUMATxxAdditionalSymbols(out,name,h,mb,fd);
  }

  void
  UMATInterfaceBase::writeUMATxxSpecificSymbols(std::ostream&,
						      const std::string&,
						      const BehaviourDescription&,
						      const FileDescription&) const
  {} // end of UMATInterfaceBase::writeUMATxxSpecificSymbols

  void
  UMATInterfaceBase::writeUMATxxBehaviourTypeSymbols(std::ostream& out,
							   const std::string& name,
							   const BehaviourDescription& mb) const
  {
    using namespace std;
    out << "MFRONT_SHAREDOBJ unsigned short " << this->getFunctionName(name) 
	<< "_BehaviourType = " ;
    if(mb.getBehaviourType()==BehaviourDescription::GENERALBEHAVIOUR){
      out << "0u;" << endl << endl;
    } else if(mb.getBehaviourType()==BehaviourDescription::SMALLSTRAINSTANDARDBEHAVIOUR){
      out << "1u;" << endl << endl;
    } else if(mb.getBehaviourType()==BehaviourDescription::FINITESTRAINSTANDARDBEHAVIOUR){
      out << "2u;" << endl << endl;
    } else if(mb.getBehaviourType()==BehaviourDescription::COHESIVEZONEMODEL){
      out << "3u;" << endl << endl;
    } else {
      string msg("UMATInterfaceBase::writeUMATxxBehaviourTypeSymbols : ");
      msg += "unsupported behaviour type.";
      throw(runtime_error(msg));
    }
  } // end of UMATInterfaceBase::writeUMATxxBehaviourTypeSymbols

  void
  UMATInterfaceBase::writeUMATxxSupportedModellingHypothesis(std::ostream& out,
								   const std::string& name,
								   const BehaviourDescription& mb) const
  {
    using namespace std;
    using namespace tfel::material;
    auto ih = this->getModellingHypothesesToBeTreated(mb);
    if(ih.empty()){
      out << "MFRONT_SHAREDOBJ unsigned short "  << this->getFunctionName(name)
	  << "_nModellingHypotheses = 0u;" << endl << endl;
      out << "MFRONT_SHAREDOBJ const char * const * "  << this->getFunctionName(name)
	  << "_ModellingHypotheses = 0;" << endl << endl;
    } else {
      out << "MFRONT_SHAREDOBJ unsigned short "  << this->getFunctionName(name)
	  << "_nModellingHypotheses = " << ih.size() << "u;" << endl << endl;
      out << "MFRONT_SHAREDOBJ const char * " << endl
	  << this->getFunctionName(name) << "_ModellingHypotheses[" << ih.size() << "u] = {";
      for(auto ph=ih.begin();ph!=ih.end();){
	out << "\"" << ModellingHypothesis::toString(*ph) << "\"";
	if(++ph!=ih.end()){
	  out << "," << endl;
	}
      }
      out << "};" << endl << endl;
    }
  } // end of UMATInterfaceBase::writeUMATxxSupportedModellingHypothesis

  void
  UMATInterfaceBase::writeUMATxxMaterialPropertiesSymbols(std::ostream& out,
								const std::string& name,
								const Hypothesis h,
								const BehaviourDescription& mb) const
  {
    using namespace std;
    const auto mprops = this->buildMaterialPropertiesList(mb,h);
    if(mprops.first.empty()){
      out << "MFRONT_SHAREDOBJ unsigned short "  << this->getSymbolName(name,h)
	  << "_nMaterialProperties = 0u;" << endl << endl;
      out << "MFRONT_SHAREDOBJ const char * const *"  << this->getSymbolName(name,h)
	  << "_MaterialProperties = nullptr;" << endl << endl;
    } else {
      const auto& last = mprops.first.back();
      SupportedTypes::TypeSize s;
      if((mprops.second.getTensorSize()!=0)||(mprops.second.getStensorSize()!=0)||
	 (mprops.second.getTVectorSize()!=0)){
	string msg("UMATInterface::writeUMATxxMaterialPropertiesSymbols : "
		   "internal error : the material properties shall all be scalars");
	throw(runtime_error(msg));
      }
      s  = last.offset;
      s += this->getTypeSize(last.type,last.arraySize);
      s -= mprops.second;
      if((s.getTensorSize()!=0)||(s.getStensorSize()!=0)||(s.getTVectorSize()!=0)){
	string msg("UMATInterface::writeUMATxxMaterialPropertiesSymbols : "
		   "internal error : the material properties shall all be scalars");
	throw(runtime_error(msg));
      }
      if(s.getScalarSize()<0){
	string msg("UMATInterface::writeUMATxxMaterialPropertiesSymbols : "
		   "internal error : negative number of the material properties");
	throw(runtime_error(msg));
      }
      vector<UMATMaterialProperty>::size_type ib=0; /* index of the first element which
						     * is not imposed by the material properties */
      bool found = false;
      for(decltype(mprops.first.size()) i=0;
	  (i!=mprops.first.size())&&(!found);++i){
	if(mprops.first[i].offset==mprops.second){
	  ib = i;
	  found = true;
	}
      }
      if(!found){
	if(s.getScalarSize()!=0){
	  string msg("UMATInterface::writeUMATxxMaterialPropertiesSymbols : "
		     "internal error : inconsistent offset declaration");
	  throw(runtime_error(msg));
	}
	out << "MFRONT_SHAREDOBJ unsigned short "  << this->getSymbolName(name,h)
	    << "_nMaterialProperties = 0u;" << endl << endl;
	out << "MFRONT_SHAREDOBJ const char * const *"  << this->getSymbolName(name,h)
	    << "_MaterialProperties = nullptr;" << endl << endl;
      } else {
	out << "MFRONT_SHAREDOBJ unsigned short "  << this->getSymbolName(name,h)
	    << "_nMaterialProperties = " << s.getScalarSize() << "u;" << endl << endl;
	out << "MFRONT_SHAREDOBJ const char *"  << this->getSymbolName(name,h)
	    << "_MaterialProperties[" <<  s.getScalarSize() << "u] = {";
        for(auto i=ib;i!=mprops.first.size();){
	  const auto& m = mprops.first[i];
	  if(m.arraySize==1u){
	    out << "\"" << m.name << "\"";
	  } else {
	    for(unsigned short j=0;j!=m.arraySize;){
	      out << "\"" << m.name << "[" << j << "]\"";
	      if(++j!=m.arraySize){
		out << "," << endl;
	      }
	    }
	  }
	  if(++i!=mprops.first.size()){
	    out << "," << endl;
	  }
	}
	out << "};" << endl << endl;
      }
    }
  } // end of UMATInterface::writeUMATxxMaterialPropertiesSymbol

  void
  UMATInterfaceBase::writeUMATxxStateVariablesSymbols(std::ostream& out,
  							    const std::string& name,
  							    const Hypothesis h,
  							    const BehaviourDescription& mb) const
  {
    using namespace std;
    const auto& d = mb.getBehaviourData(h);
    const auto& persistentVarsHolder = d.getPersistentVariables();
    const unsigned short nStateVariables = static_cast<unsigned short>(this->getNumberOfVariables(persistentVarsHolder));
    VariableDescriptionContainer::const_iterator p;
    out << "MFRONT_SHAREDOBJ unsigned short " << this->getSymbolName(name,h)
  	<< "_nInternalStateVariables = " << nStateVariables
  	<< ";" << endl;
    vector<string> stateVariablesNames;
    mb.getExternalNames(stateVariablesNames,h,persistentVarsHolder);
    this->writeExternalNames(out,name,h,stateVariablesNames,"InternalStateVariables");
    if(!persistentVarsHolder.empty()){
      out << "MFRONT_SHAREDOBJ int " << this->getSymbolName(name,h)
  	  << "_InternalStateVariablesTypes [] = {";
      for(p=persistentVarsHolder.begin();p!=persistentVarsHolder.end();){
  	const auto flag = this->getTypeFlag(p->type);
  	for(unsigned short is=0;is!=p->arraySize;){
  	  switch(flag){
  	  case SupportedTypes::Scalar : 
  	    out << 0;
  	    break;
  	  case SupportedTypes::Stensor :
  	    out << 1;
  	    break;
  	  case SupportedTypes::TVector :
  	    out << 2;
  	    break;
  	  case SupportedTypes::Tensor :
  	    out << 3;
  	    break;
  	  default :
  	    string msg("UMATInterfaceBase::writeUMATxxStateVariablesSymbols : ");
  	    msg += "internal error, tag unsupported for variable '"+p->name+"'";
  	    throw(runtime_error(msg));
  	  }
  	  if(++is!=p->arraySize){
  	    out << ",";
  	  }
  	}
  	if(++p!=persistentVarsHolder.end()){
  	  out << ",";
  	}
      }
      out << "};" << endl << endl;
    } else {
      out << "MFRONT_SHAREDOBJ const int * " << this->getSymbolName(name,h)
  	  << "_InternalStateVariablesTypes = nullptr;" << endl << endl;
    }
  } // end of UMATInterfaceBase::writeUMATxxStateVariablesSymbols
  
  void
  UMATInterfaceBase::writeUMATxxExternalStateVariablesSymbols(std::ostream& out,
  								    const std::string& name,
  								    const Hypothesis h,
  								    const BehaviourDescription& mb) const
  {
    using namespace std;
    const auto& d = mb.getBehaviourData(h);
    const auto& externalStateVarsHolder = d.getExternalStateVariables();
    out << "MFRONT_SHAREDOBJ unsigned short " << this->getSymbolName(name,h)
  	<< "_nExternalStateVariables = " << this->getNumberOfVariables(externalStateVarsHolder) << ";" << endl;
    this->writeExternalNames(out,name,h,mb.getExternalNames(h,externalStateVarsHolder),
  			     "ExternalStateVariables");
  } // end of UMATInterfaceBase::writeUMATxxExternalStateVariablesSymbols

  void
  UMATInterfaceBase::writeUMATxxRequirementsSymbols(std::ostream& out,
							  const std::string& name,
							  const Hypothesis h,
							  const BehaviourDescription& mb) const
  {
    using namespace std;
    out << "MFRONT_SHAREDOBJ unsigned short " << this->getSymbolName(name,h);
    out << "_requiresStiffnessTensor = ";
    if(mb.getAttribute(BehaviourDescription::requiresStiffnessTensor,false)){
      out << "1";
    } else {
      out << "0";
    }
    out << ";" << endl;
    out << "MFRONT_SHAREDOBJ unsigned short " << this->getSymbolName(name,h);
    out << "_requiresThermalExpansionCoefficientTensor = ";
    if(mb.getAttribute(BehaviourDescription::requiresThermalExpansionCoefficientTensor,false)){
      out << "1";
    } else {
      out << "0";
    }
    out << ";" << endl;

  } // end of UMATInterfaceBase::writeUMATxxRequirementsSymbols

  void
  UMATInterfaceBase::writeUMATxxIsUsableInPurelyImplicitResolutionSymbols(std::ostream& out,
  										const std::string& name,
										const Hypothesis h,
  										const BehaviourDescription& mb) const
  {
    using namespace std;
    const auto& d = mb.getBehaviourData(h);
    out << "MFRONT_SHAREDOBJ unsigned short " << this->getSymbolName(name,h)
  	<< "_UsableInPurelyImplicitResolution = ";
    if(d.isUsableInPurelyImplicitResolution()){
      out << "1;" << endl << endl;
    } else {
      out << "0;" << endl << endl;
    }
  } // end of UMATInterfaceBase::writeUMATxxIsUsableInPurelyImplicitResolution

  void
  UMATInterfaceBase::writeUMATxxSymmetryTypeSymbols(std::ostream& out,
  						    const std::string& name,
  						    const BehaviourDescription& mb) const
  {
    using namespace std;
    out << "MFRONT_SHAREDOBJ unsigned short " << this->getFunctionName(name) 
  	<< "_SymmetryType = " ;
    if(mb.getSymmetryType()==mfront::ISOTROPIC){
      out << "0u;" << endl << endl;
    } else if(mb.getSymmetryType()==mfront::ORTHOTROPIC){
      out << "1u;" << endl << endl;
    } else {
      string msg("UMATInterfaceBase::writeUMATxxSymmetryTypeSymbols : ");
      msg += "unsupported behaviour type.\n";
      msg += "only isotropic or orthotropic behaviours are supported at this time.";
      throw(runtime_error(msg));
    }
  } // end of UMATInterfaceBase::writeUMATxxSymmetryTypeSymbols

  void
  UMATInterfaceBase::writeUMATxxElasticSymmetryTypeSymbols(std::ostream& out,
  								  const std::string& name,
  								  const BehaviourDescription& mb) const
  {
    using namespace std;
    out << "MFRONT_SHAREDOBJ unsigned short " << this->getFunctionName(name)
  	<< "_ElasticSymmetryType = " ;
    if(mb.getElasticSymmetryType()==mfront::ISOTROPIC){
      out << "0u;" << endl << endl;
    } else if(mb.getElasticSymmetryType()==mfront::ORTHOTROPIC){
      out << "1u;" << endl << endl;
    } else {
      string msg("UMATInterfaceBase::writeUMATxxElasticSymmetryTypeSymbols : ");
      msg += "unsupported behaviour type.\n";
      msg += "only isotropic or orthotropic behaviours are supported at this time.";
      throw(runtime_error(msg));
    }
  } // end of UMATInterfaceBase::writeUMATxxElasticSymmetryTypeSymbols

  void
  UMATInterfaceBase::writeUMATxxSourceFileSymbols(std::ostream& out,
  							const std::string& name,
  							const BehaviourDescription&,
							const mfront::FileDescription& fd) const
  {
    using namespace std;
    using namespace tfel::system;
    using namespace tfel::utilities;
    out << "MFRONT_SHAREDOBJ const char *" << endl 
  	<< this->getFunctionName(name) << "_src = \""
  	<< tokenize(fd.fileName,dirSeparator()).back()
  	<< "\";" << endl << endl;
  }

  std::map<UMATInterfaceBase::Hypothesis,std::string>
  UMATInterfaceBase::gatherModellingHypothesesAndTests(const BehaviourDescription& mb) const
  {
    using namespace std;
    const auto h = this->getModellingHypothesesToBeTreated(mb);
    auto res = map<Hypothesis,string>{};
    auto h1 = set<Hypothesis>{};
    auto h2 = set<Hypothesis>{};
    for(const auto & elem : h){
      if(!mb.hasSpecialisedMechanicalData(elem)){
	h1.insert(elem);
      } else {
	h2.insert(elem);
      }
    }
    if(!h1.empty()){
      if(h1.size()==1u){
	res.insert({ModellingHypothesis::UNDEFINEDHYPOTHESIS,
	      this->getModellingHypothesisTest(*(h1.begin()))});
      } else {
	auto p = h1.begin();
	string r = "("+this->getModellingHypothesisTest(*(h1.begin()))+")";
	++p;
	for(;p!=h1.end();++p){
	  r += "||("+this->getModellingHypothesisTest(*p)+")";
	}
	res.insert({ModellingHypothesis::UNDEFINEDHYPOTHESIS,r});
      }
    }
    for(const auto & elem : h2){
      res.insert({elem,this->getModellingHypothesisTest(elem)});
    }
    return res;
  } // end of UMATInterface::gatherModellingHypothesesAndTests

  UMATInterfaceBase::~UMATInterfaceBase()
  {}

} // end of namespace mfront
