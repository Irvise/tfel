/*!
 * \file   T2toST2Concept.hxx  
 * \brief    
 * \author Helfer Thomas
 * \date   19 November 2013
 */

#ifndef _TFEL_MATH_T2TOST2_CONCEPT_LIB_
#define _TFEL_MATH_T2TOST2_CONCEPT_LIB_ 1

#include <ostream>
#include <string>

#include"TFEL/Config/TFELConfig.hxx"

#include"TFEL/Metaprogramming/EnableIf.hxx"
#include"TFEL/Metaprogramming/Implements.hxx"
#include"TFEL/Metaprogramming/InvalidType.hxx"
#include"TFEL/Metaprogramming/IF.hxx"
#include"TFEL/TypeTraits/IsTemporary.hxx"
#include"TFEL/TypeTraits/IsFundamentalNumericType.hxx"
#include"TFEL/TypeTraits/BaseType.hxx"

#include"TFEL/Math/General/Abs.hxx"
#include"TFEL/Math/Tensor/TensorConcept.hxx"
#include"TFEL/Math/Stensor/StensorConcept.hxx"
#include"TFEL/Math/Forward/T2toST2Concept.hxx"

namespace tfel{

  namespace math{

    template<class T>
    struct T2toST2Traits{
      typedef tfel::meta::InvalidType NumType;
      static const unsigned short dime = 0u;
    };

    /*!
     * \class T2tost2Tag
     * \brief Helper class to characterise t2tost2.
     */ 
    struct T2tost2Tag
    {
      /*!
       * \brief  Return the name of the class.
       * \param  void.
       * \return const std::string, the name of the class.
       * \see    Name.
       */
      static std::string 
      getName(void);
    };

    template<class T>
    struct T2toST2Concept 
    {

    private:

      typedef T2toST2Traits<T> traits;
      static const bool isTemporary = tfel::typetraits::IsTemporary<T>::cond;
      typedef typename tfel::meta::IF<isTemporary,
				      typename traits::NumType,
				      const typename traits::NumType&>::type ValueType;

    protected:
      ~T2toST2Concept(){};
      
    public:
      
      typedef T2tost2Tag ConceptTag;

      ValueType
      operator()(const unsigned short,
		 const unsigned short) const;
      
    };

    // Serialisation operator
    template<typename T>
    std::ostream &
    operator << (std::ostream &,
		 const T2toST2Concept<T>&);

    template<typename T>
    struct T2toST2Type{
      typedef T type;
    };

    template<typename T2toST2Type>
    typename tfel::meta::EnableIf<
      tfel::meta::Implements<T2toST2Type,T2toST2Concept>::cond,
      typename tfel::typetraits::AbsType<typename T2toST2Traits<T2toST2Type>::NumType>::type
    >::type
    abs(const T2toST2Type&);

