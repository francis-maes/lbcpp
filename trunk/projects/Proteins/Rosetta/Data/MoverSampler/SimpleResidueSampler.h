/*-----------------------------------------.---------------------------------.
| Filename: SimpleResidueSampler.h         | SimpleResidueSampler            |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 7 mai 2011  15:04:24           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_SIMPLERESIDUESAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_SIMPLERESIDUESAMPLER_H_

# include "precompiled.h"
# include "../PoseMoverSampler.h"

namespace lbcpp
{

class SimpleResidueSampler;
typedef ReferenceCountedObjectPtr<SimpleResidueSampler> SimpleResidueSamplerPtr;

class SimpleResidueSampler : public CompositeSampler
{
public:
  SimpleResidueSampler(size_t numResidues)
    : CompositeSampler(1), numResidues(numResidues)
  {
    ContinuousSamplerPtr temp = gaussianSampler(0.5 * numResidues, 0.25 * numResidues);
    samplers[0] = discretizeSampler(temp, 0, (int)(numResidues - 1));
  }

  SimpleResidueSampler(const DiscreteSamplerPtr& residueSampler, size_t numResidues)
      : CompositeSampler(1), numResidues(numResidues)
  {
    samplers[0] = residueSampler;
  }

  SimpleResidueSampler() : CompositeSampler(1) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
    {return samplers[0]->sample(context, random, inputs);}

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
  {
    subInputs.resize(1);
    subInputs[0] = inputs;
    subSamples.resize(1);
    subSamples[0] = samples;
  }

protected:
  friend class SimpleResidueSamplerClass;

  size_t numResidues;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_SIMPLERESIDUESAMPLER_H_
