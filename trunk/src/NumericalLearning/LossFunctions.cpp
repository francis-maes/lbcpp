/*-----------------------------------------.---------------------------------.
| Filename: LossFunctions.cpp              | Numerical Learning Losses       |
| Author  : Francis Maes                   |                                 |
| Started : 21/10/2010 19:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Core/Vector.h>
#include <lbcpp/Core/DynamicObject.h>
#include <lbcpp/NumericalLearning/LossFunctions.h>
using namespace lbcpp;

/*
** RankingLossFunction
*
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
*/