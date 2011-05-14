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
