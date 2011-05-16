/*-----------------------------------------.---------------------------------.
| Filename: ProteinMoverSampler.h          | ProteinMoverSampler             |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 16 mai 2011  07:06:13          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_PROTEIN_MOVER_SAMPLER_H_
# define LBCPP_SAMPLER_PROTEIN_MOVER_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "SimpleResidueSampler.h"
# include "ResiduePairSampler.h"
# include "../ProteinMover.h"

namespace lbcpp
{
class ProteinMoverSampler : public CompositeSampler
{
public:
  ProteinMoverSampler(size_t numResidues)
  {
    samplers.push_back(objectCompositeSampler(phiPsiMoverClass, new SimpleResidueSampler(numResidues), gaussianSampler(0, M_PI), gaussianSampler(0, M_PI)));
    samplers.push_back(objectCompositeSampler(shearMoverClass, new SimpleResidueSampler(numResidues), gaussianSampler(0, M_PI), gaussianSampler(0, M_PI)));
    samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, new ResiduePairSampler(numResidues), gaussianSampler(1, 1), gaussianSampler(0, M_PI)));
    probabilities = new DenseDoubleVector(3, 0.33);
  }

  ProteinMoverSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    jassert(probabilities->getNumValues() == samplers.size());
    size_t index = random->sampleWithProbabilities(probabilities->getValues());
    return samplers[index]->sample(context, random, inputs);
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    std::vector< std::vector<Variable> > subDatasets(samplers.size());
    probabilities->clear();
    probabilities->resize(samplers.size());

    double invZ = 1.0 / dataset.size();
    for (size_t i = 0; i < dataset.size(); ++i)
    {
      TypePtr type = dataset[i].getType();
      size_t target;
      if (type == phiPsiMoverClass)
        target = 0;
      else if (type == shearMoverClass)
        target = 1;
      else if (type == rigidBodyMoverClass)
        target = 2;
      else
        jassert(false);
      probabilities->incrementValue(target, invZ);
      subDatasets[target].push_back(dataset[i]);
    }

    for (size_t i = 0; i < samplers.size(); ++i)
      samplers[i]->learn(context, subDatasets[i]);
  }

protected:
  friend class ProteinMoverSamplerClass;

  DenseDoubleVectorPtr probabilities;
};

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_PROTEIN_MOVER_SAMPLER_H_
