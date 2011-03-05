/*-----------------------------------------.---------------------------------.
 | Filename: SplitScoringFunction.h        | SplitScoringFunction            |
 | Author  : Julien Becker                 |                                 |
 | Started : 25/11/2010 13:19              |                                 |
 `------------------------------------------/                                |
                                |                                            |
                                `-------------------------------------------*/
#include "precompiled.h"
#include "SplitScoringFunction.h"
#include <lbcpp/Distribution/DistributionBuilder.h>

using namespace lbcpp;

/** RegressionIGSplitScoringFunction **/
double RegressionIGSplitScoringFunction::compute(ExecutionContext& context, const Variable& i) const
{
  const SplitScoringInputPtr& input = i.getObjectAndCast<SplitScoringInput>();  
  return -getLeastSquareDeviation(input->getLeftExamples()) - getLeastSquareDeviation(input->getRightExamples());
}

double RegressionIGSplitScoringFunction::getLeastSquareDeviation(const DecisionTreeExampleVector& examples) const
{
  size_t n = examples.getNumExamples();
  jassert(n);
  /* compute mean */
  double sum = 0;
  for (size_t i = 0; i < n; ++i)
    sum += examples.getLabel(i).getDouble();
  double mean = sum / (double)n;
  /* compute least square */
  double leastSquare = 0;
  for (size_t i = 0; i < n; ++i)
  {
    double delta = examples.getLabel(i).getDouble() - mean;
    leastSquare += delta * delta;
  }
  return leastSquare;
}

/** ClassificationIGSplitScoringFunction **/
double ClassificationIGSplitScoringFunction::compute(ExecutionContext& context, const Variable& i) const
{
  const SplitScoringInputPtr& input = i.getObjectAndCast<SplitScoringInput>();  

  EnumerationPtr enumeration = input->examples.getLabel(0).getType();

  EnumerationDistributionPtr leftDistribution = getDiscreteOutputDistribution(context, input->getLeftExamples());
  EnumerationDistributionPtr rightDistribution = getDiscreteOutputDistribution(context, input->getRightExamples());
  DistributionBuilderPtr probabilityBuilder = createProbabilityBuilder(enumeration);

  probabilityBuilder->addDistribution(leftDistribution);
  probabilityBuilder->addDistribution(rightDistribution);

  EnumerationDistributionPtr priorDistribution = probabilityBuilder->build(context);

  double probOfTrue = input->leftIndices.size() / (double)input->examples.getNumExamples();
  double informationGain = priorDistribution->computeEntropy()
                          - probOfTrue * leftDistribution->computeEntropy() 
                          - (1 - probOfTrue) * rightDistribution->computeEntropy(); 
  return informationGain;
}

EnumerationDistributionPtr ClassificationIGSplitScoringFunction::getDiscreteOutputDistribution(ExecutionContext& context, const DecisionTreeExampleVector& examples) const
{
  EnumerationPtr enumeration = examples.getLabel(0).getType();
  DistributionBuilderPtr probabilityBuilder = createProbabilityBuilder(enumeration);  
  size_t n = examples.getNumExamples();
  for (size_t i = 0; i < n; ++i)
  {
    const Variable& output = examples.getLabel(i);
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

double BinaryIGSplitScoringFunction::compute(ExecutionContext& context, const Variable& i) const
{
  const SplitScoringInputPtr& input = i.getObjectAndCast<SplitScoringInput>();  
  
  BernoulliDistributionPtr leftDistribution = getProbabilityDistribution(input->getLeftExamples());
  BernoulliDistributionPtr rightDistribution = getProbabilityDistribution(input->getRightExamples());
  BernoulliDistributionPtr priorDistribution = new BernoulliDistribution((leftDistribution->getProbabilityOfTrue() + rightDistribution->getProbabilityOfTrue()) / 2);
  
  double probOfTrue = input->leftIndices.size() / (double)input->examples.getNumExamples();
  double informationGain = priorDistribution->computeEntropy()
                          - probOfTrue * leftDistribution->computeEntropy() 
                          - (1 - probOfTrue) * rightDistribution->computeEntropy(); 
  return informationGain;
}

BernoulliDistributionPtr BinaryIGSplitScoringFunction::getProbabilityDistribution(const DecisionTreeExampleVector& examples) const
{
  size_t numOfTrue = 0;
  size_t n = examples.getNumExamples();
  for (size_t i = 0; i < n; ++i)
    if (examples.getLabel(i).getBoolean())
      ++numOfTrue;
  return new BernoulliDistribution(numOfTrue / (double)n);
}
