/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.h             | Feature generator               |
| Author  : Francis Maes                   |                                 |
| Started : 13/02/2009 17:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   FeatureGenerator.h
**@author Francis MAES
**@date   Mon Jun 15 23:56:06 2009
**
**@brief  Read only feature dictionary.
**
**
*/

#ifndef LBCPP_FEATURE_GENERATOR_H_
# define LBCPP_FEATURE_GENERATOR_H_

# include "ObjectPredeclarations.h"
# include "FeatureDictionary.h"
# include <vector>
# include <cmath>

namespace lbcpp
{

/*!
** @class FeatureGenerator
** @brief Base class for feature containers.
*/
class FeatureGenerator : public Object
{
public:
  /**
  ** Returns the FeatureDictionary.
  **
  ** @see FeatureDictionary
  ** @return a pointer on the FeatureDictionary.
  */
  virtual FeatureDictionaryPtr getDictionary() const = 0;

  /**
  ** Checks if the current feature generator and a feature dictionary
  ** are equals.
  **
  ** @param otherDictionary : feature dictionary.
  ** @see FeatureDictionary
  ** @return True is they are equals.
  */
  bool checkDictionaryEquals(FeatureDictionaryPtr otherDictionary) const
    {return getDictionary()->checkEquals(otherDictionary);}

  /**
  ** Checks if the vector is dense. majorite non nulle
  **
  ** @return False if it's not dense.
  */
  virtual bool isDense() const
    {return false;}

  /**
  ** Converts to an ObjectGraph.
  **
  ** @see ObjectGraph
  ** @return an object graph pointer.
  */
  virtual ObjectGraphPtr toGraph() const;

  /**
  ** Converts to a Table.
  **
  ** @see Table
  ** @return a table pointer.
  */
  virtual TablePtr toTable() const;

  /*
  ** General
  */
  /**
  ** Feature visitor entry point. + important tout se passze ici
  **
  ** @see FeatureVisitor
  ** @param visitor : feature visitor pointer.
  */
  virtual void accept(FeatureVisitorPtr visitor) const = 0;

  /**
  ** Converts to a SparseVector.
  **
  ** @see SparseVector
  ** @return a sparse vector pointer.
  */
  virtual SparseVectorPtr toSparseVector() const = 0;

  /**
  ** Converts to a DenseVector.
  **
  ** @see DenseVector
  ** @return a dense vector pointer.
  */
  virtual DenseVectorPtr toDenseVector() const = 0;

  /*
  ** Const unary operations
  */
  /**
  ** Computes l0-norm, defined by
  ** \f[ |x|_0=\underset{p\to\infty}{\lim}(\sum_{k=1}^n|x_k|^p) \f]
  ** where \f$ x \f$ is the current vector and \f$ n \f$ its dimension.
  **
  ** @return the l0-norm.
  */
  virtual size_t l0norm() const = 0;

  /**
  ** Computes l1-norm, defined by
  ** \f[ |x|_1=\sum_{k=1}^n|x_k| \f]
  ** where \f$ x \f$ is the current vector and \f$ n \f$ its dimension.
  **
  ** @return the l1-norm.
  */
  virtual double l1norm() const = 0;

  /**
  ** Computes the sum of squares, defined by
  ** \f[ \sum_{k=1}^n x_k^2 \f]
  **
  ** @return the sum of squares.
  */
  virtual double sumOfSquares() const = 0;

  /**
  ** Computes l2-norm, defined by
  ** \f[ |x|_2=\sqrt{\sum_{k=1}^n|x_k|^2} \f]
  ** where \f$ x \f$ is the current vector and \f$ n \f$ its dimension.
  **
  ** @return the l2-norm of the current vector.
  */
  inline double l2norm() const
    {return std::sqrt(sumOfSquares());}

  /*
  ** Assignment operations
  */
  /**
  ** Adds feature generator to @a target.
  **
  ** @param target : dense vector pointer.
  */
  virtual void addTo(DenseVectorPtr target) const
    {addWeightedTo(target, 1.0);}

  /**
  ** Adds feature generator to @a target.
  **
  ** @param target : sparse vector pointer.
  */
  virtual void addTo(SparseVectorPtr target) const
    {addWeightedTo(target, 1.0);}

  /**
  ** Substracts feature generator to @a target.
  **
  ** @param target : dense vector pointer.
  */
  virtual void substractFrom(DenseVectorPtr target) const
    {addWeightedTo(target, -1.0);}

  /**
  ** Substracts feature generator to @a target.
  **
  ** @param target : sparse vector pointer.
  */
  virtual void substractFrom(SparseVectorPtr target) const
    {addWeightedTo(target, -1.0);}

  /**
  ** Adds weighted feature generator to @a target.
  **
  ** @param target : dense vector pointer.
  ** @param weight : feature generator weight.
  */
  virtual void addWeightedTo(DenseVectorPtr target, double weight) const = 0;

