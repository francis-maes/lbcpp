/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.h             | Feature generator               |
| Author  : Francis Maes                   |                                 |
| Started : 13/02/2009 17:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

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
  ** @return a pointer on the FeatureDictionary.
  ** @see FeatureDictionary
  */
  virtual FeatureDictionaryPtr getDictionary() const = 0;

  /**
  ** Checks if the current feature generator and a feature dictionary
  ** are equals.
  **
  ** @param otherDictionary : feature dictionary.
  ** @return True is they are equals.
  ** @see FeatureDictionary
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
  ** @return an object graph pointer.
  ** @see ObjectGraph
  */
  virtual ObjectGraphPtr toGraph() const;

  /**
  ** Converts to a Table.
  **
  ** @return a table pointer.
  ** @see Table
  */
  virtual TablePtr toTable() const;

  /*
  ** General
  */
  /**
  ** Feature visitor entry point.
  **
  ** @param visitor : feature visitor pointer.
  ** @see FeatureVisitor
  */
  virtual void accept(FeatureVisitorPtr visitor) const = 0;

  /**
  ** Converts to a SparseVector.
  **
  ** @return a sparse vector pointer.
  ** @see SparseVector
  */
  virtual SparseVectorPtr toSparseVector() const = 0;

  /**
  ** Converts to a DenseVector.
  **
  ** @return a dense vector pointer.
  ** @see DenseVector
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
  ** target <- target + weight * this
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
** Computes the weighted sum defined by:
** \f[ weight1.\text{featureGenerator1}+weight2.\text{FeatureGenerator}\f]
**
** @param featureGenerator1 : first FeatureGenerator.
** @param weight1 : first weight.
** @param featureGenerator2 : second FeatureGenerator.
** @param weight2 : second weight.
** @param computeNow : compute now (True) or only if necessary (False).
**
** @return a new FeatureGenerator instance.
*/
extern FeatureGeneratorPtr weightedSum(FeatureGeneratorPtr featureGenerator1, double weight1, FeatureGeneratorPtr featureGenerator2, double weight2, bool computeNow = false);

/**
** Computes the addition of @a featureGenerator1 and @a featureGenerator2.
**
** @param featureGenerator1 : first FeatureGenerator.
** @param featureGenerator2 : second FeatureGenerator.
** @param computeNow : compute now (True) or only if necessary (False).
**
** @return a new FeatureGenerator instance.
*/
inline FeatureGeneratorPtr addition(FeatureGeneratorPtr featureGenerator1, FeatureGeneratorPtr featureGenerator2, bool computeNow = false)
  {return weightedSum(featureGenerator1, 1.0, featureGenerator2, 1.0, computeNow);}

/**
** Computes the difference between @a featureGenerator1 and @a featureGenerator2.
**
** @param featureGenerator1 : first FeatureGenerator.
** @param featureGenerator2 : second FeatureGenerator.
** @param computeNow : compute now (True) or only if necessary (False).
**
** @return a new FeatureGenerator instance.
*/
inline FeatureGeneratorPtr difference(FeatureGeneratorPtr featureGenerator1, FeatureGeneratorPtr featureGenerator2, bool computeNow = false)
  {return weightedSum(featureGenerator1, 1.0, featureGenerator2, -1.0, computeNow);}

/**
** Computes a linear combination between FeatureGenerators contained
** into the @a compositeFeatureGenerator (weighted with the @a weights
** vector).
*
**      Example:
*
**      compositeFeatureGenerator is composed by three
**      FeatureGenerators named FG1,FG2 and FG3. So we have:
**      - compositeFG = { FG1, FG2, FG3 }
**      - weights = [ 3.0, 2.0, 1.5 ]
**
**      linearCombination(compositeFG, weights) returns a
**      new FeatureGenerator resulting on the linear combination of
**      3.0FG1, 2.0FG2 and 1.5FG3 (3.0FG1 + 2.0FG2 + 1.5FG3).
**
** @param compositeFeatureGenerator : composite FeatureGenerator.
** @param weights : weight vector.
**
** @return a new FeatureGenerator instance resulting on the linear
** combination.
*/
extern FeatureGeneratorPtr linearCombination(FeatureGeneratorPtr compositeFeatureGenerator, DenseVectorPtr weights);

/**
** Computes a linear combination of the pairs < FeatureGenerator,
** scalar value >.
**
** @param newTerms : pairs <FeatureGenerator,scalar value>.
**
** @return a new FeatureGenerator instance resulting on the linear
** combination.
*/
extern FeatureGeneratorPtr linearCombination(std::vector< std::pair<FeatureGeneratorPtr, double> >* newTerms);

