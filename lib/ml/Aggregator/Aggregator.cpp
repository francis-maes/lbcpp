/*-----------------------------------------.---------------------------------.
| Filename: Aggregator.cpp                 | Aggregator Base Class           |
| Author  : Francis Maes                   |                                 |
| Started : 14/03/2013 15:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <ml/Aggregator.h>
#include <ml/Expression.h>
#include <ml/DoubleVector.h>
#include <ml/RandomVariable.h>
using namespace lbcpp;

DataVectorPtr Aggregator::compute(ExecutionContext& context, const std::vector<DataVectorPtr>& inputs, ClassPtr outputType) const
{
  if (!inputs.size())
    return DataVectorPtr();
  ObjectPtr data = startAggregation(inputs[0]->getIndices(), inputs[0]->getElementsType(), outputType);
  for (size_t i = 0; i < inputs.size(); ++i)
    updateAggregation(data, inputs[i]);
  return finalizeAggregation(data);
}

std::pair<AggregatorPtr, ClassPtr> Aggregator::create(ClassPtr supervisionType)
{
  if (supervisionType->inheritsFrom(doubleClass))
    return std::make_pair(statisticsDoubleAggregator(), scalarVariableStatisticsClass);
  else if (supervisionType.isInstanceOf<Enumeration>())
    return std::make_pair(meanDoubleVectorAggregator(), denseDoubleVectorClass(supervisionType.staticCast<Enumeration>(), doubleClass));
  else if (supervisionType->inheritsFrom(denseDoubleVectorClass()))
    return std::make_pair(statisticsDoubleVectorAggregator(), vectorClass(scalarVariableStatisticsClass));
  else
  {
    jassertfalse; // not implemented yet
    return std::make_pair(AggregatorPtr(), ClassPtr());
  }
}
