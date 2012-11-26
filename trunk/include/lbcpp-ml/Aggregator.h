/*-----------------------------------------.---------------------------------.
| Filename: Aggregator.h                   | Aggregator                      |
| Author  : Francis Maes                   |                                 |
| Started : 20/11/2012 17:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_AGGREGATOR_H_
# define LBCPP_ML_AGGREGATOR_H_

# include "predeclarations.h"
# include <lbcpp/Core.h>
# include <lbcpp/Data/IndexSet.h>

namespace lbcpp
{

class Aggregator : public Object
{
public:
  // types / initialization
  virtual bool doAcceptInputType(const ClassPtr& type) const = 0; 
  virtual ClassPtr initialize(const ClassPtr& inputsType) = 0; // returns the output type

  // compute for a single instance
  virtual ObjectPtr compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs, ClassPtr outputType) const = 0;

  // compute for a batch of instances
  virtual DataVectorPtr compute(ExecutionContext& context, const std::vector<DataVectorPtr>& inputs, ClassPtr outputType) const;

  virtual ObjectPtr startAggregation(const IndexSetPtr& indices, ClassPtr outputType) const = 0;
  virtual void updateAggregation(const ObjectPtr& data, const DataVectorPtr& inputs) const = 0;
  virtual DataVectorPtr finalizeAggregation(const ObjectPtr& data) const = 0;
};

typedef ReferenceCountedObjectPtr<Aggregator> AggregatorPtr;

extern AggregatorPtr meanDoubleVectorAggregator();

}; /* namespace lbcpp */

#endif // !LBCPP_ML_AGGREGATOR_H_
