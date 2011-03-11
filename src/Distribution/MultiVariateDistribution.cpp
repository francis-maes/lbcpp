/*-----------------------------------------.---------------------------------.
| Filename: MultiVariateProbabilityDis..cpp| MultiVariate Probability        |
| Author  : Francis Maes                   |  Distributions                  |
| Started : 22/12/2010 00:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Distribution/MultiVariateDistribution.h>
#include "Builder/IndependentMultiVariateDistributionBuilder.h"

using namespace lbcpp;

/*
** IndependentMultiVariateDistribution
*/
IndependentMultiVariateDistribution::IndependentMultiVariateDistribution(ClassPtr elementsType)
  : MultiVariateDistribution(independentMultiVariateDistributionClass(elementsType)), distributions(elementsType->getNumMemberVariables())
{
}

double IndependentMultiVariateDistribution::computeEntropy() const
  {jassert(false); return 0.0;} // not implemented

double IndependentMultiVariateDistribution::computeProbability(const Variable& value) const
{
  const ObjectPtr& object = value.getObject();
  jassert(object->getNumVariables() == distributions.size());
  double res = 1.0;
  for (size_t i = 0; i < distributions.size(); ++i)
    res *= distributions[i]->computeProbability(object->getVariable(i));
  return res;
}

Variable IndependentMultiVariateDistribution::sample(RandomGeneratorPtr random) const
{
  ClassPtr vclass = getElementsType();
  ObjectPtr res = Object::create(vclass);
  for (size_t i = 0; i < distributions.size(); ++i)
    res->setVariable(i, distributions[i]->sample(random));
  return res;
}

Variable IndependentMultiVariateDistribution::sampleBest(RandomGeneratorPtr random) const
{
  ClassPtr vclass = getElementsType();
  ObjectPtr res = Object::create(vclass);
  for (size_t i = 0; i < distributions.size(); ++i)
    res->setVariable(i, distributions[i]->sampleBest(random));
  return res;
}

DistributionBuilderPtr IndependentMultiVariateDistribution::createBuilder() const
{
  IndependentMultiVariateDistributionBuilderPtr builder = new IndependentMultiVariateDistributionBuilder(getElementsType());
  for (size_t i = 0; i < distributions.size(); ++i)
    builder->setSubDistributionBuilder(i, distributions[i]->createBuilder());
  return builder;
}
