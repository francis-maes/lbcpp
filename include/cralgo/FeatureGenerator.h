/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.h             | Feature generator               |
| Author  : Francis Maes                   |                                 |
| Started : 13/02/2009 17:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_FEATURE_GENERATOR_H_
# define CRALGO_FEATURE_GENERATOR_H_

# include "Object.h"
# include "FeatureDictionary.h"
# include <vector>
# include <cmath>

namespace cralgo
{

class DoubleVector;
typedef ReferenceCountedObjectPtr<DoubleVector> DoubleVectorPtr;
class SparseVector;
typedef ReferenceCountedObjectPtr<SparseVector> SparseVectorPtr;
class DenseVector;
typedef ReferenceCountedObjectPtr<DenseVector> DenseVectorPtr;

class FeatureVisitor;
class FeatureGenerator : public Object
{
public:
  virtual FeatureDictionary& getDefaultDictionary() const = 0;
  
  /*
  ** General
  */
  // accept feature visitor
  virtual void accept(FeatureVisitor& visitor, FeatureDictionary* dictionary = NULL) const = 0;

  // describe as string
  virtual std::string toString(FeatureDictionary* dictionary = NULL) const = 0;
  
  // store in a sparse vector
  virtual SparseVectorPtr createSparseVector(FeatureDictionary* dictionary = NULL) const = 0;
  
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
  virtual void addTo(DenseVectorPtr target, FeatureDictionary* dictionary = NULL) const = 0;
  
  // target <- target - featureGenerator
  virtual void substractFrom(DenseVectorPtr target, FeatureDictionary* dictionary = NULL) const = 0;

  // target <- target + weight * featureGenerator
  virtual void addWeightedTo(DenseVectorPtr target, double weight, FeatureDictionary* dictionary = NULL) const = 0;

  // target <- target + weight * sign(featureGenerator), if x < 0, sign(x) = -1, else if (x > 0) sign(x) = 1 else sign(x) = 0
  virtual void addWeightedSignsTo(DenseVectorPtr target, double weight, FeatureDictionary* dictionary = NULL) const = 0;
  
  /*
  ** Dot-product operation
  */
  virtual double dotProduct(const DenseVectorPtr vector, FeatureDictionary* dictionary = NULL) const = 0;
};

typedef ReferenceCountedObjectPtr<FeatureGenerator> FeatureGeneratorPtr;

class FeatureVisitor
{
public:
  virtual ~FeatureVisitor() {}
  
  virtual bool featureEnter(FeatureDictionary& dictionary, size_t index) = 0;
  virtual void featureSense(FeatureDictionary& dictionary, size_t index, double value) = 0;

  virtual void featureCall(FeatureDictionary& dictionary, size_t scopeIndex, FeatureGeneratorPtr featureGenerator)
  {
    if (featureEnter(dictionary, scopeIndex))
    {
      featureGenerator->accept(*this, &dictionary.getSubDictionary(scopeIndex));
      featureLeave();
    }
  }

  virtual void featureCall(FeatureDictionary& dictionary, FeatureGeneratorPtr featureGenerator)
    {featureGenerator->accept(*this, &dictionary);}

  virtual void featureLeave() = 0;
};

template<class ExactType, class BaseType>
class FeatureGeneratorDefaultImplementations : public BaseType
{
public:
  // override this
  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor, FeatureDictionary& dictionary) const
    {assert(false);}
    
  FeatureDictionary& getDefaultDictionary() const
    {assert(false); return *(FeatureDictionary* )NULL;}

public:
  virtual void accept(FeatureVisitor& visitor, FeatureDictionary* dictionary = NULL) const;
  virtual SparseVectorPtr createSparseVector(FeatureDictionary* dictionary = NULL) const;
  virtual std::string toString(FeatureDictionary* dictionary = NULL) const;
  virtual size_t l0norm() const;
  virtual double l1norm() const;
  virtual double sumOfSquares() const;
  virtual void addTo(DenseVectorPtr target, FeatureDictionary* dictionary = NULL) const;
  virtual void substractFrom(DenseVectorPtr target, FeatureDictionary* dictionary = NULL) const;
  virtual void addWeightedTo(DenseVectorPtr target, double weight, FeatureDictionary* dictionary = NULL) const;
  virtual void addWeightedSignsTo(DenseVectorPtr target, double weight, FeatureDictionary* dictionary = NULL) const;
  virtual double dotProduct(const DenseVectorPtr vector, FeatureDictionary* dictionary = NULL) const;

protected:
  const ExactType& _this() const {return *static_cast<const ExactType* >(this);}
  
  FeatureDictionary& selectDictionary(FeatureDictionary* dictionary = NULL) const
    {return dictionary ? *dictionary : _this().getDefaultDictionary();}
  
  template<class VisitorType>
  void featureGenerator(VisitorType& visitor, FeatureDictionary* dictionary = NULL) const
    {_this().staticFeatureGenerator(visitor, dictionary ? *dictionary : _this().getDefaultDictionary());}
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
  void staticFeatureGenerator(VisitorType& visitor, FeatureDictionary& dictionary) const
  {
    for (size_t i = 0; i < featureGenerators.size(); ++i)
      visitor.featureCall(dictionary, featureGenerators[i]);
  }

  void add(FeatureGeneratorPtr featureGenerator)
    {featureGenerators.push_back(featureGenerator);}
  
  virtual FeatureDictionary& getDefaultDictionary() const
  {
    static FeatureDictionary defaultCompositeDictionary("SumFeatureGenerator");
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
  void staticFeatureGenerator(VisitorType& visitor, FeatureDictionary& dictionary) const
  {
    for (size_t i = 0; i < featureGenerators.size(); ++i)
      visitor.featureCall(dictionary, i, featureGenerators[i]);
  }

  void add(FeatureGeneratorPtr featureGenerator)
    {featureGenerators.push_back(featureGenerator);}
  
  virtual FeatureDictionary& getDefaultDictionary() const
  {
    static FeatureDictionary defaultCompositeDictionary("CompositeFeatureGenerator");
    return defaultCompositeDictionary;
  }

protected:
  std::vector<FeatureGeneratorPtr> featureGenerators;
};

typedef ReferenceCountedObjectPtr<CompositeFeatureGenerator> CompositeFeatureGeneratorPtr;

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_GENERATOR_H_
