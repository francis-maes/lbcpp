/*-----------------------------------------.---------------------------------.
| Filename: EnumerationSampler.h           | Enumeration Sampler             |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 30 avr. 2011  11:11:01         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_DISCRETE_ENUMERATION_H_
# define LBCPP_SAMPLER_DISCRETE_ENUMERATION_H_

# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class EnumerationSampler : public DiscreteSampler
{
public:
  EnumerationSampler(EnumerationPtr enumeration)
    : probabilities(new DenseDoubleVector(enumeration, probabilityType)) {}
  EnumerationSampler(const DenseDoubleVectorPtr& probabilities)
    : probabilities(probabilities) {}
  EnumerationSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
    {return random->sampleWithNormalizedProbabilities(probabilities->getValues());}

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    jassert(dataset.size());

    // Compute empirical frequencies
    DenseDoubleVectorPtr empiricalFrequencies = new DenseDoubleVector(
        probabilities->getElementsEnumeration(), probabilities->getElementsType(),
        probabilities->getNumElements(), 0.0);
    double increment = 1.0 / (double)dataset.size();
    for (size_t i = 0; i < dataset.size(); i++)
    {
      size_t index = dataset[i].getInteger();
      empiricalFrequencies->setValue(index, empiricalFrequencies->getValue(index) + increment);
    }
  }

protected:
  friend class EnumerationSamplerClass;

  DenseDoubleVectorPtr probabilities;
};

}; /* namespace lbcpp */


#endif //! LBCPP_PROTEINS_ROSETTA_ENUMERATION_DISCRETE_SAMPLER_H_
