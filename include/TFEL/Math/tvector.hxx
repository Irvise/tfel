/*!
 * \file   include/TFEL/Math/tvector.hxx
 * \brief
 * \author Thomas Helfer
 * \copyright Copyright (C) 2006-2018 CEA/DEN, EDF R&D. All rights
 * reserved.
 * This project is publicly released under either the GNU GPL Licence
 * or the CECILL-A licence. A copy of thoses licences are delivered
 * with the sources of TFEL. CEA or EDF may also distribute this
 * project under specific licensing conditions.
 */

#ifndef LIB_TFEL_MATH_TVECTOR_HXX
#define LIB_TFEL_MATH_TVECTOR_HXX 1

#include <cstddef>
#include <iterator>
#include <type_traits>

#include "TFEL/Config/TFELConfig.hxx"
#include "TFEL/TypeTraits/IsAssignableTo.hxx"
#include "TFEL/TypeTraits/RealPartType.hxx"
#include "TFEL/FSAlgorithm/copy.hxx"
#include "TFEL/Math/General/Abs.hxx"
#include "TFEL/Math/General/BasicOperations.hxx"
#include "TFEL/Math/General/EmptyRunTimeProperties.hxx"
#include "TFEL/Math/Array/GenericFixedSizeArray.hxx"
#include "TFEL/Math/Array/View.hxx"
#include "TFEL/Math/Array/ViewsArray.hxx"
#include "TFEL/Math/Vector/VectorConcept.hxx"
#include "TFEL/Math/Vector/VectorConceptOperations.hxx"
#include "TFEL/Math/Forward/tmatrix.hxx"
#include "TFEL/Math/Forward/tvector.hxx"
#include "TFEL/Math/fsarray.hxx"

namespace tfel::math {

  /*!
   * An helper class to deal with limitation of Visual Studio 10
   */
  template <typename T, typename T2, typename Op>
  struct IsTVectorScalarOperationValid {
    static constexpr bool cond =
        isScalar<T2>() && std::is_same<result_type<T, T2, OpMult>, T>::value;
  };  // end of struct IsTVectorScalarOperationValid

  template <unsigned short N, typename ValueType = double>
  struct tvector : VectorConcept<tvector<N, ValueType>>,
                   GenericFixedSizeArray<tvector<N, ValueType>,
                                         FixedSizeVectorPolicy<N, ValueType>> {
    //! \brief a simple alias
    using GenericFixedSizeArrayBase =
        GenericFixedSizeArray<tvector<N, ValueType>,
                              FixedSizeVectorPolicy<N, ValueType>>;
    //
    TFEL_MATH_FIXED_SIZE_ARRAY_DEFAULT_METHODS(tvector,
                                               GenericFixedSizeArrayBase);
    /*!
     * \brief implicit conversion from an `fsarray` provided for backward
     * compatibility.
     * \param[in] src: object to be copied
     */
    tvector(const fsarray<N, ValueType>&);
    // inheriting GenericFixedSizeArray' access operators
    using GenericFixedSizeArrayBase::operator[];
    using GenericFixedSizeArrayBase::operator();
    /*!
     * copy the Nth elements following this argument.
     * \param const InputIterator, an iterator to the first element
     * to be copied.
     */
    template <typename InputIterator>
    TFEL_MATH_INLINE void copy(const InputIterator src);
    /*!
     * \brief create a slice
     * \param[in] I : the starting index
     */
    template <unsigned short I>
    constexpr auto slice();
    /*!
     * \brief create a slice
     * \param[in] I : the starting index
     * \param[in] J : the size of the slice
     * \note : the slice object contains a reference to the source
     * vector, so this vector shall not be destroyed before the slice
     */
    template <unsigned short I, unsigned short J>
    constexpr auto slice();
    /*!
     * \brief create a slice (const version)
     * \param[in] I : the starting index
     * \note : the slice object contains a reference to the source
     * vector, so this vector shall not be destroyed before the slice
     */
    template <unsigned short I>
    constexpr auto slice() const;
    /*!
     * \brief create a slice (const version)
     * \param[in] I : the starting index
     * \param[in] J : the size of the slice
     * \note : the slice object contains a reference to the source
     * vector, so this vector shall not be destroyed before the slice
     */
    template <unsigned short I, unsigned short J>
    constexpr auto slice() const;
  };  // end of tvector

