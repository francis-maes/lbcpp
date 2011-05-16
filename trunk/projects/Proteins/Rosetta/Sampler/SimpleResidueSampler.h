/*-----------------------------------------.---------------------------------.
| Filename: SimpleResidueSampler.h         | SimpleResidueSampler            |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 7 mai 2011  15:04:24           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_SIMPLE_RESIDUE_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_SIMPLE_RESIDUE_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "ParzenContinuousSampler.h"

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
//    ContinuousSamplerPtr temp = new ParzenContinuousSampler(0.0001,
//        (double)(-1 * numResidues), (double)(2 * numResidues), 1.0 / 2.0, 0.5 * numResidues, 0.25
//            * numResidues);
    ContinuousSamplerPtr temp = gaussianSampler(0.5 * numResidues, 0.25 * numResidues);
    samplers[0] = discretizeSampler(temp, 0, (int)(numResidues - 1));
  }

  SimpleResidueSampler() : CompositeSampler(1) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
    {return samplers[0]->sample(context, random, inputs);}

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples) const
  {
    subInputs.resize(1, inputs);
    subSamples.resize(1, samples);
  }

protected:
  friend class SimpleResidueSamplerClass;

  size_t numResidues;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_SIMPLE_RESIDUE_SAMPLER_H_