    /*!
     * \brief compute de derivative of the push-forward of a symmetric
     * second order tensor with respect to the deformation gradient
     * knowing the value of this derivative in the initial
     * configuration.
     *
     * This function is typically used to compute the derivative of
     * the Kirchoff stress tensor knowing the derivative of the second
     * Piola-Kirschoff stress.
     *
     * \param[out] : derivative of the push-forward symmetric tensor
     * \param[in]  : derivative of the orginal tensor
     * \param[in]  : orginal tensor (second Piola-Kirschoff stress)
     * \param[in]  : deformation gradient
     */
    template<typename T2toST2ResultType,
   	     typename T2toST2Type,
   	     typename StensorType,
   	     typename TensorType>
    TFEL_MATH_INLINE2
    typename tfel::meta::EnableIf<
      tfel::meta::Implements<T2toST2ResultType,T2toST2Concept>::cond&&
      tfel::meta::Implements<T2toST2Type,T2toST2Concept>::cond&&
      tfel::meta::Implements<StensorType,StensorConcept>::cond&&
      tfel::meta::Implements<TensorType,TensorConcept>::cond&&
      T2toST2Traits<T2toST2ResultType>::dime==1u&&
      T2toST2Traits<T2toST2Type>::dime==1u&&
      StensorTraits<StensorType>::dime==1u&&
      TensorTraits<TensorType>::dime==1u&&
      tfel::typetraits::IsFundamentalNumericType<typename TensorTraits<TensorType>::NumType>::cond&&
      tfel::typetraits::IsAssignableTo<
   	typename ComputeBinaryResult<typename T2toST2Traits<T2toST2Type>::NumType,
   				     typename StensorTraits<StensorType>::NumType,OpPlus>::Result,
   	typename T2toST2Traits<T2toST2ResultType>::NumType>::cond,
      void>::type
    computePushForwardDerivative(T2toST2ResultType&,
   				 const T2toST2Type&,
   				 const StensorType&,
   				 const TensorType&);
    /*!
     * \brief compute de derivative of the push-forward of a symmetric
     * second order tensor with respect to the deformation gradient
     * knowing the value of this derivative in the initial
     * configuration.
     *
     * This function is typically used to compute the derivative of
     * the Kirchoff stress tensor knowing the derivative of the second
     * Piola-Kirschoff stress.
     *
     * \param[out] : derivative of the push-forward symmetric tensor
     * \param[in]  : derivative of the orginal tensor
     * \param[in]  : orginal tensor (second Piola-Kirschoff stress)
     * \param[in]  : deformation gradient
     */
    template<typename T2toST2ResultType,
   	     typename T2toST2Type,
   	     typename StensorType,
   	     typename TensorType>
    TFEL_MATH_INLINE2
    typename tfel::meta::EnableIf<
      tfel::meta::Implements<T2toST2ResultType,T2toST2Concept>::cond&&
      tfel::meta::Implements<T2toST2Type,T2toST2Concept>::cond&&
      tfel::meta::Implements<StensorType,StensorConcept>::cond&&
      tfel::meta::Implements<TensorType,TensorConcept>::cond&&
      T2toST2Traits<T2toST2ResultType>::dime==2u&&
      T2toST2Traits<T2toST2Type>::dime==2u&&
      StensorTraits<StensorType>::dime==2u&&
      TensorTraits<TensorType>::dime==2u&&
      tfel::typetraits::IsFundamentalNumericType<typename TensorTraits<TensorType>::NumType>::cond&&
      tfel::typetraits::IsAssignableTo<
   	typename ComputeBinaryResult<typename T2toST2Traits<T2toST2Type>::NumType,
   				     typename StensorTraits<StensorType>::NumType,OpPlus>::Result,
   	typename T2toST2Traits<T2toST2ResultType>::NumType>::cond,
      void>::type
    computePushForwardDerivative(T2toST2ResultType&,
   				 const T2toST2Type&,
   				 const StensorType&,
   				 const TensorType&);
    /*!
     * \brief compute de derivative of the push-forward of a symmetric
     * second order tensor with respect to the deformation gradient
     * knowing the value of this derivative in the initial
     * configuration.
     *
     * This function is typically used to compute the derivative of
     * the Kirchoff stress tensor knowing the derivative of the second
     * Piola-Kirschoff stress.
     *
     * \param[out] : derivative of the push-forward symmetric tensor
     * \param[in]  : derivative of the orginal tensor
     * \param[in]  : orginal tensor (second Piola-Kirschoff stress)
     * \param[in]  : deformation gradient
     */
    template<typename T2toST2ResultType,
   	     typename T2toST2Type,
   	     typename StensorType,
   	     typename TensorType>
    TFEL_MATH_INLINE2
    typename tfel::meta::EnableIf<
      tfel::meta::Implements<T2toST2ResultType,T2toST2Concept>::cond&&
      tfel::meta::Implements<T2toST2Type,T2toST2Concept>::cond&&
      tfel::meta::Implements<StensorType,StensorConcept>::cond&&
      tfel::meta::Implements<TensorType,TensorConcept>::cond&&
      T2toST2Traits<T2toST2ResultType>::dime==3u&&
      T2toST2Traits<T2toST2Type>::dime==3u&&
      StensorTraits<StensorType>::dime==3u&&
      TensorTraits<TensorType>::dime==3u&&
      tfel::typetraits::IsFundamentalNumericType<typename TensorTraits<TensorType>::NumType>::cond&&
      tfel::typetraits::IsAssignableTo<
   	typename ComputeBinaryResult<typename T2toST2Traits<T2toST2Type>::NumType,
   				     typename StensorTraits<StensorType>::NumType,OpPlus>::Result,
   	typename T2toST2Traits<T2toST2ResultType>::NumType>::cond,
      void>::type
    computePushForwardDerivative(T2toST2ResultType&,
   				 const T2toST2Type&,
   				 const StensorType&,
   				 const TensorType&);
    /*!
     * \brief compute the Cauchy stress derivative from the Kirchoff
     * stress derivative with respect to the deformation gradient
     * \param[out] ds   : Cauchy stress derivative
     * \param[in]  dt_K : Kirchoff stress derivative
     * \param[in]  s    : Cauchy stress
     * \param[in]  F    : deformation gradient
     */
    template<typename T2toST2ResultType,
   	     typename T2toST2Type,
   	     typename StensorType,
   	     typename TensorType>
    TFEL_MATH_INLINE
    typename tfel::meta::EnableIf<
      tfel::meta::Implements<T2toST2ResultType,T2toST2Concept>::cond&&
      tfel::meta::Implements<T2toST2Type,T2toST2Concept>::cond&&
      tfel::meta::Implements<StensorType,StensorConcept>::cond&&
      tfel::meta::Implements<TensorType,TensorConcept>::cond&&
      T2toST2Traits<T2toST2ResultType>::dime==T2toST2Traits<T2toST2Type>::dime&&
      T2toST2Traits<T2toST2ResultType>::dime==StensorTraits<StensorType>::dime&&
      T2toST2Traits<T2toST2ResultType>::dime==TensorTraits<TensorType>::dime&&
      tfel::typetraits::IsFundamentalNumericType<typename TensorTraits<TensorType>::NumType>::cond&&
      tfel::typetraits::IsAssignableTo<
   	typename ComputeBinaryResult<typename T2toST2Traits<T2toST2Type>::NumType,
   				     typename StensorTraits<StensorType>::NumType,OpPlus>::Result,
   	typename T2toST2Traits<T2toST2ResultType>::NumType>::cond,
      void>::type
    computeCauchyStressDerivativeFromKirchoffDerivative(T2toST2ResultType&,
							const T2toST2Type&,
							const StensorType&,
							const TensorType&);
    /*!
     * \brief compute the Kirchhoff stress derivative from the Cauchy
     * stress derivative with respect to the deformation gradient
     * \param[out] dt_K : Kirchoff stress derivative
     * \param[in]  ds   : Cauchy stress derivative
     * \param[in]  s    : Cauchy stress
     * \param[in]  F    : deformation gradient
     */
    template<typename T2toST2ResultType,
   	     typename T2toST2Type,
   	     typename StensorType,
   	     typename TensorType>
    TFEL_MATH_INLINE
    typename tfel::meta::EnableIf<
      tfel::meta::Implements<T2toST2ResultType,T2toST2Concept>::cond&&
      tfel::meta::Implements<T2toST2Type,T2toST2Concept>::cond&&
      tfel::meta::Implements<StensorType,StensorConcept>::cond&&
      tfel::meta::Implements<TensorType,TensorConcept>::cond&&
      T2toST2Traits<T2toST2ResultType>::dime==T2toST2Traits<T2toST2Type>::dime&&
      T2toST2Traits<T2toST2ResultType>::dime==StensorTraits<StensorType>::dime&&
      T2toST2Traits<T2toST2ResultType>::dime==TensorTraits<TensorType>::dime&&
      tfel::typetraits::IsFundamentalNumericType<typename TensorTraits<TensorType>::NumType>::cond&&
      tfel::typetraits::IsAssignableTo<
   	typename ComputeBinaryResult<typename T2toST2Traits<T2toST2Type>::NumType,
   				     typename StensorTraits<StensorType>::NumType,OpPlus>::Result,
   	typename T2toST2Traits<T2toST2ResultType>::NumType>::cond,
      void>::type
    computeKirchoffDerivativeFromCauchyStressDerivative(T2toST2ResultType&,
							const T2toST2Type&,
							const StensorType&,
							const TensorType&);

  } // end of namespace math

} // end of namespace tfel

#include"TFEL/Math/T2toST2/T2toST2Concept.ixx"

#endif /* _LIB_TFEL_MATH_T2TOST2_CONCEPT_LIB_ */
