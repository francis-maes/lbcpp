/*-----------------------------------------.---------------------------------.
| Filename: DoubleVector.hpp               | Static Double Vectors           |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 18:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_STATIC_DOUBLE_VECTOR_HPP_
# define LBCPP_STATIC_DOUBLE_VECTOR_HPP_

# include "FeatureGenerator.hpp"
# include "../FeatureVisitor/FeatureVisitorStaticToDynamic.hpp"
# include "../../SparseVector.h"
# include "../../DenseVector.h"

namespace lbcpp
{

/*
** Scalar
*/
template<class FeatureVisitor>
inline void Scalar::staticFeatureGenerator(FeatureVisitor& visitor) const
  {visitor.featureSense(getDictionary(), 0, value);}

template<>
struct Traits<ScalarPtr> : public ObjectPtrTraits<Scalar> {};
template<>
struct Traits<Scalar> : public ObjectTraits<Scalar> {};

/*
** Label
*/
template<class FeatureVisitor>
inline void Label::staticFeatureGenerator(FeatureVisitor& visitor) const
  {visitor.featureSense(getDictionary(), index, 1.0);}

template<>
struct Traits<LabelPtr> : public ObjectPtrTraits<Label> {};
template<>
struct Traits<Label> : public ObjectTraits<Label> {};

/*
** Sparse Vector
*/
template<class FeatureVisitor>
inline void SparseVector::staticFeatureGenerator(FeatureVisitor& visitor) const
{
  for (size_t i = 0; i < values.size(); ++i)
  {
    const std::pair<size_t, double>& feature = values[i];
    visitor.featureSense(dictionary, feature.first, feature.second);
  }
  
  for (size_t i = 0; i < subVectors.size(); ++i)
  {
    const std::pair<size_t, SparseVectorPtr>& subVector = subVectors[i];
    if (subVector.second && visitor.featureEnter(dictionary, subVector.first, subVector.second->getDictionary(), 1.0))
    {
      subVector.second->staticFeatureGenerator(visitor);
      visitor.featureLeave();
    }
  }
}

template<>
struct Traits<SparseVectorPtr> : public ObjectPtrTraits<SparseVector> {};
template<>
struct Traits<SparseVector> : public ObjectTraits<SparseVector> {};

/*
** Dense Vector
*/
template<class FeatureVisitor>
inline void DenseVector::staticFeatureGenerator(FeatureVisitor& visitor) const
{
  for (size_t i = 0; i < values.size(); ++i)
    if (values[i])
      visitor.featureSense(dictionary, i, values[i]);
  
  for (size_t i = 0; i < subVectors.size(); ++i)
  {
    DenseVectorPtr subVector = subVectors[i];
    if (subVector && visitor.featureEnter(dictionary, i, subVector->getDictionary(), 1.0))
    {
      subVector->staticFeatureGenerator(visitor);
      visitor.featureLeave();
    }
  }
}

template<>
struct Traits<DenseVectorPtr> : public ObjectPtrTraits<DenseVector> {};
template<>
struct Traits<DenseVector> : public ObjectTraits<DenseVector> {};

/*
** Double Vector
*/
template<class FeatureVisitor>
inline void EditableFeatureGenerator::staticFeatureGenerator(FeatureVisitor& visitor) const
{
  const FeatureVector* vector = dynamic_cast<const FeatureVector* >(this);
  if (vector)
  {
    vector->staticFeatureGenerator(visitor);
    return;
  }
  const LazyFeatureVector* lazyVector = dynamic_cast<const LazyFeatureVector* >(this);
  if (lazyVector)
  {
    lazyVector->staticFeatureGenerator(visitor);
    return;
  }
  
  // dynamic version must be implemented
  accept(impl::staticToDynamic(visitor));
}

template<>
struct Traits<EditableFeatureGeneratorPtr> : public ObjectPtrTraits<EditableFeatureGenerator> {};

template<class FeatureVisitor>
inline void FeatureVector::staticFeatureGenerator(FeatureVisitor& visitor) const
{
  const SparseVector* sparseVector = dynamic_cast<const SparseVector* >(this);
  if (sparseVector)
  {
    sparseVector->staticFeatureGenerator(visitor);
    return;
  }
  const DenseVector* denseVector = dynamic_cast<const DenseVector* >(this);
  if (denseVector)
  {
    denseVector->staticFeatureGenerator(visitor);
    return;
  }
  
  // dynamic version must be implemented
  accept(impl::staticToDynamic(visitor));
}

template<>
struct Traits<FeatureVectorPtr> : public ObjectPtrTraits<FeatureVector> {};

}; /* namespace lbcpp */

#endif // !LBCPP_STATIC_DOUBLE_VECTOR_HPP_
