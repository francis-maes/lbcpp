/*-----------------------------------------.---------------------------------.
| Filename: MeanDoubleVectorAggregator.h   | Mean Double Vector Aggregator   |
| Author  : Francis Maes                   |                                 |
| Started : 20/11/2012 17:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_AGGREGATOR_MEAN_DOUBLE_VECTOR_H_
# define ML_AGGREGATOR_MEAN_DOUBLE_VECTOR_H_

# include <ml/Aggregator.h>
# include <ml/Expression.h>
# include <ml/DoubleVector.h>

namespace lbcpp
{

class MeanDoubleVectorAggregator : public Aggregator
{
public:
  virtual bool doAcceptInputType(const ClassPtr& type) const
    {return type->inheritsFrom(denseDoubleVectorClass());}

  virtual ClassPtr initialize(const ClassPtr& inputsType)
    {return inputsType;}

  virtual ObjectPtr compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs, ClassPtr outputType) const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(outputType);
    if (inputs.size())
    {
      for (size_t i = 0; i < inputs.size(); ++i)
        inputs[i].staticCast<DenseDoubleVector>()->addTo(res);
      res->multiplyByScalar(1.0 / inputs.size());
    }
    return res;
  }

  struct AccumulatorData : public Object
  {
    AccumulatorData(IndexSetPtr indices, EnumerationPtr elementsEnumeration)
    : indices(indices), elementsEnumeration(elementsEnumeration), v(elementsEnumeration->getNumElements() * indices->size(), 0.0), count(0) {}
    
    IndexSetPtr indices;
    EnumerationPtr elementsEnumeration;
    std::vector<double> v;
    size_t count;
  };
  typedef ReferenceCountedObjectPtr<AccumulatorData> AccumulatorDataPtr;

  virtual ObjectPtr startAggregation(const IndexSetPtr& indices, ClassPtr inputsType, ClassPtr outputType) const
    {return new AccumulatorData(indices, DoubleVector::getElementsEnumeration(outputType));}

  virtual void updateAggregation(const ObjectPtr& d, const DataVectorPtr& inputs) const
  {
    const AccumulatorDataPtr& data = d.staticCast<AccumulatorData>();
    
    jassert(data->indices == inputs->getIndices());
    jassert(inputs->getElementsType()->inheritsFrom(denseDoubleVectorClass()));
    
    double* dest = &data->v[0];
    for (DataVector::const_iterator it = inputs->begin(); it != inputs->end(); ++it)
    {
      const DenseDoubleVectorPtr& input = it.getRawObject().staticCast<DenseDoubleVector>();
      if (input)
      {
        jassert(data->elementsEnumeration->getNumElements() == input->getNumValues());
        for (size_t j = 0; j < input->getNumValues(); ++j)
        {
          double value = input->getValue(j);
          jassert(isNumberValid(value));
          *dest++ += value;
        }
      }
      else
        dest += data->elementsEnumeration->getNumElements();
    }
    data->count++;
  }

  virtual DataVectorPtr finalizeAggregation(const ObjectPtr& d) const
  {
    const AccumulatorDataPtr& data = d.staticCast<AccumulatorData>();
    ClassPtr dvClass = denseDoubleVectorClass(data->elementsEnumeration, doubleClass);
    const OVectorPtr& res = new OVector(dvClass, data->indices->size());
    double invZ = 1.0 / (double)data->count;
    double* ptr = &data->v[0];
    for (size_t i = 0; i < res->getNumElements(); ++i)
    {
      DenseDoubleVectorPtr means = new DenseDoubleVector(dvClass);
      for (size_t j = 0; j < means->getNumValues(); ++j)
        means->setValue(j, *ptr++ * invZ);
      res->set(i, means);
    }
    return new DataVector(data->indices, res);
  }
};

}; /* namespace lbcpp */

#endif // !ML_AGGREGATOR_MEAN_DOUBLE_VECTOR_H_
