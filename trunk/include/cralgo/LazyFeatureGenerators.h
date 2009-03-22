/*-----------------------------------------.---------------------------------.
| Filename: LazyFeatureGenerators.h        | Lazy Feature Generators         |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 18:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_LAZY_FEATURE_GENERATORS_H_
# define CRALGO_LAZY_FEATURE_GENERATORS_H_

# include "FeatureGenerator.h"
# include "DoubleVector.h"

namespace cralgo
{

extern FeatureGeneratorPtr multiplyByScalar(FeatureGeneratorPtr featureGenerator, double weight);

/*
** FeatureGenerator * Scalar
*/
class WeightedFeatureGenerator
  : public FeatureGeneratorDefaultImplementations<WeightedFeatureGenerator, DoubleVector>
{
public:
  WeightedFeatureGenerator(FeatureGeneratorPtr featureGenerator, double weight)
    : featureGenerator(featureGenerator), weight(weight) {}
    
  virtual void clear()
    {featureGenerator = FeatureGeneratorPtr(); weight = 0.0;}

  bool exists() const
    {return featureGenerator && weight;}
    
  virtual size_t getNumSubGenerators() const
    {return exists() ? featureGenerator->getNumSubGenerators() : 0;}
    
  virtual FeatureGeneratorPtr getSubGenerator(size_t index) const
    {return multiplyByScalar(featureGenerator->getSubGenerator(index), weight);}

  FeatureGeneratorPtr getFeatureGenerator() const
    {return featureGenerator;}
  
  double getWeight() const
    {return weight;}

private:
  FeatureGeneratorPtr featureGenerator;
  double weight;
};

/*
** Sum_i FeatureGenerator * Weight_i
*/
class LinearCombinationFeatureGenerator
  : public FeatureGeneratorDefaultImplementations<LinearCombinationFeatureGenerator, DoubleVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<LinearCombinationFeatureGenerator, DoubleVector> BaseClass;
  
  LinearCombinationFeatureGenerator(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr())
    : BaseClass(dictionary) {}

  typedef std::map<const FeatureGeneratorPtr, double> LinearCombinationMap;
  typedef LinearCombinationMap::const_iterator const_iterator;
  
  const_iterator begin() const
    {return m.begin();}
    
  const_iterator end() const
    {return m.end();}
    
  size_t getNumElements() const
    {return m.size();}

  void addWeighted(const FeatureGeneratorPtr featureGenerator, double weight)
  {
    if (!featureGenerator || !weight)
      return;
      
    ensureDictionary(featureGenerator->getDictionary());
    m[featureGenerator] += weight;
  }
  
  void add(const FeatureGeneratorPtr featureGenerator)
    {addWeighted(featureGenerator, 1.0);}

  virtual void clear()
    {m.clear();}

  virtual size_t getNumSubGenerators() const
  {
    assert(false);
    // FIXME
    return 0;
  }
    
  virtual FeatureGeneratorPtr getSubGenerator(size_t index) const
  {
    assert(false);
    // FIXME
    return FeatureGeneratorPtr();
  }

private:
  LinearCombinationMap m;
};

/*
** (index, subFeatureGenerator)
*/
class SubFeatureGenerator :
  public FeatureGeneratorDefaultImplementations<SubFeatureGenerator, FeatureGenerator>
{
public:
  SubFeatureGenerator(size_t index, FeatureGeneratorPtr featureGenerator)
    : index(index), featureGenerator(featureGenerator) {}
  
  bool exists() const
    {return index != (size_t)-1;}
  
  virtual void clear()
    {index = (size_t)-1; featureGenerator = FeatureGeneratorPtr();}
  
  virtual size_t getNumSubGenerators() const
    {return index + 1;}
    
  virtual FeatureGeneratorPtr getSubGenerator(size_t index) const
    {return index == this->index ? featureGenerator : FeatureGeneratorPtr();}

  size_t getIndex() const
    {return index;}
    
  FeatureGeneratorPtr getFeatureGenerator() const
    {return featureGenerator;}

private:
  size_t index;
  FeatureGeneratorPtr featureGenerator;
};

/*
** (FG1, ..., FGn)
*/
class CompositeFeatureGenerator : 
  public FeatureGeneratorDefaultImplementations<CompositeFeatureGenerator, DoubleVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<CompositeFeatureGenerator, DoubleVector> BaseClass;

  CompositeFeatureGenerator(const std::vector<FeatureGeneratorPtr>& featureGenerators)
    : featureGenerators(featureGenerators) {}
  CompositeFeatureGenerator(size_t numSubGenerators, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr())
    : BaseClass(dictionary), featureGenerators(numSubGenerators, FeatureGeneratorPtr()) {}
  CompositeFeatureGenerator() {}
      
  virtual std::string getName() const
    {return "CompositeFeatureGenerator";}

  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor, FeatureDictionaryPtr dictionary) const
  {
    for (size_t i = 0; i < featureGenerators.size(); ++i)
      visitor.featureCall(dictionary, i, featureGenerators[i]);
  }

  void setSubGenerator(size_t index, FeatureGeneratorPtr featureGenerator)
  {
    if (featureGenerators.size() < index + 1)
      featureGenerators.resize(index + 1, FeatureGeneratorPtr());
    featureGenerators[index] = featureGenerator;
  }
  
  void appendSubGenerator(FeatureGeneratorPtr featureGenerator)
    {featureGenerators.push_back(featureGenerator);}
  
  virtual FeatureDictionaryPtr getDictionary() const
  {
    static FeatureDictionaryPtr defaultDictionary
      = new FeatureDictionary("CompositeFeatureGenerator");
    return defaultDictionary;
  }
  
  virtual size_t getNumSubGenerators() const
    {return featureGenerators.size();}
    
  virtual FeatureGeneratorPtr getSubGenerator(size_t index) const
    {assert(index < featureGenerators.size()); return featureGenerators[index];}

  virtual void clear()
    {featureGenerators.clear();}

protected:
  std::vector<FeatureGeneratorPtr> featureGenerators;
};

#if 0
class LazyVector : public FeatureGeneratorDefaultImplementations<LazyVector, DoubleVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<LazyVector, DoubleVector> BaseClass;

  LazyVector(FeatureDictionaryPtr dictionary);
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
  
  virtual DenseVectorPtr toDenseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {const_cast<LazyVector* >(this)->storeWithDenseVector(); return denseVector;}
  virtual SparseVectorPtr toSparseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
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

  void setSubVector(size_t i, LazyVectorPtr vector)
    {getSubVector(i) = vector;}

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
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionaryPtr featureDictionary) const;

  /*
  ** FeatureGenerator
  */
  virtual std::string getName() const
    {return "LazyVector";}
    
  virtual FeatureDictionaryPtr getDictionary() const;
  
  virtual void addWeightedTo(DenseVectorPtr target, double weight) const;
  virtual size_t l0norm() const;
  virtual double l1norm() const;
  virtual double sumOfSquares() const;
  
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
#endif

}; /* namespace cralgo */

#endif // !CRALGO_LAZY_FEATURE_GENERATORS_H_
