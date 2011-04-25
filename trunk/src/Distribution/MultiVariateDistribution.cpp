/*-----------------------------------------.---------------------------------.
| Filename: MultiVariateProbabilityDis..cpp| MultiVariate Probability        |
| Author  : Francis Maes                   |  Distributions                  |
| Started : 22/12/2010 00:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Distribution/MultiVariateDistribution.h>
#include <lbcpp/Distribution/DistributionBuilder.h>
#include <lbcpp/Data/DoubleVector.h>
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
  std::vector<DistributionBuilderPtr> subBuilders(distributions.size());
  for (size_t i = 0; i < distributions.size(); ++i)
    subBuilders[i] = distributions[i]->createBuilder();
  return independentMultiVariateDistributionBuilder(getElementsType(), subBuilders);
}

/*
** IndependentDoubleVectorDistribution
*/
IndependentDoubleVectorDistribution::IndependentDoubleVectorDistribution(EnumerationPtr elementsEnumeration)
  : MultiVariateDistribution(independentDoubleVectorDistributionClass(elementsEnumeration)),
    elementsEnumeration(elementsEnumeration), elementsType(denseDoubleVectorClass(elementsEnumeration))
{
  subDistributions.resize(elementsEnumeration->getNumElements());
}

Variable IndependentDoubleVectorDistribution::sample(RandomGeneratorPtr random) const
{
  // /!\ This distribution only outputs normalized double vectors
  DenseDoubleVectorPtr res(new DenseDoubleVector(elementsType, subDistributions.size()));
//  double sumOfSquares = 0.0;
  for (size_t i = 0; i < subDistributions.size(); ++i)
  {
    double value = subDistributions[i]->sample(random).getDouble();
//    sumOfSquares += value * value;
    res->setValue(i, value);
  }
//   if (sumOfSquares)
//     res->multiplyByScalar(1.0 / sqrt(sumOfSquares)); // normalize
  return res;
}

DistributionBuilderPtr IndependentDoubleVectorDistribution::createBuilder() const
{
  std::vector<DistributionBuilderPtr> subBuilders(subDistributions.size());
  for (size_t i = 0; i < subBuilders.size(); ++i)
    subBuilders[i] = subDistributions[i]->createBuilder();
  return independentDoubleVectorDistributionBuilder(elementsEnumeration, subBuilders);
}
