/*-----------------------------------------.---------------------------------.
| Filename: MeanDoubleAggregator.h         | Mean Double Aggregator          |
| Author  : Francis Maes                   |                                 |
| Started : 27/11/2012 14:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_AGGREGATOR_MEAN_DOUBLE_H_
# define ML_AGGREGATOR_MEAN_DOUBLE_H_

# include <ml/Aggregator.h>
# include <ml/Expression.h>
# include <ml/RandomVariable.h>

namespace lbcpp
{

class MeanDoubleAggregator : public Aggregator
{
public:
  virtual bool doAcceptInputType(const ClassPtr& type) const
    {return type->inheritsFrom(doubleClass);}

  virtual ClassPtr initialize(const ClassPtr& inputsType)
    {return inputsType;}

  virtual ObjectPtr compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs, ClassPtr outputType) const
  {
    ScalarVariableMean mean;
    for (size_t i = 0; i < inputs.size(); ++i)
      mean.push(Double::get(inputs[i]));
    return Double::create(outputType, mean.getMean());
  }

  struct AccumulatorData : public Object
  {
    AccumulatorData(IndexSetPtr indices, ClassPtr elementsType)
    : indices(indices), elementsType(elementsType), v(indices->size(), 0.0), count(0) {}
    
    IndexSetPtr indices;
    ClassPtr elementsType;
    std::vector<double> v;
    size_t count;
  };
  typedef ReferenceCountedObjectPtr<AccumulatorData> AccumulatorDataPtr;

  virtual ObjectPtr startAggregation(const IndexSetPtr& indices, ClassPtr outputType) const
    {return new AccumulatorData(indices, outputType);}

  virtual void updateAggregation(const ObjectPtr& d, const DataVectorPtr& inputs) const
  {
    const AccumulatorDataPtr& data = d.staticCast<AccumulatorData>();
    
    jassert(data->indices == inputs->getIndices());
    jassert(inputs->getElementsType()->inheritsFrom(data->elementsType));
    
    double* dest = &data->v[0];
    for (DataVector::const_iterator it = inputs->begin(); it != inputs->end(); ++it)
    {
      double value = it.getRawDouble();
      *dest++ += value;
    }
    data->count++;
  }

  virtual DataVectorPtr finalizeAggregation(const ObjectPtr& d) const
  {
    const AccumulatorDataPtr& data = d.staticCast<AccumulatorData>();
    const DVectorPtr& res = new DVector(data->elementsType, data->indices->size());
    double invZ = 1.0 / (double)data->count;
    double* ptr = &data->v[0];
    for (size_t i = 0; i < res->getNumElements(); ++i)
      res->set(i, *ptr++ * invZ);
    return new DataVector(data->indices, res);
  }
};

}; /* namespace lbcpp */

#endif // !ML_AGGREGATOR_MEAN_DOUBLE_H_