  /**
  ** Adds weighted feature generator to @a target.
  **
  ** @param target : sparse vector pointer.
  ** @param weight : feature generator weight.
  */
  virtual void addWeightedTo(SparseVectorPtr target, double weight) const = 0;

  /**
  ** Adds weighted&signed feature generator to @a target.
  **
  ** i.e. target <- target + weight * sign(featureGenerator),
  ** if (x < 0), sign(x) = -1, else if (x > 0), sign(x) = 1, else
  ** sign(x) = 0
  **
  ** @param target : dense vector pointer.
  ** @param weight : feature generator weight.
  */
  virtual void addWeightedSignsTo(DenseVectorPtr target, double weight) const = 0;

  /*
  ** Dot-product operation
  */
  /**
  ** Dot product ( featureGenerator . @a vector ).
  **
  ** @param vector : sparse vector pointer.
  **
  ** @return dot product result.
  */
  virtual double dotProduct(const SparseVectorPtr vector) const = 0;

  /**
  ** Dot product ( featureGenerator . @a vector ).
  **
  ** @param vector : dense vector pointer.
  **
  ** @return dot product result.
  */
  virtual double dotProduct(const DenseVectorPtr vector) const = 0;

  /**
  ** Dot product ( featureGenerator . @a featureGenerator ).
  **
  ** @param featureGenerator : feature generator pointer.
  **
  ** @return dot product result.
  */
  virtual double dotProduct(const FeatureGeneratorPtr featureGenerator) const = 0;



  /*
  ** Sub-generators
  */
  /**
  ** Returns the number of sub-FeatureGenerators.
  **
  ** @return the number of sub-FeatureGenerators.
  */
  virtual size_t getNumSubGenerators() const = 0;

  /**
  ** Returns the index of the sub-FeatureGenerator number @a num.
  **
  ** @param num : sub-FeatureGenerator number.
  **
  ** @return the index of the sub-FeatureGenerator number @a num.
  */
  virtual size_t getSubGeneratorIndex(size_t num) const = 0;

  /**
  ** Returns the sub-FeatureGenerator number @a num.
  **
  ** @param num : sub-FeatureGenerator number.
  **
  ** @return the sub-FeatureGenerator number @a num.
  */
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const = 0;

  /**
  ** Returns the sub-FeatureGenerator at the index @a index.
  **
  ** @param index : index of the sub-FeatureGenerator.
  **
  ** @return the sub-FeatureGenerator at the index @a num.
  */
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const = 0;
};

  /**
  ** Creates an empty feature generator.
  **
  ** @return an empty FeatureGenerator instance.
  */
extern FeatureGeneratorPtr emptyFeatureGenerator();

/**
** Creates an unit feature generator.
**
** @return an unit FeatureGenerator instance.
*/
extern FeatureGeneratorPtr unitFeatureGenerator();

/**
** Multiply by a scalar (@a weight * @a featureGenerator).
**
** @param featureGenerator : feature generator instance.
** @param weight : scalar value.
**
** @return
*/
extern FeatureGeneratorPtr multiplyByScalar(FeatureGeneratorPtr featureGenerator, double weight);

/**
** Weighted sum. Defined by: \f$ weight1.\text{featureGenerator1}+weight2.\text{FeatureGenerator}\f$
**
** @param featureGenerator1
** @param weight1
** @param featureGenerator2
** @param weight2
** @param computeNow
**
** @return
*/
extern FeatureGeneratorPtr weightedSum(FeatureGeneratorPtr featureGenerator1, double weight1, FeatureGeneratorPtr featureGenerator2, double weight2, bool computeNow = false);

/**
** #FIXME
**
** @param featureGenerator1
** @param featureGenerator2
** @param computeNow
**
** @return
*/
inline FeatureGeneratorPtr addition(FeatureGeneratorPtr featureGenerator1, FeatureGeneratorPtr featureGenerator2, bool computeNow = false)
  {return weightedSum(featureGenerator1, 1.0, featureGenerator2, 1.0, computeNow);}

/**
** #FIXME
**
** @param featureGenerator1
** @param featureGenerator2
** @param computeNow
**
** @return
*/
inline FeatureGeneratorPtr difference(FeatureGeneratorPtr featureGenerator1, FeatureGeneratorPtr featureGenerator2, bool computeNow = false)
  {return weightedSum(featureGenerator1, 1.0, featureGenerator2, -1.0, computeNow);}

/**
** #FIXME
**
** @param compositeFeatureGenerator
** @param weights
**
** @return
*/
extern FeatureGeneratorPtr linearCombination(FeatureGeneratorPtr compositeFeatureGenerator, DenseVectorPtr weights);

/**
** #FIXME
**
** @param FeatureGeneratorPtr
** @param newTerms
**
** @return
*/
extern FeatureGeneratorPtr linearCombination(std::vector< std::pair<FeatureGeneratorPtr, double> >* newTerms);

/**
** #FIXM
**
** @param dictionary
** @param index
** @param featureGenerator
**
** @return
*/
extern FeatureGeneratorPtr subFeatureGenerator(FeatureDictionaryPtr dictionary, size_t index, FeatureGeneratorPtr featureGenerator);


/**
** @class featureVisitor
** @brief #FIXME all.
*/
class FeatureVisitor : public Object
{
public:
  /**
  **
  **
  ** @param dictionary
  ** @param index
  **
  ** @return
  */
  virtual bool featureEnter(FeatureDictionaryPtr dictionary, size_t index) = 0;

