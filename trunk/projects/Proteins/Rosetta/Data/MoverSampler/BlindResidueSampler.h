/*-----------------------------------------.---------------------------------.
| Filename: BlindResidueSampler.h          | Blind Residue Sampler           |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 8, 2012  4:47:08 PM        |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_BLINDRESIDUESAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_BLINDRESIDUESAMPLER_H_

# include "precompiled.h"
# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class BlindResidueSampler : public CompositeSampler
{
public:
  BlindResidueSampler() : numResidues(0) {}
  BlindResidueSampler(size_t numResidues)
    : numResidues(numResidues)
  {
    DenseDoubleVectorPtr probas = new DenseDoubleVector(numResidues, 1.0 / numResidues);
    samplers.push_back(enumerationSampler(probas));
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
    {return samplers[0]->sample(context, random).getInteger();}

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& supervisionWeights = DenseDoubleVectorPtr())
    {}

  virtual DenseDoubleVectorPtr computeLogProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
  {
    DenseDoubleVectorPtr probabilities = new DenseDoubleVector(samples->getNumElements(), std::log(1.0 / numResidues));
    return probabilities;
  }

protected:
  friend class BlindResidueSamplerClass;

  size_t numResidues;
};

typedef ReferenceCountedObjectPtr<BlindResidueSampler> BlindResidueSamplerPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_BLINDRESIDUESAMPLER_H_
