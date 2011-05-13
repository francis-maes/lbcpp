/*-----------------------------------------.---------------------------------.
| Filename: IndependentDenseDoubleVecto...h| Samples vectors in R^n          |
| Author  : Francis Maes                   |                                 |
| Started : 13/05/2011 19:08               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_COMPOSITE_INDEPENDENT_DENSE_DOUBLE_VECTOR_H_
# define LBCPP_SAMPLER_COMPOSITE_INDEPENDENT_DENSE_DOUBLE_VECTOR_H_

# include <lbcpp/Sampler/Sampler.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class IndependentDenseDoubleVectorSampler : public CompositeSampler
{
public:
  IndependentDenseDoubleVectorSampler(const EnumerationPtr& elementsEnumeration, const SamplerPtr& elementSamplerModel)
    : CompositeSampler(elementsEnumeration->getNumElements()), elementsEnumeration(elementsEnumeration)
    {createSamplers(elementSamplerModel);}

  IndependentDenseDoubleVectorSampler(size_t numElements, const SamplerPtr& elementSamplerModel)
    : CompositeSampler(numElements), elementsEnumeration(positiveIntegerEnumerationEnumeration)
    {createSamplers(elementSamplerModel);}

  IndependentDenseDoubleVectorSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(elementsEnumeration, doubleType, samplers.size());
    for (size_t i = 0; i < samplers.size(); ++i)
      res->setValue(i, samplers[i]->sample(context, random).toDouble());
    return res;
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    size_t n = samplers.size();

    std::vector< std::vector<Variable> > subDatasets(n);
    for (size_t i = 0; i < n; ++i)
      subDatasets[i].resize(dataset.size());

    for (size_t i = 0; i < dataset.size(); ++i)
    {
      const DenseDoubleVectorPtr& vector = dataset[i].getObjectAndCast<DenseDoubleVector>();
      jassert(vector->getNumElements() == samplers.size());
      for (size_t j = 0; j < n; ++j)
        subDatasets[j][i] = vector->getValue(j);
    }

    for (size_t i = 0; i < n; ++i)
      samplers[i]->learn(context, subDatasets[i]);
  }

protected:
  friend class IndependentDenseDoubleVectorSamplerClass;

  EnumerationPtr elementsEnumeration;

  void createSamplers(const SamplerPtr& elementSamplerModel)
  {
    for (size_t i = 0; i < samplers.size(); ++i)
      samplers[i] = elementSamplerModel->cloneAndCast<Sampler>();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_COMPOSITE_INDEPENDENT_DENSE_DOUBLE_VECTOR_H_
