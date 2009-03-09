/*-----------------------------------------.---------------------------------.
| Filename: LazyVector.h                   | Lazy Vectors                    |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 18:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_LAZY_VECTOR_H_
# define CRALGO_LAZY_VECTOR_H_

# include "SparseVector.h"
# include "DenseVector.h"

namespace cralgo
{

class LazyVector;
typedef ReferenceCountedObjectPtr<LazyVector> LazyVectorPtr;

class LazyVector : public FeatureGeneratorDefaultImplementations<LazyVector, DoubleVector>
{
public:
  void clear()
  {
    combination.clear();
    value = DenseVectorPtr();
  }
  
  virtual DenseVectorPtr toDenseVector()
  {
    // FIXME
    return DenseVectorPtr();
  }
  
  /*
  ** Sub lazy-vectors
  */
  LazyVectorPtr& getSubVector(size_t i)
  {
    static LazyVectorPtr fixme;
    return fixme;
  }

  /*
  ** Operations
  */
  void add(const FeatureGeneratorPtr features)
    {combination[features] += 1;}
  
  void addWeighted(const FeatureGeneratorPtr features, double weight)
    {combination[features] += weight;}
    
  void addWeighted(const LazyVectorPtr vector, double weight)
  {
    for (LinearCombinationMap::const_iterator it = vector->combination.begin(); it != vector->combination.end(); ++it)
      combination[it->first] += it->second * weight;
  }

  void multiplyByScalar(double scalar)
  {
    // todo: improve
    for (LinearCombinationMap::iterator it = combination.begin(); it != combination.end(); ++it)
      it->second *= scalar;
//    if (value)
//      value->multiplyByScalar(scalar);
  }
  
  void set(DenseVectorPtr denseVector)
  {
    combination.clear();
    combination[denseVector] = 1;
  }
  
  void set(size_t index, double value)
  {
    ensureComputed();
    // FIXME
    //value->set(index, value);
  }
  

  /*
  ** Static FeatureGenerator
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionary& featureDictionary) const;

  /*
  ** FeatureGenerator
  */
  virtual std::string getName() const
    {return "LazyVector";}
    
  virtual FeatureDictionary& getDefaultDictionary() const
    {static FeatureDictionary defaultDictionary("LazyVector"); return dictionary ? *dictionary : defaultDictionary;}

  virtual void addWeightedTo(DenseVectorPtr target, double weight) const
  {
    for (LinearCombinationMap::const_iterator it = combination.begin(); it != combination.end(); ++it)
      it->first->addWeightedTo(target, weight * it->second);
    if (value)
      value->addWeightedTo(target, weight);
  }
  
private:
  // sparseVector + denseVector + linear-combinaison-of-feature-generators + sub-lazy-vectors

  typedef std::map<const FeatureGeneratorPtr, double> LinearCombinationMap;
  LinearCombinationMap combination;
  DoubleVectorPtr value;
  
  void ensureComputed()
  {
    if (combination.size())
    {
      FeatureDictionary& dictionary = combination.begin()->first->getDefaultDictionary();
      DenseVectorPtr denseVector(new DenseVector(dictionary));
      for (LinearCombinationMap::const_iterator it = combination.begin(); it != combination.end(); ++it)
        it->first->addWeightedTo(denseVector, it->second);
      combination.clear();
      value = denseVector;
    }
  }
};

}; /* namespace cralgo */

#endif // !CRALGO_LAZY_VECTOR_H_

