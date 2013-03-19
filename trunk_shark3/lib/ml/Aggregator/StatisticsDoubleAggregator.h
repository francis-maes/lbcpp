/*-----------------------------------------.---------------------------------.
 | Filename: StatisticsDoubleAggregator.h   | Statistics Double Aggregator    |
 | Author  : Denny Verbeeck                 |                                 |
 | Started : 19/02/2013 18:00               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef ML_AGGREGATOR_STATISTICS_DOUBLE_H_
# define ML_AGGREGATOR_STATISTICS_DOUBLE_H_

# include <ml/Aggregator.h>
# include <ml/Expression.h>
# include <ml/RandomVariable.h>

namespace lbcpp
{

class StatisticsDoubleAggregator : public Aggregator
{
public:
  virtual bool doAcceptInputType(const ClassPtr& type) const
    {return type->inheritsFrom(doubleClass);}
  
  virtual ClassPtr initialize(const ClassPtr& inputsType)
    {return scalarVariableStatisticsClass;}
  
  virtual ObjectPtr compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs, ClassPtr outputType) const
  {
    ScalarVariableStatisticsPtr res = new ScalarVariableStatistics();
    for (size_t i = 0; i < inputs.size(); ++i)
      res->push(Double::get(inputs[i]));
    return res;
  }
  
  struct AccumulatorData : public Object
  {
    AccumulatorData(IndexSetPtr indices, ClassPtr elementsType)
      : indices(indices), elementsType(elementsType), v(indices->size())
    {
      for (size_t i = 0; i < v.size(); ++i)
        v[i] = new ScalarVariableStatistics();
    }
    
    IndexSetPtr indices;
    ClassPtr elementsType;
    std::vector<ScalarVariableStatisticsPtr> v;
  };
  typedef ReferenceCountedObjectPtr<AccumulatorData> AccumulatorDataPtr;
  
  virtual ObjectPtr startAggregation(const IndexSetPtr& indices, ClassPtr inputsType, ClassPtr outputType) const
    {return new AccumulatorData(indices, outputType);}
  
  virtual void updateAggregation(const ObjectPtr& d, const DataVectorPtr& inputs) const
  {
    const AccumulatorDataPtr& data = d.staticCast<AccumulatorData>();
    
    jassert(data->indices == inputs->getIndices());
    jassert(inputs->getElementsType()->inheritsFrom(doubleClass));
    
    ScalarVariableStatisticsPtr* dest = &data->v[0];
    for (DataVector::const_iterator it = inputs->begin(); it != inputs->end(); ++it)
      (*dest++)->push(it.getRawDouble());
  }
  
  virtual DataVectorPtr finalizeAggregation(const ObjectPtr& d) const
  {
    const AccumulatorDataPtr& data = d.staticCast<AccumulatorData>();
    const OVectorPtr& res = new OVector(data->elementsType, data->indices->size());
    ScalarVariableStatisticsPtr* ptr = &data->v[0];
    for (size_t i = 0; i < res->getNumElements(); ++i)
      res->set(i, *ptr++);
    return new DataVector(data->indices, res);
  }
};
  
}; /* namespace lbcpp */

#endif // !ML_AGGREGATOR_STATISTICS_DOUBLE_H_
