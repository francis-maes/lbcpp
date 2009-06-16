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
**@brief  #FIXME: all
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
  **
  **
  ** @param otherDictionary
  **
  ** @return
  */
  bool checkDictionaryEquals(FeatureDictionaryPtr otherDictionary) const
    {return getDictionary()->checkEquals(otherDictionary);}

  /*!
  **
  **
  **
  ** @return
  */
  virtual bool isDense() const
    {return false;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual ObjectGraphPtr toGraph() const;

  /*!
  **
  **
  **
  ** @return
  */
  virtual TablePtr toTable() const;

  /*
  ** General
  */
  // accept feature visitor
  /*!
  **
  **
  ** @param visitor
  */
  virtual void accept(FeatureVisitorPtr visitor) const = 0;

  // store in a sparse vector
  /*!
  **
  **
  **
  ** @return
  */
  virtual SparseVectorPtr toSparseVector() const = 0;

  // store in a dense vector
  /*!
  **
  **
  **
  ** @return
  */
  virtual DenseVectorPtr toDenseVector() const = 0;

  /*
  ** Const unary operations
  */
  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t l0norm() const = 0;
  /*!
  **
  **
  **
  ** @return
  */
  virtual double l1norm() const = 0;
  /*!
  **
  **
  **
  ** @return
  */
  virtual double sumOfSquares() const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  inline double l2norm() const
    {return std::sqrt(sumOfSquares());}

  /*
  ** Assignment operations
  */
  // target <- target + featureGenerator
  /*!
  **
  **
  ** @param target
  */
  virtual void addTo(DenseVectorPtr target) const
    {addWeightedTo(target, 1.0);}

  /*!
  **
  **
  ** @param target
  */
  virtual void addTo(SparseVectorPtr target) const
    {addWeightedTo(target, 1.0);}

  // target <- target - featureGenerator
  /*!
  **
  **
  ** @param target
  */
  virtual void substractFrom(DenseVectorPtr target) const
    {addWeightedTo(target, -1.0);}
  /*!
  **
  **
  ** @param target
  */
  virtual void substractFrom(SparseVectorPtr target) const
    {addWeightedTo(target, -1.0);}

  // target <- target + weight * featureGenerator
  /*!
  **
  **
  ** @param target
  ** @param weight
  */
  virtual void addWeightedTo(DenseVectorPtr target, double weight) const = 0;
  /*!
  **
  **
  ** @param target
  ** @param weight
  */
  virtual void addWeightedTo(SparseVectorPtr target, double weight) const = 0;

  // target <- target + weight * sign(featureGenerator), if x < 0, sign(x) = -1, else if (x > 0) sign(x) = 1 else sign(x) = 0
  /*!
  **
  **
  ** @param target
  ** @param weight
  */
  virtual void addWeightedSignsTo(DenseVectorPtr target, double weight) const = 0;

  /*
  ** Dot-product operation
  */
  /*!
  **
  **
  ** @param vector
  **
  ** @return
  */
  virtual double dotProduct(const SparseVectorPtr vector) const = 0;
  /*!
  **
  **
  ** @param vector
  **
  ** @return
  */
  virtual double dotProduct(const DenseVectorPtr vector) const = 0;
  /*!
  **
  **
  ** @param featureGenerator
  **
  ** @return
  */
  virtual double dotProduct(const FeatureGeneratorPtr featureGenerator) const = 0;

  /*
  ** Sub-generators
  */
  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t getNumSubGenerators() const = 0;
  /*!
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual size_t getSubGeneratorIndex(size_t num) const = 0;
  /*!
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const = 0;
  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const = 0;
};

  /*!
  **
  **
  **
  ** @return
  */
extern FeatureGeneratorPtr emptyFeatureGenerator();

/*!
**
**
**
** @return
*/
extern FeatureGeneratorPtr unitFeatureGenerator();

/*!
**
**
** @param featureGenerator
** @param weight
**
** @return
*/
extern FeatureGeneratorPtr multiplyByScalar(FeatureGeneratorPtr featureGenerator, double weight);

/*!
**
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
**
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
**
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
**
**
** @param compositeFeatureGenerator
** @param weights
**
** @return
*/
extern FeatureGeneratorPtr linearCombination(FeatureGeneratorPtr compositeFeatureGenerator, DenseVectorPtr weights);

/*!
**
**
** @param FeatureGeneratorPtr
** @param newTerms
**
** @return
*/
extern FeatureGeneratorPtr linearCombination(std::vector< std::pair<FeatureGeneratorPtr, double> >* newTerms);

/*!
**
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
** @brief
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