  /**
  **
  **
  ** @param dictionary
  ** @param index
  ** @param value
  */
  virtual void featureSense(FeatureDictionaryPtr dictionary, size_t index, double value) = 0;

  /**
  **
  **
  ** @param dictionary
  ** @param scopeIndex
  ** @param featureGenerator
  */
  virtual void featureCall(FeatureDictionaryPtr dictionary, size_t scopeIndex, FeatureGeneratorPtr featureGenerator)
  {
    if (featureEnter(dictionary, scopeIndex))
    {
      featureGenerator->accept(FeatureVisitorPtr(this));
      featureLeave();
    }
  }

  /**
  **
  **
  ** @param dictionary
  ** @param featureGenerator
  */
  virtual void featureCall(FeatureDictionaryPtr dictionary, FeatureGeneratorPtr featureGenerator)
    {featureGenerator->accept(FeatureVisitorPtr(this));}

  /**
  **
  **
  */
  virtual void featureLeave() = 0;
};


/**
** @class featureGeneratorDefaultImplementation
** @brief
*/
template<class ExactType, class BaseType>
class FeatureGeneratorDefaultImplementations : public BaseType
{
public:
  /**
  **
  **
  ** @param dictionary
  **
  ** @return
  */
  FeatureGeneratorDefaultImplementations(FeatureDictionaryPtr dictionary) : BaseType(dictionary) {}

  /**
  **
  **
  **
  ** @return
  */
  FeatureGeneratorDefaultImplementations() {}

  // override this:
  //
  // template<class VisitorType>
  // void staticFeatureGenerator(VisitorType& visitor) const;

  // and also virtual FeatureGeneratorPtr getDictionary();

public:
  /**
  **
  **
  ** @param visitor
  */
  virtual void accept(FeatureVisitorPtr visitor) const;
  /**
  **
  **
  **
  ** @return
  */
  virtual SparseVectorPtr toSparseVector() const;
  /**
  **
  **
  **
  ** @return
  */
  virtual DenseVectorPtr toDenseVector() const;
  /**
  **
  **
  **
  ** @return
  */
  virtual std::string toString() const;
  /**
  **
  **
  **
  ** @return
  */
  virtual size_t l0norm() const;
  /**
  **
  **
  **
  ** @return
  */
  virtual double l1norm() const;
  /**
  **
  **
  **
  ** @return
  */
  virtual double sumOfSquares() const;
  /**
  **
  **
  ** @param target
  */
  virtual void addTo(DenseVectorPtr target) const;
  /**
  **
  **
  ** @param target
  */
  virtual void addTo(SparseVectorPtr target) const;
  /**
  **
  **
  ** @param target
  */
  virtual void substractFrom(DenseVectorPtr target) const;
  /**
  **
  **
  ** @param target
  */
  virtual void substractFrom(SparseVectorPtr target) const;
  /**
  **
  **
  ** @param target
  ** @param weight
  */
  virtual void addWeightedTo(DenseVectorPtr target, double weight) const;
  /**
  **
  **
  ** @param target
  ** @param weight
  */
  virtual void addWeightedTo(SparseVectorPtr target, double weight) const;
  /**
  **
  **
  ** @param target
  ** @param weight
  */
  virtual void addWeightedSignsTo(DenseVectorPtr target, double weight) const;
  /**
  **
  **
  ** @param vector
  **
  ** @return
  */
  virtual double dotProduct(const SparseVectorPtr vector) const;
  /**
  **
  **
  ** @param vector
  **
  ** @return
  */
  virtual double dotProduct(const DenseVectorPtr vector) const;
  /**
  **
  **
  ** @param featureGenerator
  **
  ** @return
  */
  virtual double dotProduct(const FeatureGeneratorPtr featureGenerator) const;

protected:
  /**
  **
  **
  **
  ** @return
  */
  const ExactType& _this() const {return *static_cast<const ExactType* >(this);}
};


/**
** @class FlatfeatureGenerator
** @brief
*/

class FlatFeatureGenerator : public FeatureGenerator
{
public:
  /**
  **
  **
  **
  ** @return
  */
  virtual size_t getNumSubGenerators() const
    {return 0;}

  /**
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {assert(false); return FeatureGeneratorPtr();}

  /**
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual size_t getSubGeneratorIndex(size_t num) const
    {assert(false); return (size_t)-1;}

  /**
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
    {assert(false); return FeatureGeneratorPtr();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_H_
