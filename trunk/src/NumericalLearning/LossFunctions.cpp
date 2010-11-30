/*-----------------------------------------.---------------------------------.
| Filename: LossFunctions.cpp              | Numerical Learning Losses       |
| Author  : Francis Maes                   |                                 |
| Started : 21/10/2010 19:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Core/Vector.h>
#include <lbcpp/NumericalLearning/LossFunctions.h>
#include "../Core/Object/DenseDoubleObject.h"
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
  : classes(classes), correctClass(correctClass)
{
}

String MultiClassLossFunction::toString() const
  {return getClassName() + T("(") + String((int)correctClass) + T(")");}

void MultiClassLossFunction::compute(ExecutionContext& context, const ObjectPtr& input, double* output, std::vector<double>* gradientTarget, double gradientWeight) const
{
  DenseDoubleObjectPtr denseDoubleInput = input.dynamicCast<DenseDoubleObject>();
  if (!input || denseDoubleInput)
  {
    if (gradientTarget)
    {
      gradientTarget->clear();
      gradientTarget->resize(classes->getNumElements(), 0.0);
    }
    compute(context, denseDoubleInput ? &denseDoubleInput->getValues() : NULL, output, gradientTarget, gradientWeight);
  }
  else
    jassert(false); // not implemented
}

void MultiClassLossFunction::compute(ExecutionContext& context, ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const
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
      if (!outputClass)
        const_cast<MultiClassLossFunction* >(this)->outputClass = enumBasedDoubleVectorClass(classes).staticCast<DynamicClass>().get();
      denseGradientTarget = new DenseDoubleObject(outputClass, 0.0);
      *gradientTarget = denseGradientTarget;
    }
    jassert(denseGradientTarget);
    gradientVectorTarget = &denseGradientTarget->getValues();
    jassert(gradientVectorTarget->size() == classes->getNumElements());
  }

  compute(context, input, output, gradientVectorTarget, gradientWeight);
}

/*
** RankingLossFunction
*/
void RankingLossFunction::compute(ExecutionContext& context, const ContainerPtr& scores, size_t numScores, double* output, std::vector<double>* gradient) const
{
  jassert(!scores || scores->getNumElements() == numScores);
  std::vector<double> scoreVector(numScores);
  for (size_t i = 0; i < numScores; ++i)
    scoreVector[i] = scores ? scores->getElement(i).getDouble() : 0.0;
  if (output)
    *output = 0.0;
  if (gradient)
    gradient->resize(numScores, 0.0);
  computeRankingLoss(context, scoreVector, costs, output, gradient);
}

void RankingLossFunction::compute(ExecutionContext& context, ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const
{
  const ContainerPtr& scores = input.staticCast<Container>();
  jassert(scores);
  size_t n = scores->getNumElements();

  std::vector<double> gradient;
  compute(context, input, n, output, gradientTarget ? &gradient : NULL);
  if (gradientTarget)
  {
    ContainerPtr target;
    if (!*gradientTarget)
    {
      *gradientTarget = target = lbcpp::vector(doubleType, n);
      for (size_t i = 0; i < n; ++i)
        target->setElement(i, gradient[i] * gradientWeight);
    }
    else
    {
      target = gradientTarget->staticCast<Container>();
      for (size_t i = 0; i < gradient.size(); ++i)
        target->setElement(i, target->getElement(i).getDouble() + gradient[i] * gradientWeight);
    }
  }
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
