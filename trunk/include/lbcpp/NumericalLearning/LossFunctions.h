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

# include "../Core/Container.h"
# include "../Core/DynamicObject.h"
# include "../Learning/LossFunction.h"

namespace lbcpp
{

/*
** Regression Loss Functions
*
inline ScalarFunctionPtr oldSquareLossFunction(double target)
  {return squareFunction(addConstantScalarFunction(-target));}

inline ScalarFunctionPtr oldAbsoluteLossFunction(double target)
  {return absFunction(addConstantScalarFunction(-target));}

inline ScalarFunctionPtr oldDihedralAngleSquareLossFunction(double target)
  {return squareFunction(angleDifferenceScalarFunction(target));}

*/
/*
** Ranking Loss Functions
*/
class RankingLossFunction : public ScalarVectorFunction
{
public:
  RankingLossFunction(const std::vector<double>& costs) : costs(costs) {}
  RankingLossFunction() {}

  virtual TypePtr getInputType() const
    {return containerClass(doubleType);}

  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
    {jassert(false);} // FIXME

  virtual void computeRankingLoss(ExecutionContext& context, const std::vector<double>& scores, const std::vector<double>& costs, double* output, std::vector<double>* gradient) const = 0;

  virtual void compute(ExecutionContext& context, ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const;
  void compute(ExecutionContext& context, const ContainerPtr& scores, size_t numScores, double* output, std::vector<double>* gradient) const;

  const std::vector<double>& getCosts() const
    {return costs;}

  void setCosts(const std::vector<double>& costs)
    {this->costs = costs;}

  lbcpp_UseDebuggingNewOperator

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

extern RankingLossFunctionPtr allPairsRankingLossFunction(DiscriminativeLossFunctionPtr baseLoss, const std::vector<double>& costs);
extern RankingLossFunctionPtr mostViolatedPairRankingLossFunction(DiscriminativeLossFunctionPtr baseLoss, const std::vector<double>& costs);
extern RankingLossFunctionPtr bestAgainstAllRankingLossFunction(DiscriminativeLossFunctionPtr baseLoss, const std::vector<double>& costs);

extern RankingLossFunctionPtr f1ScoreRankingLossFunction(DiscriminativeLossFunctionPtr baseLoss, const std::vector<double>& costs);
extern RankingLossFunctionPtr mccRankingLossFunction(DiscriminativeLossFunctionPtr baseLoss, const std::vector<double>& costs);

/*
** Regularizers
*/
extern ScalarVectorFunctionPtr l2RegularizerFunction(double weight);

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTIONS_H_