  /*!
   * \brief a simple alias for backward compatibility
   * \tparam N: number of values
   * \tparam T: value type
   */
  template <unsigned short N, typename T>
  using TVectorView = View<tvector<N, T>>;
  /*!
   * \brief a simple alias for backward compatibility
   * \tparam N: number of values
   * \tparam T: value type
   */
  template <unsigned short N, typename T>
  using ConstTVectorView = ConstView<tvector<N, T>>;

  /*!
   * \brief create a new tvector by applying a functor
   * \param[in] f: functor
   * \param[in] x: inital value
   */
  template <typename F, typename T, unsigned short N>
  tvector<N, typename std::result_of<F(T)>::type> map(F, const tvector<N, T>&);
  /*!
   * export the given vector to an array of the
   */
  template <unsigned short N, typename T, typename OutputIterator>
  TFEL_MATH_INLINE2 std::enable_if_t<isScalar<T>(), void> exportToBaseTypeArray(
      const tvector<N, T>&, OutputIterator);

  template <unsigned short N, typename T>
  auto abs(const tvector<N, T>& v);

  template <typename T>
  constexpr tvector<1u, T> makeTVector1D(const T);

  template <typename T>
  constexpr tvector<2u, T> makeTVector2D(const T, const T);

  template <typename T>
  constexpr tvector<3u, T> makeTVector3D(const T, const T, const T);

  template <typename T>
  constexpr tvector<3u, T> cross_product(const tvector<2u, T>&,
                                         const tvector<2u, T>&);

  template <typename T>
  constexpr tvector<3u, T> cross_product(const tvector<3u, T>&,
                                         const tvector<3u, T>&);

  /*!
   * find a vector perpendicular to the second one
   */
  template <typename T>
  void find_perpendicular_vector(tvector<3u, T>&, const tvector<3u, T>&);

  /*!
   * \brief create a slice from a tiny vector
   * \param[in] v : vector
   * \note : the slice object contains a reference to the source
   * vector, so this vector shall not be destroyed before the slice
   */
  template <unsigned short I, unsigned short N, typename T>
  constexpr auto slice(tvector<N, T>&);
  /*!
   * \brief create a slice from a tiny vector
   * \param[in] v : vector
   * \note : the slice object contains a reference to the source
   * vector, so this vector shall not be destroyed before the slice
   */
  template <unsigned short I, unsigned short J, unsigned short N, typename T>
  constexpr auto slice(tvector<N, T>&);
  /*!
   * \brief create a slice from a tiny vector
   * \param[in] v : vector
   * \note : the slice object contains a reference to the source
   * vector, so this vector shall not be destroyed before the slice
   */
  template <unsigned short I, unsigned short N, typename T>
  constexpr auto slice(const tvector<N, T>&);
  /*!
   * \brief create a slice from a tiny vector (const version)
   * \param[in] v : vector
   * \note : the slice object contains a reference to the source
   * vector, so this vector shall not be destroyed before the slice
   */
  template <unsigned short I, unsigned short J, unsigned short N, typename T>
  constexpr auto slice(const tvector<N, T>&);

  /*!
   * \brief create a view of a math object from a tiny vector
   * \tparam MappedType : type of mapped object
   * \tparam IndexingPolicyType: index policy type used by the generated view
   * \tparam N: size of the tiny vector
   * \tparam T: type hold by the tiny vector
   */
  template <typename MappedType,
            typename IndexingPolicyType =
                typename std::remove_cv_t<MappedType>::indexing_policy,
            unsigned short N>
  constexpr std::enable_if_t<
      ((!isScalar<MappedType>()) && (IndexingPolicyType::hasFixedSizes) &&
       (checkIndexingPoliciesCompatiblity<
           IndexingPolicyType,
           typename std::remove_cv_t<MappedType>::indexing_policy>())),
      View<MappedType, IndexingPolicyType>>
  map(tvector<N, base_type<numeric_type<MappedType>>>&);

