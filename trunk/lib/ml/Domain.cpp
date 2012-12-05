/*-----------------------------------------.---------------------------------.
| Filename: Domain.cpp                     | Mathematical Domain             |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <ml/Domain.h>
using namespace lbcpp;

/*
** DiscreteDomain
*/
void DiscreteDomain::addElement(const ObjectPtr& object)
  {elements.push_back(object);}

void DiscreteDomain::addElements(const DiscreteDomainPtr& domain)
{
  size_t s = elements.size();
  size_t os = domain->elements.size();
  elements.resize(s + os);
  for (size_t i = 0; i < os; ++i)
    elements[s + i] = domain->elements[i];
}

void DiscreteDomain::clone(ExecutionContext& context, const ObjectPtr& target) const
  {target.staticCast<DiscreteDomain>()->elements = elements;}

/*
** ScalarVectorDomain
*/
DenseDoubleVectorPtr ScalarVectorDomain::sampleUniformly(RandomGeneratorPtr random) const
{
  size_t n = limits.size();
  DenseDoubleVectorPtr res(new DenseDoubleVector(n, 0.0));
  for (size_t i = 0; i < n; ++i)
    res->setValue(i, random->sampleDouble(getLowerLimit(i), getUpperLimit(i)));
  return res;
}

ObjectPtr ScalarVectorDomain::projectIntoDomain(const ObjectPtr& object) const
{
  DenseDoubleVectorPtr solution = object.staticCast<DenseDoubleVector>();
  DenseDoubleVectorPtr res;
  size_t n = limits.size();
  for (size_t i = 0; i < n; ++i)
  {
    double value = solution->getValue(i);
    double projectedValue = juce::jlimit(limits[i].first, limits[i].second, value);
    if (value != projectedValue)
    {
      if (!res)
        res = solution->cloneAndCast<DenseDoubleVector>(); // allocate in a lazy way
      res->setValue(i, projectedValue);
    }
  }
  return res ? (ObjectPtr)res : object;
}

void ScalarVectorDomain::clone(ExecutionContext& context, const ObjectPtr& target) const
  {target.staticCast<ScalarVectorDomain>()->limits = limits;}
