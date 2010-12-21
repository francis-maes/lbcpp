/*-----------------------------------------.---------------------------------.
 | Filename: SplitScoringFunction.h        | SplitScoringFunction            |
 | Author  : Julien Becker                 |                                 |
 | Started : 25/11/2010 13:19              |                                 |
 `------------------------------------------/                                |
                                |                                            |
                                `-------------------------------------------*/

#include "SplitScoringFunction.h"
#include <lbcpp/Distribution/DistributionBuilder.h>

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

  EnumerationDistributionPtr leftDistribution = getDiscreteOutputDistribution(context, leftData);
  EnumerationDistributionPtr rightDistribution = getDiscreteOutputDistribution(context, rightData);
  DistributionBuilderPtr probabilityBuilder = createProbabilityBuilder(enumeration);

  probabilityBuilder->addDistribution(leftDistribution);
  probabilityBuilder->addDistribution(rightDistribution);

  EnumerationDistributionPtr priorDistribution = probabilityBuilder->build(context);

  double probOfTrue = leftData->getNumElements() / (double)(leftData->getNumElements() + rightData->getNumElements());
  double informationGain = priorDistribution->computeEntropy()
                          - probOfTrue * leftDistribution->computeEntropy() 
                          - (1 - probOfTrue) * rightDistribution->computeEntropy(); 
  return informationGain;
}

EnumerationDistributionPtr ClassificationIGSplitScoringFunction::getDiscreteOutputDistribution(ExecutionContext& context, ContainerPtr data) const
{
  EnumerationPtr enumeration = data->getElementsType()->getTemplateArgument(1);
  DistributionBuilderPtr probabilityBuilder = createProbabilityBuilder(enumeration);  
  for (size_t i = 0; i < data->getNumElements(); ++i)
  {
    Variable output = data->getElement(i)[1];
    jassert(output.exists());
    probabilityBuilder->addElement(output);
  }
  return probabilityBuilder->build(context);
}

DistributionBuilderPtr ClassificationIGSplitScoringFunction::createProbabilityBuilder(EnumerationPtr enumeration) const
{
  if (!cacheBuilder)
    const_cast<ClassificationIGSplitScoringFunction* >(this)->cacheBuilder = enumerationDistributionBuilder(enumeration);
  return cacheBuilder->cloneAndCast<DistributionBuilder>();
}
