/*-----------------------------------------.---------------------------------.
| Filename: MultiVariateProbabilityDis..cpp| MultiVariate Probability        |
| Author  : Francis Maes                   |  Distributions                  |
| Started : 22/12/2010 00:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Distribution/MultiVariateDistribution.h>
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

double IndependentMultiVariateDistribution::compute(ExecutionContext& context, const Variable& value) const
{
  const ObjectPtr& object = value.getObject();
  jassert(object->getNumVariables() == distributions.size());
  double res = 1.0;
  for (size_t i = 0; i < distributions.size(); ++i)
    res *= distributions[i]->compute(context, object->getVariable(i));
  return res;
}

Variable IndependentMultiVariateDistribution::sample(RandomGeneratorPtr random) const
{
  ClassPtr vclass = getElementsType();
  ObjectPtr res = Object::create(vclass);
  for (size_t i = 0; i < distributions.size(); ++i)
    res->setVariable(defaultExecutionContext(), i, distributions[i]->sample(random));
  return res;
}

Variable IndependentMultiVariateDistribution::sampleBest(RandomGeneratorPtr random) const
{
  ClassPtr vclass = getElementsType();
  ObjectPtr res = Object::create(vclass);
  for (size_t i = 0; i < distributions.size(); ++i)
    res->setVariable(defaultExecutionContext(), i, distributions[i]->sampleBest(random));
  return res;
}
