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
  LazyVector(FeatureDictionary& dictionary);
  LazyVector() {}
  virtual void clear();  
  
  /*
  ** State
  */
  bool isStoredWithFeatureGenerator() const;
  bool isStoredWithDenseVector() const;
  bool isStoredWithSparseVector() const;
  
  void storeWithDenseVector();  
  void storeWithSparseVector();
  
  virtual DenseVectorPtr toDenseVector(FeatureDictionary* dictionary = NULL) const
    {const_cast<LazyVector* >(this)->storeWithDenseVector(); return denseVector;}
  virtual SparseVectorPtr toSparseVector(FeatureDictionary* dictionary = NULL) const
    {const_cast<LazyVector* >(this)->storeWithSparseVector(); return sparseVector;}

  bool guessIfDense() const;

  /*
  ** Set
  */
  void set(FeatureGeneratorPtr featureGenerator)
    {clear(); this->featureGenerator = featureGenerator;}
    
  void set(SparseVectorPtr sparseVector)
    {clear(); this->sparseVector = sparseVector;}

  void set(DenseVectorPtr denseVector)
    {clear(); this->denseVector = denseVector;}  

  /*
  ** Sub lazy-vectors
  */
  LazyVectorPtr& getSubVector(size_t i, bool createIfMissing = false);

  /*
  ** Operations
  */
  void add(const FeatureGeneratorPtr features)
    {addInCombination(features, 1);}
  
  void addWeighted(const FeatureGeneratorPtr features, double weight)
    {addInCombination(features, weight);}
    
  void addWeighted(const LazyVectorPtr vector, double weight);
  void multiplyByScalar(double scalar);
    
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

  virtual void addWeightedTo(DenseVectorPtr target, double weight) const;
  
private:
  typedef std::map<const FeatureGeneratorPtr, double> LinearCombinationMap;

  /*
  ** Internal representation:
  **   LazyVector = featureGenerator + sparseVector + denseVector + linear-combination-of-feature-generators + sub-vectors
  */
  FeatureGeneratorPtr featureGenerator;
  SparseVectorPtr sparseVector;
  DenseVectorPtr denseVector;
  LinearCombinationMap linearCombination;
  std::vector<LazyVectorPtr> subVectors;

  void addInCombination(const FeatureGeneratorPtr featureGenerator, double weight = 1.0)
    {linearCombination[featureGenerator] += weight;}


  template<class T>
  void ensureInCombination(ReferenceCountedObjectPtr<T>& featureGenerator)
  {
    if (featureGenerator)
      addInCombination(featureGenerator);
    featureGenerator.clear();
  }
};

}; /* namespace cralgo */

#endif // !CRALGO_LAZY_VECTOR_H_
