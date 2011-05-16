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
  std::vector<ContainerPtr> subTrainingInputs, subTrainingSamples;
  makeSubExamples(trainingInputs, trainingSamples, subTrainingInputs, subTrainingSamples);
  jassert(subTrainingInputs.size() == samplers.size() && subTrainingSamples.size() == samplers.size());

  if (validationSamples)
  {
    std::vector<ContainerPtr> subValidationInputs, subValidationSamples;
    makeSubExamples(validationInputs, validationSamples, subValidationInputs, subValidationSamples);
    jassert(subValidationInputs.size() == samplers.size() && subValidationSamples.size() == samplers.size());
    
    for (size_t i = 0; i < samplers.size(); ++i)
      samplers[i]->learn(context, subTrainingInputs[i], subTrainingSamples[i], subValidationInputs[i], subValidationSamples[i]);
  }
  else
  {
    for (size_t i = 0; i < samplers.size(); ++i)
      samplers[i]->learn(context, subTrainingInputs[i], subTrainingSamples[i]);
  }
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
