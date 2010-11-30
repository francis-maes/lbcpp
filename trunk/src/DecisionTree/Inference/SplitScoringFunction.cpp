/*-----------------------------------------.---------------------------------.
 | Filename: SplitScoringFunction.h        | SplitScoringFunction            |
 | Author  : Julien Becker                 |                                 |
 | Started : 25/11/2010 13:19              |                                 |
 `------------------------------------------/                                |
                                |                                            |
                                `-------------------------------------------*/

#include "SplitScoringFunction.h"
#include <lbcpp/ProbabilityDistribution/ProbabilityDistributionBuilder.h>

using namespace lbcpp;

/** RegressionIGSplitScoringFunction **/
double RegressionIGSplitScoringFunction::compute(ExecutionContext& context, const Variable& input) const
{
  ContainerPtr leftData = input[0].getObjectAndCast<Container>();
  ContainerPtr rightData = input[1].getObjectAndCast<Container>();
  jassert(leftData && rightData);
  
  return - getLeastSquareDeviation(leftData) - getLeastSquareDeviation(rightData);
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

/** ClassificationIGSplitScoringFunction **/
double ClassificationIGSplitScoringFunction::compute(ExecutionContext& context, const Variable& input) const
{
  ContainerPtr leftData = input[0].getObjectAndCast<Container>();
  ContainerPtr rightData = input[1].getObjectAndCast<Container>();
  jassert(leftData && rightData);

  EnumerationPtr enumeration = leftData->getElementsType()->getTemplateArgument(1);

  EnumerationProbabilityDistributionPtr leftDistribution = getDiscreteOutputDistribution(context, leftData);
  EnumerationProbabilityDistributionPtr rightDistribution = getDiscreteOutputDistribution(context, rightData);
  ProbabilityDistributionBuilderPtr probabilityBuilder = enumerationProbabilityDistributionBuilder(enumeration);

  probabilityBuilder->addDistribution(leftDistribution);
  probabilityBuilder->addDistribution(rightDistribution);

  EnumerationProbabilityDistributionPtr priorDistribution = probabilityBuilder->build();

  double probOfTrue = leftData->getNumElements() / (double)(leftData->getNumElements() + rightData->getNumElements());
  double informationGain = priorDistribution->computeEntropy()
                          - probOfTrue * leftDistribution->computeEntropy() 
                          - (1 - probOfTrue) * rightDistribution->computeEntropy(); 
  return informationGain;
}

EnumerationProbabilityDistributionPtr ClassificationIGSplitScoringFunction::getDiscreteOutputDistribution(ExecutionContext& context, ContainerPtr data) const
{
  EnumerationPtr enumeration = data->getElementsType()->getTemplateArgument(1);
  ProbabilityDistributionBuilderPtr probabilityBuilder = enumerationProbabilityDistributionBuilder(enumeration);  
  for (size_t i = 0; i < data->getNumElements(); ++i)
  {
    Variable output = data->getElement(i)[1];
    jassert(output.exists());
    probabilityBuilder->addElement(output);
  }
  return probabilityBuilder->build();
}
