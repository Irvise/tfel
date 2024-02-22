/*!
 * \file   mfront/src/BehaviourCodeGeneratorBase-BehaviourData.cxx
 * \brief  This file implements the methods of the BehaviourCodeGeneratorBase
 * associated with the generation of the BehaviourData class
 * \author Thomas Helfer
 * \date   22/02/2024
 */

#include <ostream>
#include "TFEL/Raise.hxx"
#include "TFEL/UnicodeSupport/UnicodeSupport.hxx"
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "MFront/MFrontHeader.hxx"
#include "MFront/DSLUtilities.hxx"
#include "MFront/MFrontUtilities.hxx"
#include "MFront/MFrontDebugMode.hxx"
#include "MFront/PedanticMode.hxx"
#include "MFront/SupportedTypes.hxx"
#include "MFront/MFrontLogStream.hxx"
#include "MFront/FileDescription.hxx"
#include "MFront/BehaviourDescription.hxx"
#include "MFront/AbstractBehaviourInterface.hxx"
#include "MFront/BehaviourCodeGeneratorBase.hxx"

namespace mfront {

  void BehaviourCodeGeneratorBase::checkBehaviourDataFile(
      std::ostream& os) const {
    if ((!os) || (!os.good())) {
      this->throwRuntimeError(
          "BehaviourCodeGeneratorBase::checkBehaviourDataOutputFile",
          "output file is not valid");
    }
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataMainVariablesSetters(
      std::ostream& os) const {
    this->checkBehaviourDataFile(os);
    for (const auto& i : this->interfaces) {
      i.second->writeBehaviourDataMainVariablesSetters(os, this->bd);
      os << '\n';
    }
  }  // end of writeBehaviourDataMainVariablesSetters

  void BehaviourCodeGeneratorBase::writeBehaviourDataFileHeader(
      std::ostream& os) const {
    this->checkBehaviourDataFile(os);
    os << "/*!\n"
       << "* \\file   " << this->getBehaviourDataFileName() << '\n'
       << "* \\brief  this file implements the " << this->bd.getClassName()
       << "BehaviourData class.\n"
       << "*         File generated by " << MFrontHeader::getVersionName()
       << " "
       << "version " << MFrontHeader::getVersionNumber() << '\n';
    if (!this->fd.authorName.empty()) {
      os << "* \\author " << this->fd.authorName << '\n';
    }
    if (!this->fd.date.empty()) {
      os << "* \\date   " << this->fd.date << '\n';
    }
    os << " */\n\n";
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataFileHeaderBegin(
      std::ostream& os) const {
    this->checkBehaviourDataFile(os);
    os << "#ifndef LIB_TFELMATERIAL_" << makeUpperCase(this->bd.getClassName())
       << "_BEHAVIOUR_DATA_HXX\n";
    os << "#define LIB_TFELMATERIAL_" << makeUpperCase(this->bd.getClassName())
       << "_BEHAVIOUR_DATA_HXX\n\n";
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataFileHeaderEnd(
      std::ostream& os) const {
    this->checkBehaviourDataFile(os);
    os << "#endif /* LIB_TFELMATERIAL_"
       << makeUpperCase(this->bd.getClassName()) << "_BEHAVIOUR_DATA_HXX */\n";
  }

  static bool hasVariableOfType(const BehaviourData& bd,
                                const SupportedTypes::TypeFlag f) {
    auto update = [f](const auto& variables) {
      const auto& flags = SupportedTypes::getTypeFlags();
      for (const auto& v : variables) {
        const auto pf = flags.find(v.type);
        if (pf == flags.end()) {
          continue;
        }
        if (pf->second == f) {
          return true;
        }
      }
      return false;
    };
    if ((update(bd.getMaterialProperties())) ||
        (update(bd.getIntegrationVariables())) ||
        (update(bd.getStateVariables())) ||
        (update(bd.getAuxiliaryStateVariables())) ||
        //        (update(bd.getLocalVariables())) ||
        (update(bd.getExternalStateVariables())) ||
        (update(bd.getInitializeFunctionVariables())) ||
        (update(bd.getPostProcessingVariables()))) {
      return true;
    }
    return false;
  }  // end of hasVariableOfType

  static bool hasVariableOfType(const BehaviourDescription& bd,
                                const SupportedTypes::TypeFlag f) {
    using ModellingHypothesis = BehaviourDescription::ModellingHypothesis;
    for (const auto& mv : bd.getMainVariables()) {
      if (mv.first.getTypeFlag() == f) {
        return true;
      }
      if (mv.second.getTypeFlag() == f) {
        return true;
      }
    }
    if (!bd.areAllMechanicalDataSpecialised()) {
      return hasVariableOfType(
          bd.getBehaviourData(ModellingHypothesis::UNDEFINEDHYPOTHESIS), f);
    }
    for (const auto& h : bd.getDistinctModellingHypotheses()) {
      if (hasVariableOfType(bd.getBehaviourData(h), f)) {
        return true;
      }
    }
    return false;
  }  // end of requiresTVectorOrVectorIncludes

  void BehaviourCodeGeneratorBase::writeBehaviourDataStandardTFELIncludes(
      std::ostream& os) const {
    auto b1 = false;
    auto b2 = false;
    this->checkBehaviourDataFile(os);
    os << "#include<limits>\n"
       << "#include<string>\n"
       << "#include<sstream>\n"
       << "#include<iostream>\n"
       << "#include<stdexcept>\n"
       << "#include<algorithm>\n\n"
       << "#include\"TFEL/Raise.hxx\"\n"
       << "#include\"TFEL/PhysicalConstants.hxx\"\n"
       << "#include\"TFEL/Config/TFELConfig.hxx\"\n"
       << "#include\"TFEL/Config/TFELTypes.hxx\"\n"
       << "#include\"TFEL/TypeTraits/IsFundamentalNumericType.hxx\"\n"
       << "#include\"TFEL/TypeTraits/IsReal.hxx\"\n"
       << "#include\"TFEL/Math/General/Abs.hxx\"\n"
       << "#include\"TFEL/Math/General/IEEE754.hxx\"\n"
       << "#include\"TFEL/Math/Array/ViewsArrayIO.hxx\"\n"
       << "#include\"TFEL/Math/Array/fsarrayIO.hxx\"\n"
       << "#include\"TFEL/Math/Array/runtime_arrayIO.hxx\"\n"
       << "#include\"TFEL/Math/fsarray.hxx\"\n"
       << "#include\"TFEL/Math/runtime_array.hxx\"\n";
    if (this->bd.useQt()) {
      os << "#include\"TFEL/Math/qt.hxx\"\n";
      os << "#include\"TFEL/Math/Quantity/qtIO.hxx\"\n";
    }
    this->bd.requiresTVectorOrVectorIncludes(b1, b2);
    if (b1) {
      os << "#include\"TFEL/Math/tvector.hxx\"\n"
         << "#include\"TFEL/Math/Vector/tvectorIO.hxx\"\n";
    }
    if (b2) {
      os << "#include\"TFEL/Math/vector.hxx\"\n";
    }
    os << "#include\"TFEL/Math/tmatrix.hxx\"\n"
       << "#include\"TFEL/Math/Matrix/tmatrixIO.hxx\"\n";
    if (hasVariableOfType(this->bd, SupportedTypes::STENSOR)) {
      os << "#include\"TFEL/Math/stensor.hxx\"\n"
         << "#include\"TFEL/Math/Stensor/StensorConceptIO.hxx\"\n"
         << "#include\"TFEL/Math/st2tost2.hxx\"\n"
         << "#include\"TFEL/Math/ST2toST2/ST2toST2ConceptIO.hxx\"\n";
    }
    if (hasVariableOfType(this->bd, SupportedTypes::TENSOR)) {
      os << "#include\"TFEL/Math/tensor.hxx\"\n"
         << "#include\"TFEL/Math/Tensor/TensorConceptIO.hxx\"\n"
         << "#include\"TFEL/Math/t2tot2.hxx\"\n"
         << "#include\"TFEL/Math/T2toT2/T2toT2ConceptIO.hxx\"\n";
    }
    if ((hasVariableOfType(this->bd, SupportedTypes::STENSOR)) &&
        (hasVariableOfType(this->bd, SupportedTypes::TENSOR))) {
      os << "#include\"TFEL/Math/t2tost2.hxx\"\n"
         << "#include\"TFEL/Math/T2toST2/T2toST2ConceptIO.hxx\"\n"
         << "#include\"TFEL/Math/st2tot2.hxx\"\n"
         << "#include\"TFEL/Math/ST2toT2/ST2toT2ConceptIO.hxx\"\n";
    }
    if ((this->bd.getBehaviourType() ==
         BehaviourDescription::STANDARDFINITESTRAINBEHAVIOUR) ||
        (this->bd.getBehaviourType() ==
         BehaviourDescription::GENERALBEHAVIOUR)) {
      os << "#include\"TFEL/Math/ST2toST2/ConvertToTangentModuli.hxx\"\n"
         << "#include\"TFEL/Math/ST2toST2/"
            "ConvertSpatialModuliToKirchhoffJaumanRateModuli.hxx\"\n"
         << "#include\"TFEL/Material/"
            "FiniteStrainBehaviourTangentOperator.hxx\"\n";
    }
    os << "#include\"TFEL/Material/ModellingHypothesis.hxx\"\n\n";
  }  // end of writeBehaviourDataStandardTFELIncludes

  void BehaviourCodeGeneratorBase::writeBehaviourDataDefaultMembers(
      std::ostream& os) const {
    this->checkBehaviourDataFile(os);
    if (this->bd.getAttribute(BehaviourDescription::requiresStiffnessTensor,
                              false)) {
      os << "//! stiffness tensor computed by the calling solver\n"
         << "StiffnessTensor D;\n";
    }
    if (this->bd.getAttribute(
            BehaviourDescription::requiresThermalExpansionCoefficientTensor,
            false)) {
      os << "ThermalExpansionCoefficientTensor A;\n";
    }
    for (const auto& mv : this->bd.getMainVariables()) {
      checkArraySizeOfMainVariables(
          "BehaviourCodeGeneratorBase::"
          "writeBehaviourDataAssignementOperator",
          mv);
      if (mv.first.arraySize == 1) {
        if (Gradient::isIncrementKnown(mv.first)) {
          os << mv.first.type << " " << mv.first.name << ";\n\n";
        } else {
          os << mv.first.type << " " << mv.first.name << "0;\n\n";
        }
        os << mv.second.type << " " << mv.second.name << ";\n\n";
      } else {
        if (Gradient::isIncrementKnown(mv.first)) {
          os << mv.first.type << " " << mv.first.name << "["
             << mv.first.arraySize << "];\n\n";
        } else {
          os << mv.first.type << " " << mv.first.name << "0["
             << mv.first.arraySize << "];\n\n";
        }
        os << mv.second.type << " " << mv.second.name << "["
           << mv.second.arraySize << "];\n\n";
      }
    }
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataTypeAliases(
      std::ostream& os) const {
    this->checkBehaviourDataFile(os);
    os << "static constexpr unsigned short TVectorSize = N;\n"
       << "using StensorDimeToSize = tfel::math::StensorDimeToSize<N>;\n"
       << "static constexpr unsigned short StensorSize = "
       << "StensorDimeToSize::value;\n"
       << "using TensorDimeToSize = tfel::math::TensorDimeToSize<N>;\n"
       << "static constexpr unsigned short TensorSize = "
       << "TensorDimeToSize::value;\n\n";
    this->writeTypeAliases(os);
    os << '\n';
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataDisabledConstructors(
      std::ostream& os) const {
    this->checkBehaviourDataFile(os);
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataConstructors(
      std::ostream& os, const Hypothesis h) const {
    const auto& md = this->bd.getBehaviourData(h);
    this->checkBehaviourDataFile(os);
    os << "/*!\n"
       << "* \\brief Default constructor\n"
       << "*/\n"
       << this->bd.getClassName() << "BehaviourData()\n"
       << "{}\n\n"
       << "/*!\n"
       << "* \\brief copy constructor\n"
       << "*/\n"
       << this->bd.getClassName() << "BehaviourData(const "
       << this->bd.getClassName() << "BehaviourData& src)\n"
       << ": ";
    auto first = true;
    if (this->bd.getAttribute(BehaviourDescription::requiresStiffnessTensor,
                              false)) {
      os << "D(src.D)";
      first = false;
    }
    if (this->bd.getAttribute(
            BehaviourDescription::requiresThermalExpansionCoefficientTensor,
            false)) {
      if (!first) {
        os << ",\n";
      }
      os << "A(src.A)";
      first = false;
    }
    for (const auto& mv : this->bd.getMainVariables()) {
      if (!first) {
        os << ",\n";
      }
      if (Gradient::isIncrementKnown(mv.first)) {
        os << mv.first.name << "(src." << mv.first.name << "),\n";
      } else {
        os << mv.first.name << "0(src." << mv.first.name << "0),\n";
      }
      os << mv.second.name << "(src." << mv.second.name << ")";
      first = false;
    }
    for (const auto& v : md.getMaterialProperties()) {
      if (!first) {
        os << ",\n";
      }
      os << v.name << "(src." << v.name << ")";
      first = false;
    }
    for (const auto& v : md.getStateVariables()) {
      if (!first) {
        os << ",\n";
      }
      os << v.name << "(src." << v.name << ")";
      first = false;
    }
    for (const auto& v : md.getAuxiliaryStateVariables()) {
      if (!first) {
        os << ",\n";
      }
      os << v.name << "(src." << v.name << ")";
      first = false;
    }
    for (const auto& v : md.getExternalStateVariables()) {
      if (!first) {
        os << ",\n";
      }
      os << v.name << "(src." << v.name << ")";
      first = false;
    }
    os << "\n{}\n\n";
    // Creating constructor for external interfaces
    for (const auto& i : this->interfaces) {
      if (i.second->isBehaviourConstructorRequired(h, this->bd)) {
        i.second->writeBehaviourDataConstructor(os, h, this->bd);
      }
    }
  }  // end of writeBehaviourDataConstructors

  void BehaviourCodeGeneratorBase::writeBehaviourDataAssignementOperator(
      std::ostream& os, const Hypothesis h) const {
    const auto& md = this->bd.getBehaviourData(h);
    this->checkBehaviourDataFile(os);
    os << "/*\n"
       << "* \\brief Assignement operator\n"
       << "*/\n"
       << this->bd.getClassName() << "BehaviourData&\n"
       << "operator=(const " << this->bd.getClassName()
       << "BehaviourData& src){\n";
    for (const auto& dv : this->bd.getMainVariables()) {
      checkArraySizeOfMainVariables(
          "BehaviourCodeGeneratorBase::"
          "writeBehaviourDataAssignementOperator",
          dv);
      const auto getArrayIndex = [&dv](const auto idx) -> std::string {
        if (dv.first.arraySize == 1) {
          return "";
        }
        return '[' + std::to_string(idx) + ']';
      };
      for (unsigned short idx = 0; idx != dv.first.arraySize; ++idx) {
        const auto aidx = getArrayIndex(idx);
        if (Gradient::isIncrementKnown(dv.first)) {
          os << "this->" << dv.first.name << aidx << " = src." << dv.first.name
             << aidx << ";\n";
        } else {
          os << "this->" << dv.first.name << "0" << aidx << " = src."
             << dv.first.name << "0" << aidx << ";\n";
        }
        os << "this->" << dv.second.name << aidx << " = "
           << "src." << dv.second.name << ";\n"
           << "this->" << dv.second.name << aidx << " = "
           << "src." << dv.second.name << ";\n";
      }
    }
    for (const auto& mp : md.getMaterialProperties()) {
      os << "this->" << mp.name << " = src." << mp.name << ";\n";
    }
    for (const auto& iv : md.getStateVariables()) {
      os << "this->" << iv.name << " = src." << iv.name << ";\n";
    }
    for (const auto& iv : md.getAuxiliaryStateVariables()) {
      os << "this->" << iv.name << " = src." << iv.name << ";\n";
    }
    for (const auto& ev : md.getExternalStateVariables()) {
      os << "this->" << ev.name << " = src." << ev.name << ";\n";
    }
    os << "return *this;\n"
       << "}\n\n";
  }  // end of writeBehaviourAssignementOperator

  void BehaviourCodeGeneratorBase::writeBehaviourDataExport(
      std::ostream& os, const Hypothesis h) const {
    this->checkBehaviourDataFile(os);
    for (const auto& i : this->interfaces) {
      i.second->exportMechanicalData(os, h, this->bd);
    }
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataInitializeMethods(
      std::ostream& os, const Hypothesis h) const {
    this->checkBehaviourDataFile(os);
    const auto& d = this->bd.getBehaviourData(h);
    for (const auto& n : d.getUserDefinedInitializeCodeBlocksNames()) {
      const auto& c = d.getUserDefinedInitializeCodeBlock(n);
      os << "void initialize" << n << "(){\n"
         << "using namespace std;\n"
         << "using namespace tfel::math;\n"
         << "using std::vector;\n";
      writeMaterialLaws(os, this->bd.getMaterialLaws());
      os << c.code;
      os << "} // end of initialize" << n << "\n\n";
    }
  }  // end of writeBehaviourDataInitializeMethods

  void BehaviourCodeGeneratorBase::writeBehaviourDataPublicMembers(
      std::ostream& os) const {
    this->checkBehaviourDataFile(os);
    if (this->bd.getAttribute(BehaviourDescription::requiresStiffnessTensor,
                              false)) {
      os << "StiffnessTensor& getStiffnessTensor()\n"
         << "{\nreturn this->D;\n}\n\n"
         << "const StiffnessTensor& getStiffnessTensor() const\n"
         << "{\nreturn this->D;\n}\n\n";
    }
    if (this->bd.getAttribute(
            BehaviourDescription::requiresThermalExpansionCoefficientTensor,
            false)) {
      os << "ThermalExpansionCoefficientTensor& "
         << "getThermalExpansionCoefficientTensor()\n"
         << "{\nreturn this->A;\n}\n\n"
         << "const ThermalExpansionCoefficientTensor& "
         << "getThermalExpansionCoefficientTensor() const\n"
         << "{\nreturn this->A;\n}\n\n";
    }
  }  // end of writeBehaviourDataPublicMembers

  void BehaviourCodeGeneratorBase::writeBehaviourDataClassHeader(
      std::ostream& os) const {
    this->checkBehaviourDataFile(os);
    os << "/*!\n"
       << "* \\class " << this->bd.getClassName() << "BehaviourData\n"
       << "* \\brief This class implements the " << this->bd.getClassName()
       << "BehaviourData"
       << " .\n"
       << "* \\tparam H: modelling hypothesis.\n"
       << "* \\tparam NumericType: numerical type.\n"
       << "* \\tparam use_qt: conditional saying if quantities are use.\n";
    if (!this->fd.authorName.empty()) {
      os << "* \\author " << this->fd.authorName << '\n';
    }
    if (!this->fd.date.empty()) {
      os << "* \\date   " << this->fd.date << '\n';
    }
    os << "*/\n";
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataForwardDeclarations(
      std::ostream& os) const {
    this->checkBehaviourDataFile(os);
    os << "//! \\brief forward declaration\n"
       << "template<ModellingHypothesis::Hypothesis hypothesis,typename,bool>\n"
       << "class " << this->bd.getClassName() << "BehaviourData;\n\n"
       << "//! \\brief forward declaration\n"
       << "template<ModellingHypothesis::Hypothesis hypothesis, "
       << "typename NumericType,bool use_qt>\n"
       << "class " << this->bd.getClassName() << "IntegrationData;\n\n";
    if (this->bd.useQt()) {
      os << "//! \\brief forward declaration\n";
      os << "template<ModellingHypothesis::Hypothesis hypothesis, "
         << "typename NumericType, bool use_qt>\n";
      os << "std::ostream&\n operator <<(std::ostream&,";
      os << "const " << this->bd.getClassName()
         << "BehaviourData<hypothesis, NumericType, use_qt>&);\n\n";
    } else {
      os << "//! \\brief forward declaration\n";
      os << "template<ModellingHypothesis::Hypothesis hypothesis,"
         << "typename NumericType>\n";
      os << "std::ostream&\n operator <<(std::ostream&,";
      os << "const " << this->bd.getClassName()
         << "BehaviourData<hypothesis, NumericType,false>&);\n\n";
    }
    // maintenant, il faut déclarer toutes les spécialisations partielles...
    for (const auto& h : this->bd.getModellingHypotheses()) {
      if (this->bd.hasSpecialisedMechanicalData(h)) {
        if (this->bd.useQt()) {
          os << "//! \\brief forward declaration\n"
             << "template<typename NumericType,bool use_qt>\n"
             << "std::ostream&\n operator <<(std::ostream&,"
             << "const " << this->bd.getClassName()
             << "BehaviourData<ModellingHypothesis::"
             << ModellingHypothesis::toUpperCaseString(h)
             << ", NumericType, use_qt>&);\n\n";
        } else {
          os << "//! \\brief forward declaration\n"
             << "template<typename NumericType>\n"
             << "std::ostream&\n operator <<(std::ostream&,"
             << "const " << this->bd.getClassName()
             << "BehaviourData<ModellingHypothesis::"
             << ModellingHypothesis::toUpperCaseString(h)
             << ", NumericType, false>&);\n\n";
        }
      }
    }
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataClassBegin(
      std::ostream& os, const Hypothesis h) const {
    this->checkBehaviourDataFile(os);
    if (h == ModellingHypothesis::UNDEFINEDHYPOTHESIS) {
      if (this->bd.useQt()) {
        os << "template<ModellingHypothesis::Hypothesis hypothesis,"
           << "typename NumericType,bool use_qt>\n";
        os << "class " << this->bd.getClassName() << "BehaviourData\n";
      } else {
        os << "template<ModellingHypothesis::Hypothesis hypothesis, "
           << "typename NumericType>\n";
        os << "class " << this->bd.getClassName()
           << "BehaviourData<hypothesis, NumericType,false>\n";
      }
    } else {
      if (this->bd.useQt()) {
        os << "template<typename NumericType,bool use_qt>\n";
        os << "class " << this->bd.getClassName()
           << "BehaviourData<ModellingHypothesis::"
           << ModellingHypothesis::toUpperCaseString(h)
           << ", NumericType, use_qt>\n";
      } else {
        os << "template<typename NumericType>\n";
        os << "class " << this->bd.getClassName()
           << "BehaviourData<ModellingHypothesis::"
           << ModellingHypothesis::toUpperCaseString(h)
           << ", NumericType, false>\n";
      }
    }
    os << "{\n\n";
    if (h != ModellingHypothesis::UNDEFINEDHYPOTHESIS) {
      os << "static constexpr ModellingHypothesis::Hypothesis hypothesis = "
         << "ModellingHypothesis::" << ModellingHypothesis::toUpperCaseString(h)
         << ";\n";
    }
    os << "static constexpr unsigned short N = "
       << "ModellingHypothesisToSpaceDimension<hypothesis>::value;\n"
       << "static_assert(N==1||N==2||N==3);\n"
       << "static_assert(tfel::typetraits::"
       << "IsFundamentalNumericType<NumericType>::cond);\n"
       << "static_assert(tfel::typetraits::IsReal<NumericType>::cond);\n\n"
       << "friend std::ostream& operator<< <>(std::ostream&,const "
       << this->bd.getClassName() << "BehaviourData&);\n\n"
       << "/* integration data is declared friend to access"
       << "   driving variables at the beginning of the time step */\n";
    if (this->bd.useQt()) {
      os << "friend class " << this->bd.getClassName()
         << "IntegrationData<hypothesis, NumericType, use_qt>;\n\n";
    } else {
      os << "friend class " << this->bd.getClassName()
         << "IntegrationData<hypothesis, NumericType, false>;\n\n";
    }
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataClassEnd(
      std::ostream& os) const {
    this->checkBehaviourDataFile(os);
    os << "}; // end of " << this->bd.getClassName() << "BehaviourData"
       << "class\n\n";
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataMaterialProperties(
      std::ostream& os, const Hypothesis h) const {
    this->checkBehaviourDataFile(os);
    this->writeVariablesDeclarations(
        os, this->bd.getBehaviourData(h).getMaterialProperties(), "", "",
        this->fd.fileName, false);
    os << '\n';
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataStateVariables(
      std::ostream& os, const Hypothesis h) const {
    this->checkBehaviourDataFile(os);
    const auto& d = this->bd.getBehaviourData(h);
    this->writeVariablesDeclarations(os, d.getStateVariables(), "", "",
                                     this->fd.fileName, false);
    this->writeVariablesDeclarations(os, d.getAuxiliaryStateVariables(), "", "",
                                     this->fd.fileName, false);
    this->writeVariablesDeclarations(os, d.getExternalStateVariables(), "", "",
                                     this->fd.fileName, false);
    os << '\n';
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataOutputOperator(
      std::ostream& os, const Hypothesis h) const {
    this->checkBehaviourFile(os);
    const auto& d = this->bd.getBehaviourData(h);
    if (h == ModellingHypothesis::UNDEFINEDHYPOTHESIS) {
      if (this->bd.useQt()) {
        os << "template<ModellingHypothesis::Hypothesis hypothesis,"
           << "typename NumericType, bool use_qt>\n";
        os << "std::ostream&\n";
        os << "operator <<(std::ostream& os,";
        os << "const " << this->bd.getClassName()
           << "BehaviourData<hypothesis, NumericType, use_qt>& b)\n";
      } else {
        os << "template<ModellingHypothesis::Hypothesis hypothesis, "
           << "typename NumericType>\n";
        os << "std::ostream&\n";
        os << "operator <<(std::ostream& os,";
        os << "const " << this->bd.getClassName()
           << "BehaviourData<hypothesis, NumericType, false>& b)\n";
      }
    } else {
      if (this->bd.useQt()) {
        os << "template<typename NumericType, bool use_qt>\n";
        os << "std::ostream&\n";
        os << "operator <<(std::ostream& os,";
        os << "const " << this->bd.getClassName()
           << "BehaviourData<ModellingHypothesis::"
           << ModellingHypothesis::toUpperCaseString(h)
           << ", NumericType, use_qt>& b)\n";
      } else {
        os << "template<typename NumericType>\n";
        os << "std::ostream&\n";
        os << "operator <<(std::ostream& os,";
        os << "const " << this->bd.getClassName()
           << "BehaviourData<ModellingHypothesis::"
           << ModellingHypothesis::toUpperCaseString(h)
           << ", NumericType, false>& b)\n";
      }
    }
    os << "{\n";
    for (const auto& v : this->bd.getMainVariables()) {
      if (Gradient::isIncrementKnown(v.first)) {
        os << "os << \"" << displayName(v.first) << " : \" << b."
           << v.first.name << " << '\\n';\n";
      } else {
        if (getUnicodeOutputOption()) {
          os << "os << \"" << displayName(v.first) << "\u2080 : \" << b."
             << v.first.name << "0 << '\\n';\n";
        } else {
          os << "os << \"" << displayName(v.first) << "0 : \" << b."
             << v.first.name << "0 << '\\n';\n";
        }
      }
      os << "os << \"" << displayName(v.second) << " : \" << b."
         << v.second.name << " << '\\n';\n";
    }
    for (const auto& v : d.getMaterialProperties()) {
      os << "os << \"" << displayName(v) << " : \" << b." << v.name
         << " << '\\n';\n";
    }
    for (const auto& v : d.getStateVariables()) {
      os << "os << \"" << displayName(v) << " : \" << b." << v.name
         << " << '\\n';\n";
    }
    for (const auto& v : d.getAuxiliaryStateVariables()) {
      os << "os << \"" << displayName(v) << " : \" << b." << v.name
         << " << '\\n';\n";
    }
    for (const auto& v : d.getExternalStateVariables()) {
      os << "os << \"" << displayName(v) << " : \" << b." << v.name
         << " << '\\n';\n";
    }
    os << "return os;\n"
       << "}\n\n";
  }  //  BehaviourCodeGeneratorBase::writeBehaviourDataOutputOperator

  void BehaviourCodeGeneratorBase::writeBehaviourDataFileBegin(
      std::ostream& os) const {
    this->checkBehaviourDataFile(os);
    this->writeBehaviourDataFileHeader(os);
    this->writeBehaviourDataFileHeaderBegin(os);
    this->writeBehaviourDataStandardTFELIncludes(os);
    this->writeIncludes(os);
    // includes specific to interfaces
    for (const auto& i : this->interfaces) {
      i.second->writeInterfaceSpecificIncludes(os, this->bd);
    }
    this->writeNamespaceBegin(os);
    this->writeBehaviourDataForwardDeclarations(os);
  }  // end of writeBehaviourDataFile

  void BehaviourCodeGeneratorBase::writeBehaviourDataClass(
      std::ostream& os, const Hypothesis h) const {
    this->checkBehaviourDataFile(os);
    this->writeBehaviourDataClassBegin(os, h);
    this->writeBehaviourDataTypeAliases(os);
    os << "protected:\n\n";
    this->writeBehaviourDataDefaultMembers(os);
    this->writeBehaviourDataMaterialProperties(os, h);
    this->writeBehaviourDataStateVariables(os, h);
    os << "public:\n\n";
    this->writeBehaviourDataDisabledConstructors(os);
    this->writeBehaviourDataConstructors(os, h);
    this->writeBehaviourDataMainVariablesSetters(os);
    this->writeBehaviourDataPublicMembers(os);
    this->writeBehaviourDataAssignementOperator(os, h);
    this->writeBehaviourDataInitializeMethods(os, h);
    this->writeBehaviourDataExport(os, h);
    this->writeBehaviourDataClassEnd(os);
    this->writeBehaviourDataOutputOperator(os, h);
  }

  void BehaviourCodeGeneratorBase::writeBehaviourDataFileEnd(
      std::ostream& os) const {
    this->writeNamespaceEnd(os);
    this->writeBehaviourDataFileHeaderEnd(os);
  }  // end of writeBehaviourDataFileEnd

}  // namespace mfront