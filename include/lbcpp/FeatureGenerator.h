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
** @brief
*/
class FeatureGenerator : public Object
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual FeatureDictionaryPtr getDictionary() const = 0;

  /*!
  ** Check if the current feature generator and a feature dictionary
  ** are equals.
  **
  ** @param otherDictionary : feature dictionary.
  **
  ** @return True is they are equals.
  */
  bool checkDictionaryEquals(FeatureDictionaryPtr otherDictionary) const
    {return getDictionary()->checkEquals(otherDictionary);}

  /*!
  ** Check if the vector is dense.
  **
  ** @return False if it's not dense.
  */
  virtual bool isDense() const
    {return false;}

  /*!
  ** Convert to graph.
  **
  ** @return an object graph pointer.
  */
  virtual ObjectGraphPtr toGraph() const;

  /*!
  ** Convert to table.
  **
  ** @return a table pointer.
  */
  virtual TablePtr toTable() const;

  /*
  ** General
  */
  /*!
  ** Feature visitor entry point.
  **
  ** @param visitor : feature visitor pointer.
  */
  virtual void accept(FeatureVisitorPtr visitor) const = 0;

  /*!
  ** Convert to a sparse vector.
  **
  ** @return a sparse vector pointer.
  */
  virtual SparseVectorPtr toSparseVector() const = 0;

  /*!
  ** Convert to a dense vector.
  **
  ** @return a dense vector pointer.
  */
  virtual DenseVectorPtr toDenseVector() const = 0;

  /*
  ** Const unary operations
  */
  /*!
  ** Compute l0-norm.
  **
  ** @return the l0-norm.
  */
  virtual size_t l0norm() const = 0;

  /*!
  ** Compute l1-norm.
  **
  ** @return the l1-norm.
  */
  virtual double l1norm() const = 0;

  /*!
  ** Compute some of squares.
  **
  ** @return the some of squares.
  */
  virtual double sumOfSquares() const = 0;

  /*!
  ** Compute l2-norm.
  **
  ** @return the l2-norm.
  */
  inline double l2norm() const
    {return std::sqrt(sumOfSquares());}

  /*
  ** Assignment operations
  */
  /*!
  ** Add feature generator to @a target.
  **
  ** @param target : dense vector pointer.
  */
  virtual void addTo(DenseVectorPtr target) const
    {addWeightedTo(target, 1.0);}

  /*!
  ** Add feature generator to @a target.
  **
  ** @param target : sparse vector pointer.
  */
  virtual void addTo(SparseVectorPtr target) const
    {addWeightedTo(target, 1.0);}

  /*!
  ** Substract feature generator to @a target.
  **
  ** @param target : dense vector pointer.
  */
  virtual void substractFrom(DenseVectorPtr target) const
    {addWeightedTo(target, -1.0);}

  /*!
  ** Substract feature generator to @a target.
  **
  ** @param target : sparse vector pointer.
  */
  virtual void substractFrom(SparseVectorPtr target) const
    {addWeightedTo(target, -1.0);}

  /*!
  ** Add weighted feature generator to @a target.
  **
  ** @param target : dense vector pointer.
  ** @param weight : feature generator weight.
  */
  virtual void addWeightedTo(DenseVectorPtr target, double weight) const = 0;

  /*!
  ** Add weighted feature generator to @a target.
  **
  ** @param target : sparse vector pointer.
  ** @param weight : feature generator weight.
  */
  virtual void addWeightedTo(SparseVectorPtr target, double weight) const = 0;

  /*!
  ** Add weighted and signed feature generator to @a target.
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
  /*!
  ** Dot product ( featureGenerator . @a vector ).
  **
  ** @param vector : sparse vector pointer.
  **
  ** @return dot product result.
  */
  virtual double dotProduct(const SparseVectorPtr vector) const = 0;

  /*!
  ** Dot product ( featureGenerator . @a vector ).
  **
  ** @param vector : dense vector pointer.
  **
  ** @return dot product result.
  */
  virtual double dotProduct(const DenseVectorPtr vector) const = 0;

  /*!
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
  /*!
  ** Get number of subgenerators.
  **
  ** @return number of subgenerators.
  */
  virtual size_t getNumSubGenerators() const = 0;

  /*!
  ** #FIXME:
  **
  ** @param num
  **
  ** @return
  */
  virtual size_t getSubGeneratorIndex(size_t num) const = 0;

  /*!
  ** #FIXME
  **
  ** @param num : subgenerator index.
  **
  ** @return the feature generator instance corresponding.
  */
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const = 0;

  /*!
  ** #FIXME
  **
  ** @param index
  **
  ** @return
  */
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const = 0;
};

  /*!
  ** Create an empty feature generator.
  **
  ** @return an empty feature generator instance.
  */
extern FeatureGeneratorPtr emptyFeatureGenerator();

/*!
** Create an unit feature generator.
**
** @return an unit feature generator instance.
*/
extern FeatureGeneratorPtr unitFeatureGenerator();

