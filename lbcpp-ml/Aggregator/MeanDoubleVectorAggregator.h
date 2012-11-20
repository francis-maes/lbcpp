/*-----------------------------------------.---------------------------------.
| Filename: MeanDoubleVectorAggregator.h   | Mean Double Vector Aggregator   |
| Author  : Francis Maes                   |                                 |
| Started : 20/11/2012 17:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_AGGREGATOR_MEAN_DOUBLE_VECTOR_H_
# define LBCPP_ML_AGGREGATOR_MEAN_DOUBLE_VECTOR_H_

# include <lbcpp-ml/Aggregator.h>
# include <lbcpp-ml/Expression.h>
# include <lbcpp/Data/DoubleVector.h>

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

  virtual DataVectorPtr initializeOutputs(const IndexSetPtr& indices, ClassPtr outputType) const
  {
    OVectorPtr res = new OVector(outputType, indices->size());
    for (size_t i = 0; i < indices->size(); ++i)
      res->set(i, new DenseDoubleVector(outputType));
    return new DataVector(indices, res);
  }

  virtual void updateOutputs(const DataVectorPtr& outputs, const DataVectorPtr& inputs) const
  {
    const OVectorPtr& a = outputs->getVector().staticCast<OVector>();
    jassert(inputs->size() == a->getNumElements());
    jassert(inputs->getElementsType()->inheritsFrom(denseDoubleVectorClass()));
    size_t i = 0;
    for (DataVector::const_iterator it = inputs->begin(); it != inputs->end(); ++it, ++i)
    {
      const DenseDoubleVectorPtr& input = it.getRawObject().staticCast<DenseDoubleVector>();
      if (input)
      {
        DenseDoubleVectorPtr target = a->get(i).staticCast<DenseDoubleVector>();
        input->addTo(target);
  #ifdef JUCE_DEBUG
        for (size_t j = 0; j < target->getNumValues(); ++j)
          jassert(isNumberValid(target->getValue(j)));
  #endif // JUCE_DBEUG
      }
    }
  }

  virtual void finalizeOutputs(const DataVectorPtr& outputs, size_t numAggregatedElements) const
  {
    const OVectorPtr& a = outputs->getVector().staticCast<OVector>();
    double invZ = 1.0 / (double)numAggregatedElements;
    for (size_t i = 0; i < a->getNumElements(); ++i)
    {
      DenseDoubleVectorPtr output = a->get(i).staticCast<DenseDoubleVector>();
      output->multiplyByScalar(invZ);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_AGGREGATOR_MEAN_DOUBLE_VECTOR_H_
