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
| Filename: LossFunctions.h                | Numerical Learning Losses       |
| Author  : Francis Maes                   |                                 |
| Started : 21/10/2010 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTIONS_H_
# define LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTIONS_H_

# include "../Data/Container.h"
# include "../Core/DynamicObject.h"
# include "../Function/ScalarFunction.h"
# include "../Function/ScalarObjectFunction.h"

namespace lbcpp
{

/*
** Regression Loss Functions
*/
inline ScalarFunctionPtr squareLossFunction(double target)
  {return squareFunction(addConstantScalarFunction(-target));}

inline ScalarFunctionPtr absoluteLossFunction(double target)
  {return absFunction(addConstantScalarFunction(-target));}

inline ScalarFunctionPtr dihedralAngleSquareLossFunction(double target)
  {return squareFunction(angleDifferenceScalarFunction(target));}

/*
** Binary Classification Loss Functions
*/
class BinaryClassificationLossFunction : public ScalarFunction
{
public:
  BinaryClassificationLossFunction(bool isPositive) : isPositive(isPositive) {}
  BinaryClassificationLossFunction() {}

  virtual String toString() const;

  virtual void computePositive(double input, double* output, const double* derivativeDirection, double* derivative) const = 0;

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const;

protected:
  friend class BinaryClassificationLossFunctionClass;

  bool isPositive;
};

typedef ReferenceCountedObjectPtr<BinaryClassificationLossFunction> BinaryClassificationLossFunctionPtr;

extern BinaryClassificationLossFunctionPtr hingeLossFunction(bool isPositive, double margin = 1);
extern BinaryClassificationLossFunctionPtr logBinomialLossFunction(bool isPositive);

/*
** Multi Class Loss Functions
*/
class MultiClassLossFunction : public ScalarObjectFunction
{
public:
  MultiClassLossFunction(EnumerationPtr classes, size_t correctClass);
  MultiClassLossFunction() : correctClass(0) {}

  virtual String toString() const;
  virtual TypePtr getInputType() const
    {return containerClass(doubleType);}
  virtual void compute(ExecutionContext& context, ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const;

  virtual void compute(ExecutionContext& context, const std::vector<double>* input, double* output, std::vector<double>* gradientTarget, double gradientWeight) const = 0;

  void compute(ExecutionContext& context, const ObjectPtr& input, double* output, std::vector<double>* gradientTarget, double gradientWeight) const;

  size_t getNumClasses() const
    {return classes->getNumElements();}

  size_t getCorrectClass() const
    {return correctClass;}

protected:
  friend class MultiClassLossFunctionClass;

  EnumerationPtr classes;
  size_t correctClass;

  DynamicClassSharedPtr outputClass;
};

typedef ReferenceCountedObjectPtr<MultiClassLossFunction> MultiClassLossFunctionPtr;

extern MultiClassLossFunctionPtr oneAgainstAllMultiClassLossFunction(BinaryClassificationLossFunctionPtr binaryLossFunction, EnumerationPtr classes, size_t correctClass);
extern MultiClassLossFunctionPtr mostViolatedMultiClassLossFunction(BinaryClassificationLossFunctionPtr binaryLossFunction, EnumerationPtr classes, size_t correctClass);
extern MultiClassLossFunctionPtr logBinomialMultiClassLossFunction(EnumerationPtr classes, size_t correctClass);

/*
** Ranking Loss Functions
*/
class RankingLossFunction : public ScalarObjectFunction
{
public:
  RankingLossFunction(const std::vector<double>& costs) : costs(costs) {}
  RankingLossFunction() {}

  virtual TypePtr getInputType() const
    {return containerClass(doubleType);}

  virtual void computeRankingLoss(ExecutionContext& context, const std::vector<double>& scores, const std::vector<double>& costs, double* output, std::vector<double>* gradient) const = 0;

  virtual void compute(ExecutionContext& context, ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const;
  void compute(ExecutionContext& context, const ContainerPtr& scores, size_t numScores, double* output, std::vector<double>* gradient) const;

  const std::vector<double>& getCosts() const
    {return costs;}

  void setCosts(const std::vector<double>& costs)
    {this->costs = costs;}

protected:
  friend class RankingLossFunctionClass;

  std::vector<double> costs;

  static void multiplyOutputAndGradient(double* output, std::vector<double>* gradient, double k);
  static void sortScores(const std::vector<double>& scores, std::vector<size_t>& res);

  // returns true if all costs are equal to 0 or equal to a shared positive constant
  static bool areCostsBipartite(const std::vector<double>& costs);

  // returns a map from costs to (argmin scores, argmax scores) pairs
  static void getScoreRangePerCost(const std::vector<double>& scores, const std::vector<double>& costs, std::map<double, std::pair<size_t, size_t> >& res);
  static bool hasFewDifferentCosts(size_t numAlternatives, size_t numDifferentCosts);
};

typedef ReferenceCountedObjectPtr<RankingLossFunction> RankingLossFunctionPtr;
extern ClassPtr rankingLossFunctionClass;

extern RankingLossFunctionPtr allPairsRankingLossFunction(BinaryClassificationLossFunctionPtr baseLoss, const std::vector<double>& costs);
extern RankingLossFunctionPtr mostViolatedPairRankingLossFunction(BinaryClassificationLossFunctionPtr baseLoss, const std::vector<double>& costs);
extern RankingLossFunctionPtr bestAgainstAllRankingLossFunction(BinaryClassificationLossFunctionPtr baseLoss, const std::vector<double>& costs);

extern RankingLossFunctionPtr f1ScoreRankingLossFunction(BinaryClassificationLossFunctionPtr baseLoss, const std::vector<double>& costs);
extern RankingLossFunctionPtr mccRankingLossFunction(BinaryClassificationLossFunctionPtr baseLoss, const std::vector<double>& costs);

/*
** Regularizers
*/
extern ScalarObjectFunctionPtr l2RegularizerFunction(double weight);

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTIONS_H_
