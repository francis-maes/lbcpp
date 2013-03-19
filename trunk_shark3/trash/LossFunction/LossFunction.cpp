/*-----------------------------------------.---------------------------------.
| Filename: LossFunction.cpp               | Learning Loss Functions         |
| Author  : Francis Maes                   |                                 |
| Started : 21/10/2010 19:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Learning/LossFunction.h>
#include <algorithm>
using namespace lbcpp;

/*
** RegressionLossFunction
*
size_t RegressionLossFunction::getNumRequiredInputs() const
  {return 2;}

TypePtr RegressionLossFunction::getRequiredInputType(size_t index, size_t numInputs) const
  {return doubleType;}

String RegressionLossFunction::getOutputPostFix() const
  {return T("Loss");}*/

void RegressionLossFunction::computeScalarFunction(double input, const Variable* otherInputs, double* output, double* derivative) const
  {computeRegressionLoss(input, otherInputs[0].getDouble(), output, derivative);}

/*
** DiscriminativeLossFunction
*
size_t DiscriminativeLossFunction::getNumRequiredInputs() const
  {return 2;}

TypePtr DiscriminativeLossFunction::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? sumType(booleanType, probabilityType) : doubleType;}

String DiscriminativeLossFunction::getOutputPostFix() const
  {return T("Loss");}
  */
void DiscriminativeLossFunction::computeScalarFunction(double input, const Variable* otherInputs, double* output, double* derivative) const
{
  bool isPositive;
  if (convertSupervisionVariableToBoolean(otherInputs[0], isPositive))
  {
    computeDiscriminativeLoss(isPositive ? input : -input, output, derivative);
    if (derivative && !isPositive)
      *derivative = - (*derivative);
  }
  else
    jassert(false);
}

/*
** MultiClassLossFunction
*
size_t MultiClassLossFunction::getNumRequiredInputs() const
  {return 2;}

TypePtr MultiClassLossFunction::getRequiredInputType(size_t index, size_t numInputs) const
  {return index == 1 ? sumType(enumValueType, doubleVectorClass(enumValueType, probabilityType)) : (TypePtr)denseDoubleVectorClass();}

TypePtr MultiClassLossFunction::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  classes = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());
  jassert(classes);

  TypePtr supervisionType = inputVariables[1]->getType();
  if (supervisionType != classes  && DoubleVector::getElementsEnumeration(supervisionType) != classes)
  {
    context.errorCallback(T("Type mismatch: double vector type is ") + classes->getName() + T(" supervision type is ") + inputVariables[1]->getType()->getName());
    return TypePtr();
  }
  return ScalarVectorFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
}
*/
void MultiClassLossFunction::computeScalarVectorFunction(const DenseDoubleVectorPtr& scores, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
{
  jassert(classes);
  size_t numClasses = classes->getNumElements();
  jassert(numClasses > 1);
  size_t correct;
  if (!lbcpp::convertSupervisionVariableToEnumValue(otherInputs[0], correct))
  {
    jassertfalse;
    return;
  }
  jassert(correct < numClasses);
  computeMultiClassLoss(scores, correct, numClasses, output, gradientTarget, gradientWeight);
}

/*
** RankingLossFunction
*
size_t RankingLossFunction::getNumRequiredInputs() const
  {return 2;}

TypePtr RankingLossFunction::getRequiredInputType(size_t index, size_t numInputs) const
  {return denseDoubleVectorClass();}

TypePtr RankingLossFunction::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  if (inputVariables[0]->getType() != inputVariables[1]->getType())
  {
    context.errorCallback(T("Type mismatch: scores is ") + inputVariables[0]->getType()->getName() +
                            T(" costs is ") + inputVariables[1]->getType()->getName());
    return TypePtr();
  }
  return ScalarVectorFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
}
*/
void RankingLossFunction::computeScalarVectorFunction(const DenseDoubleVectorPtr& scores, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
{
  const DenseDoubleVectorPtr& costs = otherInputs[0].getObjectAndCast<DenseDoubleVector>();
  jassert(costs->getNumElements() == scores->getNumElements());
  computeRankingLoss(scores, costs, output, gradientTarget, gradientWeight);
}

void RankingLossFunction::computeRankingLoss(const DenseDoubleVectorPtr& scores, const DenseDoubleVectorPtr& costs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
{
  jassert(scores && costs);
  if (output)
    *output = 0.0;
  std::vector<double> grad;
  if (gradientTarget)
    grad.resize(scores->getNumElements(), 0.0);
  computeRankingLoss(scores->getValues(), costs->getValues(), output, gradientTarget ? &grad : NULL);
  if (gradientTarget)
    for (size_t i = 0; i < grad.size(); ++i)
      (*gradientTarget)->incrementValue(i, grad[i] * gradientWeight);
}

void RankingLossFunction::computeRankingLoss(const std::vector<double>& scores, const std::vector<double>& costs, double* output, std::vector<double>* gradient) const
{
  jassert(false);
}

bool RankingLossFunction::areCostsBipartite(const std::vector<double>& costs, bool& zeroIsPositive)
{
  double positiveCost = 0.0;
  bool positiveCostDefined = false;
  for (size_t i = 0; i < costs.size(); ++i)
    if (costs[i])
    {
      if (positiveCostDefined)
      {
        if (costs[i] != positiveCost)
          return false;
      }
      else
        positiveCost = costs[i], positiveCostDefined = true, zeroIsPositive = (costs[i] > 0);
    }
    
  return positiveCostDefined;
}

bool RankingLossFunction::isPositiveCost(double cost, bool& zeroIsPositive)
{
  if (cost == 0)
    return zeroIsPositive;
  else
    return !zeroIsPositive;
}

// returns a map from costs to (argmin scores, argmax scores) pairs
void RankingLossFunction::getScoreRangePerCost(const std::vector<double>& scores, const std::vector<double>& costs, std::map<double, std::pair<size_t, size_t> >& res)
{
  res.clear();
  for (size_t i = 0; i < costs.size(); ++i)
  {
    double cost = costs[i];
    double score = scores[i];
    std::map<double, std::pair<size_t, size_t> >::iterator it = res.find(cost);
    if (it == res.end())
      res[cost] = std::make_pair(i, i);
    else
    {
      if (score < scores[it->second.first]) it->second.first = i;
      if (score > scores[it->second.second]) it->second.second = i;
    }
  }
}

bool RankingLossFunction::hasFewDifferentCosts(size_t numAlternatives, size_t numDifferentCosts)
  {return numAlternatives > 3 && numDifferentCosts < numAlternatives / 3;}  

void RankingLossFunction::multiplyOutputAndGradient(double* output, std::vector<double>* gradient, double k)
{
  if (k != 1.0)
  {
    if (output)
      *output *= k;
    if (gradient)
      for (size_t i = 0; i < gradient->size(); ++i)
        (*gradient)[i] *= k;
  }
}

struct CompareRankingLossScores
{
  CompareRankingLossScores(const std::vector<double>& scores) : scores(scores) {}

  const std::vector<double>& scores;

  bool operator()(size_t first, size_t second) const
    {return scores[first] > scores[second];}
};

void RankingLossFunction::sortScores(const std::vector<double>& scores, std::vector<size_t>& res)
{
  res.resize(scores.size());
  for (size_t i = 0; i < res.size(); ++i)
    res[i] = i;
  std::sort(res.begin(), res.end(), CompareRankingLossScores(scores));
}
