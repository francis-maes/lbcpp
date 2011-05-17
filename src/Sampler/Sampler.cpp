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
** CompositeSampler
*/
void CompositeSampler::learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, 
                                                        const ContainerPtr& validationInputs, const ContainerPtr& validationSamples)
{
  std::vector<ContainerPtr> subTrainingInputs(samplers.size()), subTrainingSamples(samplers.size());
  makeSubExamples(trainingInputs, trainingSamples, subTrainingInputs, subTrainingSamples);
  jassert(subTrainingInputs.size() == samplers.size() && subTrainingSamples.size() == samplers.size());

  if (validationSamples)
  {
    std::vector<ContainerPtr> subValidationInputs(samplers.size()), subValidationSamples(samplers.size());
    makeSubExamples(validationInputs, validationSamples, subValidationInputs, subValidationSamples);
    jassert(subValidationInputs.size() == samplers.size() && subValidationSamples.size() == samplers.size());
    
    for (size_t i = 0; i < samplers.size(); ++i)
      if (subTrainingSamples[i]->getNumElements())
        samplers[i]->learn(context, subTrainingInputs[i], subTrainingSamples[i], subValidationInputs[i], subValidationSamples[i]);
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