  /*!
   * \brief create a constant view of a math object from a tiny vector
   * \tparam MappedType : type of mapped object
   * \tparam IndexingPolicyType: index policy type used by the generated view
   * \tparam N: size of the tiny vector
   * \tparam T: type hold by the tiny vector
   */
  template <typename MappedType,
            typename IndexingPolicyType =
                typename std::remove_cv_t<MappedType>::indexing_policy,
            unsigned short N>
  constexpr std::enable_if_t<
      ((!isScalar<MappedType>()) && (IndexingPolicyType::hasFixedSizes) &&
       (checkIndexingPoliciesCompatiblity<
           IndexingPolicyType,
           typename std::remove_cv_t<MappedType>::indexing_policy>())),
      View<const MappedType, IndexingPolicyType>>
  map(const tvector<N, base_type<numeric_type<MappedType>>>&);

  /*!
   * \brief create a view of a math object from a tiny vector
   * \tparam MappedType : type of mapped object
   * \tparam offset: offset in the tiny vector
   * \tparam IndexingPolicyType: index policy type used by the generated view
   * \tparam N: size of the tiny vector
   * \tparam T: type hold by the tiny vector
   */
  template <typename MappedType,
            unsigned short offset,
            typename IndexingPolicyType = typename MappedType::indexing_policy,
            unsigned short N,
            typename real>
  constexpr std::enable_if_t<
      ((!std::is_const_v<MappedType>)&&(IndexingPolicyType::hasFixedSizes) &&
       (checkIndexingPoliciesCompatiblity<
           IndexingPolicyType,
           typename MappedType::indexing_policy>())),
      View<MappedType, IndexingPolicyType>>
  map(tvector<N, real>&);

  /*!
   * \brief create a constant view of a math object from a tiny vector
   * \tparam MappedType : type of mapped object
   * \tparam offset: offset in the tiny vector
   * \tparam IndexingPolicyType: index policy type used by the generated view
   * \tparam N: size of the tiny vector
   * \tparam T: type hold by the tiny vector
   */
  template <typename MappedType,
            unsigned short offset,
            typename IndexingPolicyType =
                typename std::remove_cv_t<MappedType>::indexing_policy,
            unsigned short N,
            typename real>
  constexpr std::enable_if_t<
      ((IndexingPolicyType::hasFixedSizes) &&
       (checkIndexingPoliciesCompatiblity<
           IndexingPolicyType,
           typename std::remove_cv_t<MappedType>::indexing_policy>())),
      View<const MappedType, IndexingPolicyType>>
  map(const tvector<N, real>&);

  /*!
   * \brief create a view on an array of fixed sized math objects from a tiny
   * vector
   * \tparam M: number of objects mapped
   * \tparam MappedType : type of mapped object
   * \tparam offset: offset from the start of the tiny vector
   * \tparam N: size of the tiny vector
   */
  template <unsigned short M,
            typename MappedType,
            unsigned short offset = 0u,
            unsigned short stride = getViewsArrayMinimalStride<MappedType>(),
            unsigned short N>
  constexpr std::enable_if_t<
      !std::is_const_v<MappedType>,
      ViewsFixedSizeVector<MappedType, unsigned short, M, stride>>
  map(tvector<N, ViewsArrayNumericType<MappedType>>&);

  /*!
   * \brief create a const view on an array of fixed sized math objects from a
   * tiny vector
   * \tparam M: number of objects mapped
   * \tparam MappedType : type of mapped object
   * \tparam offset: offset from the start of the tiny vector
   * \tparam N: size of the tiny vector
   */
  template <unsigned short M,
            typename MappedType,
            unsigned short offset = 0u,
            unsigned short stride = getViewsArrayMinimalStride<MappedType>(),
            unsigned short N>
  constexpr auto map(const tvector<N, ViewsArrayNumericType<MappedType>>&);

}  // namespace tfel::math

namespace tfel::typetraits {

  //! \brief partial specialisation for tvectors
  template <unsigned short N, typename T2, typename T>
  struct IsAssignableTo<tfel::math::tvector<N, T2>, tfel::math::tvector<N, T>> {
    //! \brief result
    static constexpr bool cond = isAssignableTo<T2, T>();
  };

}  // end of namespace tfel::typetraits

#include "TFEL/Math/Vector/tvector.ixx"
#include "TFEL/Math/Vector/tvectorResultType.hxx"

#endif /* LIB_TFEL_MATH_TVECTOR_HXX */
