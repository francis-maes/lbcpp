/*-----------------------------------------.---------------------------------.
| Filename: IndependentDoubleVectorSampler.h| Samples vectors in R^n         |
| Author  : Francis Maes                   |                                 |
| Started : 13/05/2011 19:08               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_COMPOSITE_INDEPENDENT_DOUBLE_VECTOR_H_
# define LBCPP_SAMPLER_COMPOSITE_INDEPENDENT_DOUBLE_VECTOR_H_

# include <lbcpp/Sampler/Sampler.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class IndependentDoubleVectorSampler : public CompositeSampler
{
public:
  IndependentDoubleVectorSampler(const EnumerationPtr& elementsEnumeration, const SamplerPtr& elementSamplerModel)
    : CompositeSampler(elementsEnumeration->getNumElements()), elementsEnumeration(elementsEnumeration)
    {initialize(elementSamplerModel);}

  IndependentDoubleVectorSampler(size_t numElements, const SamplerPtr& elementSamplerModel)
    : CompositeSampler(numElements), elementsEnumeration(positiveIntegerEnumerationEnumeration)
    {initialize(elementSamplerModel);}

  IndependentDoubleVectorSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(outputType, samplers.size());
    for (size_t i = 0; i < samplers.size(); ++i)
      res->setValue(i, samplers[i]->sample(context, random).toDouble());
    return res;
  }

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples) const
  {
    size_t n = samples->getNumElements();
    size_t numSamplers = samplers.size();
    subInputs.resize(numSamplers, inputs);
    subSamples.resize(numSamplers);
    for (size_t i = 0; i < numSamplers; ++i)
      subSamples[i] = new DenseDoubleVector(0, 0.0);

    for (size_t i = 0; i < n; ++i)
    {
      DenseDoubleVectorPtr vector = samples->getElement(i).getObjectAndCast<DenseDoubleVector>();
      jassert(vector->getNumElements() == numSamplers);
      for (size_t j = 0; j < numSamplers; ++j)
        subSamples[j].staticCast<DenseDoubleVector>()->appendValue(vector->getValue(j));
    }
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    CompositeSampler::clone(context, target);
    target.staticCast<IndependentDoubleVectorSampler>()->outputType = outputType;
  }

protected:
  friend class IndependentDoubleVectorSamplerClass;

  EnumerationPtr elementsEnumeration;
  ClassPtr outputType;

  void initialize(const SamplerPtr& elementSamplerModel)
  {
    for (size_t i = 0; i < samplers.size(); ++i)
      samplers[i] = elementSamplerModel->cloneAndCast<Sampler>();
    outputType = denseDoubleVectorClass(elementsEnumeration, doubleType);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_COMPOSITE_INDEPENDENT_DOUBLE_VECTOR_H_
