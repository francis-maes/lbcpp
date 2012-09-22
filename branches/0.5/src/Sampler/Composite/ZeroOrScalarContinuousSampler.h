/*-----------------------------------------.---------------------------------.
| Filename: ZeroOrScalarContinuousSampler.h| Either samples 0 or sample a    |
| Author  : Francis Maes                   |  scalar value                   |
| Started : 17/05/2011 19:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_COMPOSITE_ZERO_OR_SCALAR_CONTINUOUS_H_
# define LBCPP_SAMPLER_COMPOSITE_ZERO_OR_SCALAR_CONTINUOUS_H_

# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class ZeroOrScalarContinuousSampler : public CompositeSampler
{
public:
  ZeroOrScalarContinuousSampler(const DiscreteSamplerPtr& equalZeroSampler, const ScalarContinuousSamplerPtr& scalarSampler)
    : CompositeSampler(2) {samplers[0] = equalZeroSampler; samplers[1] = scalarSampler;}
  ZeroOrScalarContinuousSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    bool equalZero = samplers[0]->sample(context, random, inputs).getBoolean();
    return equalZero ? 0.0 : samplers[1]->sample(context, random, inputs).getDouble();
  }

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
  {
    size_t n = samples->getNumElements();

    subInputs[0] = inputs;
    subSamples[0] = vector(booleanType, n);

    VectorPtr subInputsVector;
    if (inputs)
      subInputsVector = vector(inputs->getElementsType());
    subInputs[1] = subInputsVector;
    DenseDoubleVectorPtr subSamplesVector = new DenseDoubleVector(0, 0.0);
    subSamples[1] = subSamplesVector;
    for (size_t i = 0; i < n; ++i)
    {
      double sample = samples->getElement(i).getDouble();
      bool equalZero = (sample == 0.0);
      subSamples[0]->setElement(i, equalZero);
      if (!equalZero)
      {
        if (subInputsVector)
          subInputsVector->append(inputs->getElement(i));
        subSamplesVector->appendValue(sample);
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_COMPOSITE_ZERO_OR_SCALAR_CONTINUOUS_H_