/**
** Returns a sub FeatureGenerator
**
** @param dictionary : feature dictionary.
** @param index : feature index.
** @param featureGenerator : feature generator.
**
** @return a new FeatureGenerator instance.
*/
extern FeatureGeneratorPtr subFeatureGenerator(FeatureDictionaryPtr dictionary, size_t index, FeatureGeneratorPtr featureGenerator);


/**
** @class featureGeneratorDefaultImplementation
** @brief Feature generator base class implementation.
** @see FeatureGenerator
*/
template<class ExactType, class BaseType>
class FeatureGeneratorDefaultImplementations : public BaseType
{
public:
  /**
  ** Constructor
  **
  ** @param dictionary : feature dictionary.
  ** @return a new FeatureGeneratorDefaultImplementations instance.
  */
  FeatureGeneratorDefaultImplementations(FeatureDictionaryPtr dictionary) : BaseType(dictionary) {}

  /**
  ** Constructor.
  **
  ** @return a new FeatureGeneratorDefaultImplementations instance.
  */
  FeatureGeneratorDefaultImplementations() {}

  // override this:
  //
  // template<class VisitorType>
  // void staticFeatureGenerator(VisitorType& visitor) const;

  // and also virtual FeatureGeneratorPtr getDictionary();

public:
  /**
  ** @see FeatureGenerator::accept
  */
  virtual void accept(FeatureVisitorPtr visitor) const;

  /**
  ** @see FeatureGenerator::toSparseVector
  */
  virtual SparseVectorPtr toSparseVector() const;

  /**
  ** @see FeatureGenerator::toDenseVector
  */
  virtual DenseVectorPtr toDenseVector() const;

  /**
  ** @see FeatureGenerator::toString
  */
  virtual String toString() const;

  /**
  ** @see FeatureGenerator::l0norm
  */
  virtual size_t l0norm() const;

  /**
  ** @see FeatureGenerator::l1norm
  */
  virtual double l1norm() const;

  /**
  ** @see FeatureGenerator::sumOfSquares
  */
  virtual double sumOfSquares() const;

  /**
  ** @see FeatureGenerator::addTo
  */
  virtual void addTo(DenseVectorPtr target) const;

  /**
  ** @see FeatureGenerator::addTo
  */
  virtual void addTo(SparseVectorPtr target) const;

  /**
  ** @see FeatureGenerator::substractFrom
  */
  virtual void substractFrom(DenseVectorPtr target) const;

  /**
   ** @see FeatureGenerator::substractFrom
  */
  virtual void substractFrom(SparseVectorPtr target) const;

  /**
  ** @see FeatureGenerator::addWaightedTo
  */
  virtual void addWeightedTo(DenseVectorPtr target, double weight) const;

  /**
  ** @see FeatureGenerator::addWeightedTo
  */
  virtual void addWeightedTo(SparseVectorPtr target, double weight) const;

  /**
  ** @see FeatureGenerator::addWaightedSignsTo
  */
  virtual void addWeightedSignsTo(DenseVectorPtr target, double weight) const;

  /**
  ** @see FeatureGenerator::dotProduct
  */
  virtual double dotProduct(const SparseVectorPtr vector) const;

  /**
  ** @see FeatureGenerator::dotProduct
  */
  virtual double dotProduct(const DenseVectorPtr vector) const;

  /**
  ** @see FeatureGenerator::dotProduct
  */
  virtual double dotProduct(const FeatureGeneratorPtr featureGenerator) const;

protected:
  const ExactType& _this() const {return *static_cast<const ExactType* >(this);}
};

class FlatFeatureGenerator : public FeatureGenerator
{
public:
  /**
  ** Returns the number of subgenerators.
  **
  ** @return the number of subgenerators.
  */
  virtual size_t getNumSubGenerators() const
    {return 0;}

  /**
  ** Returns subgenerator number @a num.
  **
  ** @param num : number of the subgenerator.
  **
  ** @return a FeatureGenerator instance.
  */
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {jassert(false); return FeatureGeneratorPtr();}

  /**
  ** Returns the index of the subgenerator number @a num.
  **
  ** @param num : number of the subgenerator.
  **
  ** @return the index of the subgenerator number @a num.
  */
  virtual size_t getSubGeneratorIndex(size_t num) const
    {jassert(false); return (size_t)-1;}

  /**
  ** Returns the subgenerator corresponding to the index @a index.
  **
  ** @param index : index of the subgenerator.
  **
  ** @return a FeatureGenerator instance.
  */
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
    {jassert(false); return FeatureGeneratorPtr();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_H_
