/*-----------------------------------------.---------------------------------.
| Filename: Sampler.cpp                    | Base classes for Samplers       |
| Author  : Francis Maes                   |                                 |
| Started : 14/05/2011 19:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Sampler/Sampler.h>
using namespace lbcpp;

/*
** ConstantSampler
*/
Variable ConstantSampler::computeExpectation(const Variable* inputs) const
  {return value;}

Variable ConstantSampler::sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs) const
  {return value;}

void ConstantSampler::learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                              const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& validationWeights)
  {}

DenseDoubleVectorPtr ConstantSampler::computeProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
{
  size_t n = samples->getNumElements();
  DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);
  for (size_t i = 0; i < n; ++i)
    if (samples->getElement(i) == value)
      res->setValue(i, 1.0);
  return res;
}

/*
** CompositeSampler
*/
void CompositeSampler::learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                        const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
{
  std::vector<ContainerPtr> subTrainingInputs(samplers.size()), subTrainingSamples(samplers.size()), subTrainingWeights(samplers.size());
  makeSubExamples(trainingInputs, trainingSamples, trainingWeights, subTrainingInputs, subTrainingSamples, subTrainingWeights);
  jassert(subTrainingInputs.size() == samplers.size() && subTrainingSamples.size() == samplers.size());

  if (validationSamples)
  {
    std::vector<ContainerPtr> subValidationInputs(samplers.size()), subValidationSamples(samplers.size()), subValidationWeights(samplers.size());
    makeSubExamples(validationInputs, validationSamples, supervisionWeights, subValidationInputs, subValidationSamples, subValidationWeights);
    jassert(subValidationInputs.size() == samplers.size() && subValidationSamples.size() == samplers.size());
    
    for (size_t i = 0; i < samplers.size(); ++i)
      if (subTrainingSamples[i]->getNumElements())
        samplers[i]->learn(context, subTrainingInputs[i], subTrainingSamples[i], subTrainingWeights[i], subValidationInputs[i], subValidationSamples[i], subValidationWeights[i]);
  }
  else
  {
    for (size_t i = 0; i < samplers.size(); ++i)
      if (subTrainingSamples[i]->getNumElements())
        samplers[i]->learn(context, subTrainingInputs[i], subTrainingSamples[i]);
  }
}

void CompositeSampler::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  Sampler::clone(context, t);
  const CompositeSamplerPtr& target = t.staticCast<CompositeSampler>();
  for (size_t i = 0; i < samplers.size(); ++i)
    target->samplers[i] = samplers[i]->cloneAndCast<Sampler>();
}

String CompositeSampler::toShortString() const
{
  String res = T("(");
  for (size_t i = 0; i < samplers.size(); ++i)
  {
    res += samplers[i]->toShortString();
    if (i < samplers.size() - 1)
      res += T(", ");
  }
  return res + T(")");
}

/*
** ObjectCompositeSampler
*/
CompositeSamplerPtr lbcpp::objectCompositeSampler(ClassPtr objectClass, const SamplerPtr& firstVariableSampler)
{
  std::vector<SamplerPtr> samplers(1);
  samplers[0] = firstVariableSampler;
  return objectCompositeSampler(objectClass, samplers);
}

CompositeSamplerPtr lbcpp::objectCompositeSampler(ClassPtr objectClass, const SamplerPtr& firstVariableSampler, const SamplerPtr& secondVariableSampler)
{
  std::vector<SamplerPtr> samplers(2);
  samplers[0] = firstVariableSampler;
  samplers[1] = secondVariableSampler;
  return objectCompositeSampler(objectClass, samplers);
}

CompositeSamplerPtr lbcpp::objectCompositeSampler(ClassPtr objectClass, const SamplerPtr& firstVariableSampler, const SamplerPtr& secondVariableSampler, const SamplerPtr& thirdVariableSampler)
{
  std::vector<SamplerPtr> samplers(3);
  samplers[0] = firstVariableSampler;
  samplers[1] = secondVariableSampler;
  samplers[2] = thirdVariableSampler;
  return objectCompositeSampler(objectClass, samplers);
}

/*
** DecoratorSampler
*/
Variable DecoratorSampler::computeExpectation(const Variable* inputs) const
  {return sampler->computeExpectation(inputs);}

Variable DecoratorSampler::sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs) const
  {return sampler->sample(context, random, inputs);}

void DecoratorSampler::learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                              const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& validationWeights)
  {return sampler->learn(context, trainingInputs, trainingSamples, trainingWeights, validationInputs, validationSamples, validationWeights);}

DenseDoubleVectorPtr DecoratorSampler::computeProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
  {return sampler->computeProbabilities(inputs, samples);}

void DecoratorSampler::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  Sampler::clone(context, target);
  target.staticCast<DecoratorSampler>()->sampler = sampler->cloneAndCast<Sampler>(context);
}
