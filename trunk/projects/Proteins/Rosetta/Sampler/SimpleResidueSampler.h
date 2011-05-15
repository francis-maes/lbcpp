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
# include "DiscretizeSampler.h"
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
    DiscretizeSamplerPtr samp = new DiscretizeSampler(temp, 0, (int)(numResidues - 1));
    samplers[0] = samp;
  }

  SimpleResidueSampler() : CompositeSampler(1) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {return samplers[0]->sample(context, random, inputs);}

  /**
   * dataset = first : a Variable of integer type containing the residue observed.
   */
  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    jassert((dataset.size() > 0) && (numResidues > 0));
    samplers[0]->learn(context, dataset);
  }

protected:
  friend class SimpleResidueSamplerClass;

  size_t numResidues;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_SIMPLE_RESIDUE_SAMPLER_H_