/*!
** Multiply by a scalar (@a weight * @a featureGenerator).
**
** @param featureGenerator : feature generator instance.
** @param weight : scalar value.
**
** @return
*/
extern FeatureGeneratorPtr multiplyByScalar(FeatureGeneratorPtr featureGenerator, double weight);

/*!
** #FIXME
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

/*!
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

/*!
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

/*!
** #FIXME
**
** @param compositeFeatureGenerator
** @param weights
**
** @return
*/
extern FeatureGeneratorPtr linearCombination(FeatureGeneratorPtr compositeFeatureGenerator, DenseVectorPtr weights);

/*!
** #FIXME
**
** @param FeatureGeneratorPtr
** @param newTerms
**
** @return
*/
extern FeatureGeneratorPtr linearCombination(std::vector< std::pair<FeatureGeneratorPtr, double> >* newTerms);

/*!
** #FIXME
**
** @param dictionary
** @param index
** @param featureGenerator
**
** @return
*/
extern FeatureGeneratorPtr subFeatureGenerator(FeatureDictionaryPtr dictionary, size_t index, FeatureGeneratorPtr featureGenerator);


/*!
** @class featureVisitor
** @brief #FIXME all.
*/
class FeatureVisitor : public Object
{
public:
  /*!
  **
  **
  ** @param dictionary
  ** @param index
  **
  ** @return
  */
  virtual bool featureEnter(FeatureDictionaryPtr dictionary, size_t index) = 0;

  /*!
  **
  **
  ** @param dictionary
  ** @param index
  ** @param value
  */
  virtual void featureSense(FeatureDictionaryPtr dictionary, size_t index, double value) = 0;

  /*!
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

  /*!
  **
  **
  ** @param dictionary
  ** @param featureGenerator
  */
  virtual void featureCall(FeatureDictionaryPtr dictionary, FeatureGeneratorPtr featureGenerator)
    {featureGenerator->accept(FeatureVisitorPtr(this));}

  /*!
  **
  **
  */
  virtual void featureLeave() = 0;
};


/*!
** @class featureGeneratorDefaultImplementation
** @brief
*/
template<class ExactType, class BaseType>
class FeatureGeneratorDefaultImplementations : public BaseType
{
public:
  /*!
  **
  **
  ** @param dictionary
  **
  ** @return
  */
  FeatureGeneratorDefaultImplementations(FeatureDictionaryPtr dictionary) : BaseType(dictionary) {}

  /*!
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
  /*!
  **
  **
  ** @param visitor
  */
  virtual void accept(FeatureVisitorPtr visitor) const;
  /*!
  **
  **
  **
  ** @return
  */
  virtual SparseVectorPtr toSparseVector() const;
  /*!
  **
  **
  **
  ** @return
  */
  virtual DenseVectorPtr toDenseVector() const;
  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string toString() const;
  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t l0norm() const;
  /*!
  **
  **
  **
  ** @return
  */
  virtual double l1norm() const;
  /*!
  **
  **
  **
  ** @return
  */
  virtual double sumOfSquares() const;
  /*!
  **
  **
  ** @param target
  */
  virtual void addTo(DenseVectorPtr target) const;
  /*!
  **
  **
  ** @param target
  */
  virtual void addTo(SparseVectorPtr target) const;
  /*!
  **
  **
  ** @param target
  */
  virtual void substractFrom(DenseVectorPtr target) const;
  /*!
  **
  **
  ** @param target
  */
  virtual void substractFrom(SparseVectorPtr target) const;
  /*!
  **
  **
  ** @param target
  ** @param weight
  */
  virtual void addWeightedTo(DenseVectorPtr target, double weight) const;
  /*!
  **
  **
  ** @param target
  ** @param weight
  */
  virtual void addWeightedTo(SparseVectorPtr target, double weight) const;
  /*!
  **
  **
  ** @param target
  ** @param weight
  */
  virtual void addWeightedSignsTo(DenseVectorPtr target, double weight) const;
  /*!
  **
  **
  ** @param vector
  **
  ** @return
  */
  virtual double dotProduct(const SparseVectorPtr vector) const;
  /*!
  **
  **
  ** @param vector
  **
  ** @return
  */
  virtual double dotProduct(const DenseVectorPtr vector) const;
  /*!
  **
  **
  ** @param featureGenerator
  **
  ** @return
  */
  virtual double dotProduct(const FeatureGeneratorPtr featureGenerator) const;

protected:
  /*!
  **
  **
  **
  ** @return
  */
  const ExactType& _this() const {return *static_cast<const ExactType* >(this);}
};


/*!
** @class FlatfeatureGenerator
** @brief
*/

class FlatFeatureGenerator : public FeatureGenerator
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t getNumSubGenerators() const
    {return 0;}

  /*!
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {assert(false); return FeatureGeneratorPtr();}

  /*!
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual size_t getSubGeneratorIndex(size_t num) const
    {assert(false); return (size_t)-1;}

  /*!
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
