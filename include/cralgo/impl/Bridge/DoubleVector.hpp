/*-----------------------------------------.---------------------------------.
| Filename: DoubleVector.hpp               | Static Double Vectors           |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 18:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_STATIC_DOUBLE_VECTOR_HPP_
# define CRALGO_STATIC_DOUBLE_VECTOR_HPP_

# include "FeatureGenerator.hpp"
# include "../FeatureVisitor/FeatureVisitorStaticToDynamic.hpp"
# include "../../SparseVector.h"
# include "../../DenseVector.h"
# include "../../LazyVector.h"

namespace cralgo
{

/*
** Sparse Vector
*/
template<class FeatureVisitor>
inline void SparseVector::staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionary& featureDictionary) const
{
  for (size_t i = 0; i < values.size(); ++i)
  {
    const std::pair<size_t, double>& feature = values[i];
    visitor.featureSense(featureDictionary, feature.first, feature.second);
  }
  
  for (size_t i = 0; i < subVectors.size(); ++i)
  {
    const std::pair<size_t, SparseVectorPtr>& subVector = subVectors[i];
    if (visitor.featureEnter(featureDictionary, subVector.first))
    {
      //assert(subVector.second);
      if (subVector.second)
        subVector.second->staticFeatureGenerator(visitor, featureDictionary.getSubDictionary(subVector.first));
      visitor.featureLeave();
    }
  }
}

template<>
struct Traits<SparseVectorPtr> : public ObjectPtrTraits<SparseVector> {};

/*
** Dense Vector
*/
template<class FeatureVisitor>
inline void DenseVector::staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionary& featureDictionary) const
{
  for (size_t i = 0; i < values.size(); ++i)
    if (values[i])
      visitor.featureSense(featureDictionary, i, values[i]);
  
  for (size_t i = 0; i < subVectors.size(); ++i)
    if (subVectors[i] && visitor.featureEnter(featureDictionary, i))
    {
      subVectors[i]->staticFeatureGenerator(visitor, featureDictionary.getSubDictionary(i));
      visitor.featureLeave();
    }
}

template<>
struct Traits<DenseVectorPtr> : public ObjectPtrTraits<DenseVector> {};

/*
** Lazy Vector
*/
template<class FeatureVisitor>
inline void LazyVector::staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionary& featureDictionary) const
{
  if (isStoredWithFeatureGenerator())
    visitor.featureCall(featureDictionary, featureGenerator);
  else if (guessIfDense())
  {
    const_cast<LazyVector* >(this)->storeWithDenseVector();
    visitor.featureCall(featureDictionary, denseVector);
  }
  else
  {
    const_cast<LazyVector* >(this)->storeWithSparseVector();
    visitor.featureCall(featureDictionary, sparseVector);
  }
}

template<>
struct Traits<LazyVectorPtr> : public ObjectPtrTraits<LazyVector> {};

/*
** Double Vector
*/
template<class FeatureVisitor>
inline void DoubleVector::staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionary& featureDictionary) const
{
  const SparseVector* sparseVector = dynamic_cast<const SparseVector* >(this);
  if (sparseVector)
  {
    sparseVector->staticFeatureGenerator(visitor, featureDictionary);
    return;
  }
  const DenseVector* denseVector = dynamic_cast<const DenseVector* >(this);
  if (denseVector)
  {
    denseVector->staticFeatureGenerator(visitor, featureDictionary);
    return;
  }
  const LazyVector* lazyVector = dynamic_cast<const LazyVector* >(this);
  if (lazyVector)
  {
    lazyVector->staticFeatureGenerator(visitor, featureDictionary);
    return;
  }
  // dynamic version must be implemented
  accept(impl::staticToDynamic(visitor), &featureDictionary);
}

template<>
struct Traits<DoubleVectorPtr> : public ObjectPtrTraits<DoubleVector> {};

}; /* namespace cralgo */

#endif // !CRALGO_STATIC_DOUBLE_VECTOR_HPP_
