/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.h             | Feature generator               |
| Author  : Francis Maes                   |                                 |
| Started : 13/02/2009 17:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_FEATURE_GENERATOR_H_
# define CRALGO_FEATURE_GENERATOR_H_

# include "ObjectPredeclarations.h"
# include "FeatureDictionary.h"
# include <vector>
# include <cmath>

namespace cralgo
{

class FeatureGenerator : public Object
{
public:
  virtual FeatureDictionaryPtr getDictionary() const = 0;
  
  /*
  ** General
  */
  // accept feature visitor
  virtual void accept(FeatureVisitorPtr visitor, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const = 0;

  // describe as string
  virtual std::string toString(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const = 0;
  
  // store in a sparse vector
  virtual SparseVectorPtr toSparseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const = 0;
  
  // store in a dense vector
  virtual DenseVectorPtr toDenseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const = 0;
  
  /*
  ** Const unary operations
  */
  virtual size_t l0norm() const = 0;
  virtual double l1norm() const = 0;  
  virtual double sumOfSquares() const = 0;
  
  inline double l2norm() const
    {return std::sqrt(sumOfSquares());}
  
  /*
  ** Assignment operations
  */
  // target <- target + featureGenerator
  virtual void addTo(DenseVectorPtr target, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const = 0;
  
  // target <- target - featureGenerator
  virtual void substractFrom(DenseVectorPtr target, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const = 0;

  // target <- target + weight * featureGenerator
  virtual void addWeightedTo(DenseVectorPtr target, double weight, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const = 0;
  virtual void addWeightedTo(SparseVectorPtr target, double weight, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const = 0;

  // target <- target + weight * sign(featureGenerator), if x < 0, sign(x) = -1, else if (x > 0) sign(x) = 1 else sign(x) = 0
  virtual void addWeightedSignsTo(DenseVectorPtr target, double weight, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const = 0;
  
  /*
  ** Dot-product operation
  */
  virtual double dotProduct(const DenseVectorPtr vector, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const = 0;
};

template<>
struct Traits<FeatureGeneratorPtr> : public ObjectPtrTraits<FeatureGenerator> {};

class FeatureVisitor : public Object
{
public:
  virtual bool featureEnter(FeatureDictionaryPtr dictionary, size_t index) = 0;
  virtual void featureSense(FeatureDictionaryPtr dictionary, size_t index, double value) = 0;

  virtual void featureCall(FeatureDictionaryPtr dictionary, size_t scopeIndex, FeatureGeneratorPtr featureGenerator)
  {
    if (featureEnter(dictionary, scopeIndex))
    {
      featureGenerator->accept(FeatureVisitorPtr(this), dictionary->getSubDictionary(scopeIndex));
      featureLeave();
    }
  }

  virtual void featureCall(FeatureDictionaryPtr dictionary, FeatureGeneratorPtr featureGenerator)
    {featureGenerator->accept(FeatureVisitorPtr(this), dictionary);}

  virtual void featureLeave() = 0;
};

template<class ExactType, class BaseType>
class FeatureGeneratorDefaultImplementations : public BaseType
{
public:
  FeatureGeneratorDefaultImplementations(FeatureDictionaryPtr dictionary) : BaseType(dictionary) {}
  FeatureGeneratorDefaultImplementations() {}
  
  // override this
  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor, FeatureDictionaryPtr dictionary) const
    {assert(false);}
    
  FeatureDictionaryPtr getDictionary() const
    {assert(false); return FeatureDictionaryPtr();}

public:
  virtual void accept(FeatureVisitorPtr visitor, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const;
  virtual SparseVectorPtr toSparseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const;
  virtual DenseVectorPtr toDenseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const;
  virtual std::string toString(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const;
  virtual size_t l0norm() const;
  virtual double l1norm() const;
  virtual double sumOfSquares() const;
  virtual void addTo(DenseVectorPtr target, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const;
  virtual void substractFrom(DenseVectorPtr target, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const;
  virtual void addWeightedTo(DenseVectorPtr target, double weight, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const;
  virtual void addWeightedTo(SparseVectorPtr target, double weight, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const;
  virtual void addWeightedSignsTo(DenseVectorPtr target, double weight, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const;
  virtual double dotProduct(const DenseVectorPtr vector, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const;

protected:
  const ExactType& _this() const {return *static_cast<const ExactType* >(this);}
  
  FeatureDictionaryPtr selectDictionary(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {return dictionary ? dictionary : _this().getDictionary();}
  
  template<class StaticVisitorType>
  void featureGenerator(StaticVisitorType& visitor, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {_this().staticFeatureGenerator(visitor, dictionary ? dictionary : _this().getDictionary());}
};

class SumFeatureGenerator : 
  public FeatureGeneratorDefaultImplementations<SumFeatureGenerator, FeatureGenerator>
{
public:
  SumFeatureGenerator(const std::vector<FeatureGeneratorPtr>& featureGenerators)
    : featureGenerators(featureGenerators) {}
  SumFeatureGenerator() {}
      
  virtual std::string getName() const
    {return "SumFeatureGenerator";}

  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor, FeatureDictionaryPtr dictionary) const
  {
    for (size_t i = 0; i < featureGenerators.size(); ++i)
      visitor.featureCall(dictionary, featureGenerators[i]);
  }

  void add(FeatureGeneratorPtr featureGenerator)
    {featureGenerators.push_back(featureGenerator);}
  
  virtual FeatureDictionaryPtr getDictionary() const
  {
    static FeatureDictionaryPtr defaultCompositeDictionary = new FeatureDictionary("SumFeatureGenerator");
    return defaultCompositeDictionary;
  }

protected:
  std::vector<FeatureGeneratorPtr> featureGenerators;
};

typedef ReferenceCountedObjectPtr<SumFeatureGenerator> SumFeatureGeneratorPtr;

class CompositeFeatureGenerator : 
  public FeatureGeneratorDefaultImplementations<CompositeFeatureGenerator, FeatureGenerator>
{
public:
  CompositeFeatureGenerator(const std::vector<FeatureGeneratorPtr>& featureGenerators)
    : featureGenerators(featureGenerators) {}
  CompositeFeatureGenerator() {}
      
  virtual std::string getName() const
    {return "CompositeFeatureGenerator";}

  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor, FeatureDictionaryPtr dictionary) const
  {
    for (size_t i = 0; i < featureGenerators.size(); ++i)
      visitor.featureCall(dictionary, i, featureGenerators[i]);
  }

  void add(FeatureGeneratorPtr featureGenerator)
    {featureGenerators.push_back(featureGenerator);}
  
  virtual FeatureDictionaryPtr getDictionary() const
  {
    static FeatureDictionaryPtr defaultCompositeDictionary = new FeatureDictionary("CompositeFeatureGenerator");
    return defaultCompositeDictionary;
  }
  
  size_t getNumFeatureGenerators() const
    {return featureGenerators.size();}
    
  FeatureGeneratorPtr getFeatureGenerator(size_t index) const
    {assert(index < featureGenerators.size()); return featureGenerators[index];}

protected:
  std::vector<FeatureGeneratorPtr> featureGenerators;
};

typedef ReferenceCountedObjectPtr<CompositeFeatureGenerator> CompositeFeatureGeneratorPtr;

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_GENERATOR_H_
