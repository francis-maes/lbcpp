/*-----------------------------------------.---------------------------------.
 | Filename: SplitScoringFunction.h        | SplitScoringFunction            |
 | Author  : Julien Becker                 |                                 |
 | Started : 25/11/2010 13:19              |                                 |
 `------------------------------------------/                                |
                                |                                            |
                                `-------------------------------------------*/

#include "SplitScoringFunction.h"

using namespace lbcpp;

double RegressionIGSplitScoringFunction::compute(const Variable& input) const
{
  ContainerPtr leftData = input[0].getObjectAndCast<Container>();
  ContainerPtr rightData = input[1].getObjectAndCast<Container>();
  jassert(leftData && rightData);
  
  return -getLeastSquareDeviation(leftData) - getLeastSquareDeviation(rightData);
}

double RegressionIGSplitScoringFunction::getLeastSquareDeviation(ContainerPtr data) const
{
  size_t n = data->getNumElements();
  jassert(n);
  /* compute mean */
  double sum = 0;
  for (size_t i = 0; i < n; ++i)
    sum += data->getElement(i)[1].getDouble();
  double mean = sum / (double)n;
  /* compute least square */
  double leastSquare = 0;
  for (size_t i = 0; i < n; ++i)
  {
    double delta = data->getElement(i)[1].getDouble() - mean;
    leastSquare += delta * delta;
  }

  return leastSquare;
}

double ClassificationIGSplitScoringFunction::compute(const Variable& input) const
{
  ContainerPtr leftData = input[0].getObjectAndCast<Container>();
  ContainerPtr rightData = input[1].getObjectAndCast<Container>();
  jassert(leftData && rightData);
  
  EnumerationPtr enumeration = leftData->getElementsType()->getTemplateArgument(1);
  
  DiscreteProbabilityDistributionPtr leftDistribution = getDiscreteOutputDistribution(leftData);
  DiscreteProbabilityDistributionPtr rightDistribution = getDiscreteOutputDistribution(rightData);
  DiscreteProbabilityDistributionPtr priorDistribution = new DiscreteProbabilityDistribution(enumeration);
  
  for (size_t i = 0; i < enumeration->getNumElements(); ++i)
    priorDistribution->setProbability(i, (leftDistribution->getProbability(i) + rightDistribution->getProbability(i)) / 2);

#if 0
  double probOfTrue = positiveExamples->getNumElements() / (double)examples->getNumElements();
  double informationGain = priorDistribution->computeEntropy()
  - probOfTrue * positiveDistribution->computeEntropy() 
  - (1 - probOfTrue) * negativeDistribution->computeEntropy(); 
  return informationGain;
#endif // 0
  return 0.0;
}

DiscreteProbabilityDistributionPtr ClassificationIGSplitScoringFunction::getDiscreteOutputDistribution(ContainerPtr data) const
{
#if 0
  EnumerationPtr enumeration = examples->getElementsType()->getTemplateArgument(1);
  DiscreteProbabilityDistributionPtr res = new DiscreteProbabilityDistribution(enumeration);
  
  for (size_t i = 0; i < enumeration->getNumElements(); ++i)
  {
    Variable output = examples->getElement(i)[1];
    jassert(output.exists());
    res->increment(output);
  }
  res->normalize();
  return res;

#endif // 0
  return DiscreteProbabilityDistributionPtr();
}
