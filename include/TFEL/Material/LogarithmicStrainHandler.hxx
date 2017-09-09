/*!
 * \file   LogarithmicStrainHandler.hxx
 * \brief    
 * \author HELFER Thomas 202608
 * \date   25 juin 2017
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef LIB_TFEL_MATH_LOGARITHMICSTRAINHANDLER_HXX
#define LIB_TFEL_MATH_LOGARITHMICSTRAINHANDLER_HXX

#include"TFEL/Math/tensor.hxx"
#include"TFEL/Math/tvector.hxx"
#include"TFEL/Math/tmatrix.hxx"
#include"TFEL/Math/stensor.hxx"
#include"TFEL/Math/st2tost2.hxx"

namespace tfel
{

  namespace material
  {

    /*!
     * \brief A base class for all specialisations of the
     * `LogarithmicStrainHandler` class.
     */
    struct TFELMATERIAL_VISIBILITY_EXPORT LogarithmicStrainHandlerBase
    {
      enum Setting{
	LAGRANGIAN,
	EULERIAN
      }; // end of enum Setting
    protected:
      /*!
       * \brief constructor
       * \param[in] c: setting
       */
      TFEL_INLINE LogarithmicStrainHandlerBase(const Setting);
      /*!
       * \brief throw an exception if the setting is not equal to
       * `LAGRANGIAN`.
       */
      void checkLagrangianSetting() const;
      /*!
       * \brief throw an exception if the setting is not equal to
       * `EULERIAN`.
       */
      void checkEulerianSetting() const;
      //! setting
      const Setting s;
    }; // end of struct LogarithmicStrainHandlerBase

    /*!
     * \brief This class gathers various methods used to
     * \tparam N: space dimension
     * \tparam StressType: type used to describe the stress.
     */
    template<unsigned short N,typename StressType>
    struct LogarithmicStrainHandler;

    /*!
     * \brief partial specialisation of the `LogarithmicStrainHandler`
     * class for 3D.
     */
    template<typename StressType>
    struct LogarithmicStrainHandler<1u,StressType>
      : public LogarithmicStrainHandlerBase
    {
      //! numeric type
      using real = tfel::typetraits::base_type<StressType>;
      //! a simple alias
      using stress = StressType;
      //! a simple alias
      using DeformationGradient = tfel::math::tensor<1u,real>;
      //! a simple alias
      using StrainStensor = tfel::math::stensor<1u,real>;
      //! a simple alias
      using StressStensor = tfel::math::stensor<1u,stress>;
      //! a simple alias
      using TangentOperator = tfel::math::st2tost2<1u,stress>;
      //! a simple alias
      using size_type = unsigned short;
      //! numerical parameter used to distinguish egal eigenvalues.
      static constexpr const real eps = 1.e-14;
      /*!
       * \brief constructor
       * \param[in] c:  setting
       * \param[in] F1: deformation gradient
       */
      LogarithmicStrainHandler(const Setting,
			       const DeformationGradient&);
      //! \return the the logarithmic strain
      StrainStensor getHenckyLogarithmicStrain() const;
      /*!
       * \param[out] elog: the logarithmic strain in `Abaqus/Standard` conventions
       */
      void getHenckyLogarithmicStrain(real *const) const;
      /*!
       * \return the Cauchy stress from the dual of the logarithmic strain
       * \param[in] T: dual of the logarithmic strain
       */
      StressStensor
      convertToSecondPiolaKirchhoffStress(const StressStensor&) const;
      /*!
       * \brief compute the second Piola-Kirchhoff stress from the
       * dual of the logarithmic strain.
       * \param[in,out] TS: stress in `Abaqus/Standard` conventions.
       */
      void convertToSecondPiolaKirchhoffStress(stress *const) const;
      /*!
       * \return the dual of the logarithmic strain
       * \param[in] S: Second Piola Kirchhoff stress.
       */
      StressStensor
      convertFromSecondPiolaKirchhoffStress(const StressStensor&) const;
      /*!
       * \brief compute the dual of the logarithmic strain from the
       * second Piola-Kirchhoff stress from.
       * \param[in,out] ST: stress in `Abaqus/Standard` conventions.
       */
      void convertFromSecondPiolaKirchhoffStress(stress *const) const;
      /*!
       * \return the Cauchy stress from the dual of the logarithmic strain
       * \param[in] T: dual of the logarithmic strain
       */
      StressStensor convertToCauchyStress(const StressStensor&) const;
      /*!
       * \brief compute the Cauchy stress from the dual of the
       * logarithmic strain.
       * \param[in,out] Ts: stress in `Abaqus/Standard` conventions.
       */
      void convertToCauchyStress(stress *const) const;
      /*!
       * \return the dual of the logarithmic strain
       * \param[in] s: Cauchy stress.
       */
      StressStensor convertFromCauchyStress(const StressStensor&) const;
      /*!
       * \brief compute the dual of the logarithmic strain from the
       * Cauchy stress.
       * \param[in,out] sT: stress in `Abaqus/Standard` conventions.
       */
      void convertFromCauchyStress(stress *const) const;
      /*!
       * \return the material tangent moduli
       * \param[in] Ks: tangent moduli in the logarithmic space
       * \param[in]  T: dual of the logarithmic strain
       */
      TangentOperator
      convertToMaterialTangentModuli(const TangentOperator&,
				     const StressStensor&) const;
      /*!
       * \return the spatial tangent moduli
       * \param[in] Ks: tangent moduli in the logarithmic space
       * \param[in]  T: dual of the logarithmic strain
       */
      TangentOperator
      convertToSpatialTangentModuli(const TangentOperator&,
				    const StressStensor&) const;
      /*!
       * \return the moduli associated with the Truesdell' rate of the
       * Cauchy stress.
       * \param[in] Ks: tangent moduli in the logarithmic space
       * \param[in]  T: dual of the logarithmic strain
       */
      TangentOperator
      convertToCauchyStressTruesdellRateTangentModuli(const TangentOperator&,
						      const StressStensor&) const;
      /*!
       * \param[in,out] K: On input, the consistent tangent operator
       * in the logarithmic space. On output, the moduli associated
       * with the Truesdell' rate of the Cauchy stress. Both are
       * expressed in the Abaqus/Standard conventions.
       * \param[in] T: dual of the logarithmic strain in
       * Abaqus/Standard conventions.
       */
      void
      convertToCauchyStressTruesdellRateTangentModuli(stress *const,
						      const stress* const) const;
    private:
      //! deformation gradient
      DeformationGradient F;
      /*!
       * \brief half the logarithm of the eigenvalues of the right
       * Cauchy Green tensor
       */
      tfel::math::tvector<3u,real> e;
    }; // end of LogarithmicStrainHandler<1u,StressType>

    /*!
     * \brief partial specialisation of the `LogarithmicStrainHandler`
     * class for 2D.
     */
    template<typename StressType>
    struct LogarithmicStrainHandler<2u,StressType>
      : public LogarithmicStrainHandlerBase
    {
      //! numeric type
      using real = tfel::typetraits::base_type<StressType>;
      //! a simple alias
      using stress = StressType;
      //! a simple alias
      using DeformationGradient = tfel::math::tensor<2u,real>;
      //! a simple alias
      using StrainStensor = tfel::math::stensor<2u,real>;
      //! a simple alias
      using StressStensor = tfel::math::stensor<2u,stress>;
      //! a simple alias
      using TangentOperator = tfel::math::st2tost2<2u,stress>;
      //! a simple alias
      using size_type = unsigned short;
      //! criterion used to check if eigenvalues are equal.
      static constexpr const real eps = 1.e-14;
      /*!
       * \brief constructor
       * \param[in] c:  setting
       * \param[in] F1: deformation gradient
       * \param[in] b: compute the axial component. This component is
       * unused in plane stress analyses.
       */
      LogarithmicStrainHandler(const Setting,
			       const DeformationGradient&,
			       const bool = true);
      /*!
       * \brief update the axial deformation gradient
       * \param[in] Fzz: axial deformation gradient
       * \note This only updates the deformation gradient, not the
       * Hencky strain, nor the eigen values and so on.  This is
       * because the whole deformation gradient is only needed for
       * stress and tangent moduli conversion.
       */
      void updateAxialDeformationGradient(const real);
      /*!
       * \return the Cauchy stress from the dual of the logarithmic strain
       * \param[in] T: dual of the logarithmic strain
       */
      StressStensor
      convertToSecondPiolaKirchhoffStress(const StressStensor&) const;
      /*!
       * \brief compute the second Piola-Kirchhoff stress from the
       * dual of the logarithmic strain.
       * \param[in,out] TS: stress in `Abaqus/Standard` conventions.
       */
      void convertToSecondPiolaKirchhoffStress(stress *const) const;
      //! \return the the logarithmic strain
      StrainStensor getHenckyLogarithmicStrain() const;
      /*!
       * \param[out] elog: the logarithmic strain in `Abaqus/Standard`
       * conventions.
       */
      void getHenckyLogarithmicStrain(real *const) const;
      /*!
       * \return the dual of the logarithmic strain
       * \param[in] S: Second Piola Kirchhoff stress.
       */
      StressStensor
      convertFromSecondPiolaKirchhoffStress(const StressStensor&) const;
      /*!
       * \brief compute the dual of the logarithmic strain from the
       * second Piola-Kirchhoff stress from.
       * \param[in,out] ST: stress in `Abaqus/Standard` conventions.
       */
      void convertFromSecondPiolaKirchhoffStress(stress *const) const;
      /*!
       * \return the Cauchy stress from the dual of the logarithmic strain
       * \param[in] T: dual of the logarithmic strain
       */
      StressStensor convertToCauchyStress(const StressStensor&) const;
      /*!
       * \brief compute the Cauchy stress from the dual of the
       * logarithmic strain.
       * \param[in,out] Ts: stress in `Abaqus/Standard` conventions.
       */
      void convertToCauchyStress(stress *const) const;
      /*!
       * \return the dual of the logarithmic strain
       * \param[in] s: Cauchy stress.
       */
      StressStensor convertFromCauchyStress(const StressStensor&) const;
      /*!
       * \brief compute the dual of the logarithmic strain from the
       * Cauchy stress.
       * \param[in,out] sT: stress in `Abaqus/Standard` conventions.
       */
      void convertFromCauchyStress(stress *const) const;
      /*!
       * \return the material tangent moduli
       * \param[in] Ks: tangent moduli in the logarithmic space
       * \param[in]  T: dual of the logarithmic strain
       */
      TangentOperator
      convertToMaterialTangentModuli(const TangentOperator&,
				     const StressStensor&) const;
      /*!
       * \return the spatial tangent moduli
       * \param[in] Ks: tangent moduli in the logarithmic space
       * \param[in]  T: dual of the logarithmic strain
       */
      TangentOperator
      convertToSpatialTangentModuli(const TangentOperator&,
				    const StressStensor&) const;
      /*!
       * \return the moduli associated with the Truesdell' rate of the
       * Cauchy stress.
       * \param[in] Ks: tangent moduli in the logarithmic space
       * \param[in]  T: dual of the logarithmic strain
       */
      TangentOperator
      convertToCauchyStressTruesdellRateTangentModuli(const TangentOperator&,
						      const StressStensor&) const;
      /*!
       * \param[in,out] K: On input, the consistent tangent operator
       * in the logarithmic space. On output, the moduli associated
       * with the Truesdell' rate of the Cauchy stress. Both are
       * expressed in the Abaqus/Standard conventions.
       * \param[in] T: dual of the logarithmic strain in
       * Abaqus/Standard conventions.
       */
      void
      convertToCauchyStressTruesdellRateTangentModuli(stress *const,
						      const stress* const) const;
    private:
      /*!
       * \brief compute the N tensors
       * \param[in] m: eigenvectors of the right Cauchy-Green tensor
       * \param[in] F: deformation gradient
       */
      static tfel::math::tvector<4u,tfel::math::stensor<2u,real>>
      getNTensors(const tfel::math::tmatrix<3u,3u,real>&);
      /*!
       * \brief compute the M tensors in the Eulerian setting.
       * \param[in] m: eigenvectors of the right Cauchy-Green tensor
       * \param[in] F: deformation gradient
       */
      static tfel::math::tvector<4u,tfel::math::stensor<2u,real>>
      getEulerianMTensors(const tfel::math::tmatrix<3u,3u,real>&,
			  const DeformationGradient&);
      /*!
       * \brief: an helper class used to build the a
       * `LogarithmicStrainHandler` object.
       */
      struct Builder{
	/*!
	 * \brief constructor
	 * \param[in] c:  setting
	 * \param[in] F1: deformation gradient
	 * \param[in] b: compute the axial component. This component is
	 * unused in plane stress analyses.
	 */
	Builder(const Setting,
		const DeformationGradient&,
		const bool);
	/*!
	 * derivative of the logarithmic strain with respect to the
	 * right Cauchy-Green tensor
	 */
	tfel::math::st2tost2<2u,real> p;
	//! eigenvectors of the right Cauchy-Green tensor
	tfel::math::tmatrix<3u,3u,real>  m;
	//! eigenvalues of the right Cauchy-Green tensor
	tfel::math::tvector<3u,real>    vp;
	//! half the logarithm of the eigenvalues
	tfel::math::tvector<3u,real>    e;
      };
      /*!
       * \brief build from the `LogarithmicStrainHandlerBuilder`.
       * This trick is used to ensure that the members of the
       * `LogarithmicStrainHandler` class are `const`.
       * \param[in] b:  builder object
       * \param[in] c:  setting
       * \param[in] F1: deformation gradient
       */ 
      LogarithmicStrainHandler(Builder&&,
			       const Setting,
			       const DeformationGradient&);
      /*!
       * \brief general method to convert the tangent operator,
       * whatever the setting used.
       * \param[out] Kr: tangent moduli to be computed
       * \param[in]  Ks: tangent moduli in the logarithmic space
       * \param[in]   T: dual of the logarithmic strain
       * \param[in]   N: set of tensors (meaning depends on the setting)
       * \param[in]   M: set of tensors (meaning depends on the setting)
       */
      void convertTangentModuli(TangentOperator&,
				const TangentOperator&,
				const StressStensor&,
				const tfel::math::tvector<4u,tfel::math::stensor<2u,real>>&,
				const tfel::math::tvector<4u,tfel::math::stensor<2u,real>>&) const;
      /*!
       * derivative of the logarithmic strain with respect to the
       * right Cauchy-Green tensor
       */
      const tfel::math::st2tost2<2u,real> p;
      //! deformation gradient
      DeformationGradient F;
      //! eigenvectors of the right Cauchy-Green tensor
      const tfel::math::tmatrix<3u,3u,real>  m;
      //! eigenvalues of the right Cauchy-Green tensor
      const tfel::math::tvector<3u,real>    vp;
      //! eigenvalues of the right Cauchy-Green tensor
      const tfel::math::tvector<3u,real>    e;
    };
    
    /*!
     * \brief partial specialisation of the `LogarithmicStrainHandler`
     * class for 3D.
     */
    template<typename StressType>
    struct LogarithmicStrainHandler<3u,StressType>
      : public LogarithmicStrainHandlerBase
    {
      //! numeric type
      using real = tfel::typetraits::base_type<StressType>;
      //! a simple alias
      using stress = StressType;
      //! a simple alias
      using DeformationGradient = tfel::math::tensor<3u,real>;
      //! a simple alias
      using StrainStensor = tfel::math::stensor<3u,real>;
      //! a simple alias
      using StressStensor = tfel::math::stensor<3u,stress>;
      //! a simple alias
      using TangentOperator = tfel::math::st2tost2<3u,stress>;
      //! a simple alias
      using size_type = unsigned short;
      //! numerical parameter used to distinguish egal eigenvalues.
      static constexpr const real eps = 1.e-14;
      /*!
       * \brief constructor
       * \param[in] c:  setting
       * \param[in] F1: deformation gradient
       */
      LogarithmicStrainHandler(const Setting,
			       const DeformationGradient&);
      //! \return the the logarithmic strain
      StrainStensor getHenckyLogarithmicStrain() const;
      /*!
       * \param[out] elog: the logarithmic strain in `Abaqus/Standard` conventions
       */
      void getHenckyLogarithmicStrain(real *const) const;
      /*!
       * \return the Cauchy stress from the dual of the logarithmic strain
       * \param[in] T: dual of the logarithmic strain
       */
      StressStensor
      convertToSecondPiolaKirchhoffStress(const StressStensor&) const;
      /*!
       * \brief compute the second Piola-Kirchhoff stress from the
       * dual of the logarithmic strain.
       * \param[in,out] TS: stress in `Abaqus/Standard` conventions.
       */
      void convertToSecondPiolaKirchhoffStress(stress *const) const;
      /*!
       * \return the dual of the logarithmic strain
       * \param[in] S: Second Piola Kirchhoff stress.
       */
      StressStensor
      convertFromSecondPiolaKirchhoffStress(const StressStensor&) const;
      /*!
       * \brief compute the dual of the logarithmic strain from the
       * second Piola-Kirchhoff stress from.
       * \param[in,out] ST: stress in `Abaqus/Standard` conventions.
       */
      void convertFromSecondPiolaKirchhoffStress(stress *const) const;
      /*!
       * \return the Cauchy stress from the dual of the logarithmic strain
       * \param[in] T: dual of the logarithmic strain
       */
      StressStensor convertToCauchyStress(const StressStensor&) const;
      /*!
       * \brief compute the Cauchy stress from the dual of the
       * logarithmic strain.
       * \param[in,out] Ts: stress in `Abaqus/Standard` conventions.
       */
      void convertToCauchyStress(stress *const) const;
      /*!
       * \return the dual of the logarithmic strain
       * \param[in] s: Cauchy stress.
       */
      StressStensor convertFromCauchyStress(const StressStensor&) const;
      /*!
       * \brief compute the dual of the logarithmic strain from the
       * Cauchy stress.
       * \param[in,out] sT: stress in `Abaqus/Standard` conventions.
       */
      void convertFromCauchyStress(stress *const) const;
      /*!
       * \return the material tangent moduli
       * \param[in] Ks: tangent moduli in the logarithmic space
       * \param[in]  T: dual of the logarithmic strain
       */
      TangentOperator
      convertToMaterialTangentModuli(const TangentOperator&,
				     const StressStensor&) const;
      /*!
       * \return the spatial tangent moduli
       * \param[in] Ks: tangent moduli in the logarithmic space
       * \param[in]  T: dual of the logarithmic strain
       */
      TangentOperator
      convertToSpatialTangentModuli(const TangentOperator&,
				    const StressStensor&) const;
      /*!
       * \return the moduli associated with the Truesdell' rate of the
       * Cauchy stress.
       * \param[in] Ks: tangent moduli in the logarithmic space
       * \param[in]  T: dual of the logarithmic strain
       */
      TangentOperator
      convertToCauchyStressTruesdellRateTangentModuli(const TangentOperator&,
						      const StressStensor&) const;
      /*!
       * \param[in,out] K: On input, the consistent tangent operator
       * in the logarithmic space. On output, the moduli associated
       * with the Truesdell' rate of the Cauchy stress. Both are
       * expressed in the Abaqus/Standard conventions.
       * \param[in] T: dual of the logarithmic strain in
       * Abaqus/Standard conventions.
       */
      void
      convertToCauchyStressTruesdellRateTangentModuli(stress *const,
						      const stress* const) const;
    private:
      //! \return true if all eigenvalues are equal
      static bool areEigenValuesEqual(const tfel::math::tvector<3u,real>&);
      /*!
       * \return the index of the single value if two eigenvalues are
       * equal, 3 otherwise.
       */
      static size_type findSingleEigenValue(const tfel::math::tvector<3u,real>&);
      /*!
       * \brief compute the N tensors
       * \param[in] m: eigenvectors of the right Cauchy-Green tensor
       * \param[in] F: deformation gradient
       */
      static tfel::math::tmatrix<3u,3u,tfel::math::stensor<3u,real>>
      getNTensors(const tfel::math::tmatrix<3u,3u,real>&);
      /*!
       * \brief compute the M tensors in the Eulerian setting.
       * \param[in] m: eigenvectors of the right Cauchy-Green tensor
       * \param[in] F: deformation gradient
       */
      static tfel::math::tmatrix<3u,3u,tfel::math::stensor<3u,real>>
      getEulerianMTensors(const tfel::math::tmatrix<3u,3u,real>&,
			  const DeformationGradient&);
      /*!
       * \brief: an helper class used to build the a
       * `LogarithmicStrainHandler` object.
       */
      struct Builder{
	/*!
	 * \brief constructor
	 * \param[in] c:  setting
	 * \param[in] F1: deformation gradient
	 */
	Builder(const Setting,
		const DeformationGradient&);
	/*!
	 * derivative of the logarithmic strain with respect to the
	 * right Cauchy-Green tensor
	 */
	tfel::math::st2tost2<3u,real> p;
	//! eigenvectors of the right Cauchy-Green tensor
	tfel::math::tmatrix<3u,3u,real>  m;
	//! eigenvalues of the right Cauchy-Green tensor
	tfel::math::tvector<3u,real>    vp;
	//! half the logarithm of the eigenvalues
	tfel::math::tvector<3u,real>    e;
      };
      /*!
       * \brief build from the `LogarithmicStrainHandlerBuilder`.
       * This trick is used to ensure that the members of the
       * `LogarithmicStrainHandler` class are `const`.
       * \param[in] b: builder object
       * \param[in] c:  setting
       * \param[in] F1: deformation gradient
       */ 
      LogarithmicStrainHandler(Builder&&,
			       const Setting,
			       const DeformationGradient&);
      /*!
       * \brief general method to convert the tangent operator,
       * whatever the setting used.
       * \param[out] Kr: tangent moduli to be computed
       * \param[in]  Ks: tangent moduli in the logarithmic space
       * \param[in]   T: dual of the logarithmic strain
       * \param[in]   N: set of tensors (meaning depends on the setting)
       * \param[in]   M: set of tensors (meaning depends on the setting)
       */
      void convertTangentModuli(TangentOperator&,
				const TangentOperator&,
				const StressStensor&,
				const tfel::math::tmatrix<3u,3u,tfel::math::stensor<3u,real>>&,
				const tfel::math::tmatrix<3u,3u,tfel::math::stensor<3u,real>>&) const;
      /*!
       * derivative of the logarithmic strain with respect to the
       * right Cauchy-Green tensor
       */
      const tfel::math::st2tost2<3u,real> p;
      //! deformation gradient
      const DeformationGradient F;
      //! eigenvectors of the right Cauchy-Green tensor
      const tfel::math::tmatrix<3u,3u,real>  m;
      //! eigenvalues of the right Cauchy-Green tensor
      const tfel::math::tvector<3u,real>    vp;
      //! eigenvalues of the right Cauchy-Green tensor
      const tfel::math::tvector<3u,real>    e;
    };
    
  } // end of namespace material

} // end of namespace material

#include"TFEL/Material/LogarithmicStrainHandler.ixx"

#endif /* LIB_TFEL_MATH_LOGARITHMICSTRAINHANDLER_HXX */
