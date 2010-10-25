/*-----------------------------------------.---------------------------------.
| Filename: LossFunctions.cpp              | Numerical Learning Losses       |
| Author  : Francis Maes                   |                                 |
| Started : 21/10/2010 19:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/NumericalLearning/LossFunctions.h>
#include "../Data/Object/DenseDoubleObject.h"
#include <algorithm>
using namespace lbcpp;

/*
** BinaryClassificationLossFunction
*/
String BinaryClassificationLossFunction::toString() const
  {return getClassName() + T("(") + (isPositive ? T("+") : T("-")) + T(")");}

void BinaryClassificationLossFunction::compute(double input, double* output, const double* derivativeDirection, double* derivative) const
{
  double dd;
  if (derivativeDirection)
    dd = isPositive ? *derivativeDirection : -(*derivativeDirection);
  computePositive(isPositive ? input : -input, output, derivativeDirection ? &dd : NULL, derivative);
  if (derivative && !isPositive)
    *derivative = - (*derivative);
}

/*
** MultiClassLossFunction
*/
MultiClassLossFunction::MultiClassLossFunction(EnumerationPtr classes, size_t correctClass)
  : classes(classes), correctClass(correctClass) {}

String MultiClassLossFunction::toString() const
  {return getClassName() + T("(") + String((int)correctClass) + T(")");}

void MultiClassLossFunction::compute(ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const
{
  DenseDoubleObjectPtr denseDoubleInput = input.dynamicCast<DenseDoubleObject>();
  if (!input || denseDoubleInput)
  {
    jassert(!gradientTarget || !*gradientTarget || gradientTarget->isInstanceOf<DenseDoubleObject>());
    std::vector<double>* gradientVectorTarget = NULL;
    if (gradientTarget)
    {
      DenseDoubleObjectPtr denseGradientTarget;
      if (*gradientTarget)
        denseGradientTarget = gradientTarget->dynamicCast<DenseDoubleObject>();
      else
      {
        DynamicClassSharedPtr cl = enumBasedDoubleVectorClass(classes).staticCast<DynamicClass>().get();
        denseGradientTarget = new DenseDoubleObject(cl, 0.0);
        *gradientTarget = denseGradientTarget;
      }
      jassert(denseGradientTarget);
      gradientVectorTarget = &denseGradientTarget->getValues();
      jassert(gradientVectorTarget->size() == classes->getNumElements());
    }

    compute(denseDoubleInput ? &denseDoubleInput->getValues() : NULL, output, gradientVectorTarget, gradientWeight);
  }
  else
    jassert(false); // not implemented
}

/*
** RankingLossFunction
*/
void RankingLossFunction::compute(ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const
{
  DenseDoubleObjectPtr denseDoubleInput = input.dynamicCast<DenseDoubleObject>();
  if (denseDoubleInput)
  {
    std::vector<double> gradient;
    if (gradientTarget)
      gradient.resize(denseDoubleInput->getValues().size(), 0.0);
    computeRankingLoss(denseDoubleInput->getValues(), costs, output, gradientTarget ? &gradient : NULL);

    if (gradientTarget)
    {
      DenseDoubleObjectPtr denseGradientTarget;
      if (*gradientTarget)
        denseGradientTarget = gradientTarget->dynamicCast<DenseDoubleObject>();
      else
      {
        denseGradientTarget = denseDoubleInput->createCompatibleNullObject();
        *gradientTarget = denseGradientTarget;
      }
      for (size_t i = 0; i < gradient.size(); ++i)
        denseGradientTarget->getValueReference(i) += gradient[i] * gradientWeight;
    }
  }
  else
    jassert(false); // not implemented
}

bool RankingLossFunction::areCostsBipartite(const std::vector<double>& costs)
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
        positiveCost = costs[i], positiveCostDefined = true;
    }
    
  return positiveCostDefined;
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
  {return numAlternatives > 3 && (double)numAlternatives < 2.5 * numDifferentCosts;}  

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

/*
** AdditiveRankingLossFunction
*/
void AdditiveRankingLossFunction::addRankingPair(double deltaCost, double deltaScore, size_t positiveAlternative, size_t negativeAlternative, double* output, std::vector<double>* gradient) const
{
  assert(deltaCost > 0);
  // deltaScore = scores[positiveAlternative] - scores[negativeAlternative]
  // deltaScore should be positive
  
  double discriminantValue, discriminantDerivative;
  baseLoss->computePositive(deltaScore, output ? &discriminantValue : NULL, NULL, gradient ? &discriminantDerivative : NULL);
  if (gradient)
  {
    double delta = deltaCost * discriminantDerivative;
    (*gradient)[positiveAlternative] += delta;
    (*gradient)[negativeAlternative] -= delta;
  }
  if (output)
    *output += deltaCost * discriminantValue;
}
