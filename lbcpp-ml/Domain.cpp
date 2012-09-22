/*-----------------------------------------.---------------------------------.
| Filename: Domain.cpp                     | Mathematical Domain             |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/Domain.h>
using namespace lbcpp;

/*
** ContinuousDomain
*/
DenseDoubleVectorPtr ContinuousDomain::sampleUniformly(RandomGeneratorPtr random) const
{
  size_t n = limits.size();
  DenseDoubleVectorPtr res(new DenseDoubleVector(n, 0.0));
  for (size_t i = 0; i < n; ++i)
    res->setValue(i, random->sampleDouble(getLowerLimit(i), getUpperLimit(i)));
  return res;
}

ObjectPtr ContinuousDomain::projectIntoDomain(const ObjectPtr& object) const
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
  return res ? res : object;
}

void ContinuousDomain::clone(ExecutionContext& context, const ObjectPtr& target) const
  {target.staticCast<ContinuousDomain>()->limits = limits;}
