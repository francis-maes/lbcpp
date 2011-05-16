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

class ProteinMoverSampler;
typedef ReferenceCountedObjectPtr<ProteinMoverSampler> ProteinMoverSamplerPtr;

class ProteinMoverSampler : public CompositeSampler
{
public:
  ProteinMoverSampler(DiscreteSamplerPtr classSampler, size_t numResidues)
    : classSampler(classSampler) {createObjectSamplers(numResidues);}

  ProteinMoverSampler(size_t numResidues)
  {
    createObjectSamplers(numResidues);
    classSampler = enumerationSampler(proteinMoverEnumerationEnumeration);
  }

  ProteinMoverSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    size_t index = (size_t)classSampler->sample(context, random, inputs).getInteger();
    jassert(index < samplers.size());
    return samplers[index]->sample(context, random, inputs);
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    std::vector<Variable> classSamplerDataset(dataset.size());
    std::vector< std::vector<Variable> > subDatasets(samplers.size());

    double invZ = 1.0 / dataset.size();
    for (size_t i = 0; i < dataset.size(); ++i)
    {
      const Variable& example = dataset[i];
      TypePtr type;
      bool isConditional = false;
      Variable input;
      if (example.dynamicCast<Pair>())
      {
        const PairPtr& pair = example.getObjectAndCast<Pair>();
        input = pair->getFirst();
        objectClass = pair->getSecond().getType();
        isConditional = true;
      }
      else
        objectClass = example.getType();

      size_t target;
      if (type == phiPsiMoverClass)
        target = 0;
      else if (type == shearMoverClass)
        target = 1;
      else if (type == rigidBodyMoverClass)
        target = 2;
      else
        jassert(false);
      
      Variable targetVariable(target, proteinMoverEnumerationEnumeration);
      classSamplerDataset.push_back(isConditional ? Variable(new Pair(input, targetVariable)) : targetVariable);
      subDatasets[target].push_back(dataset[i]);
    }

    for (size_t i = 0; i < samplers.size(); ++i)
      samplers[i]->learn(context, subDatasets[i]);
  }

protected:
  friend class ProteinMoverSamplerClass;

  DiscreteSamplerPtr classSampler;

  void createObjectSamplers(size_t numResidues)
  {
    samplers.push_back(objectCompositeSampler(phiPsiMoverClass, new SimpleResidueSampler(numResidues), gaussianSampler(0, M_PI), gaussianSampler(0, M_PI)));
    samplers.push_back(objectCompositeSampler(shearMoverClass, new SimpleResidueSampler(numResidues), gaussianSampler(0, M_PI), gaussianSampler(0, M_PI)));
    samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, new ResiduePairSampler(numResidues), gaussianSampler(1, 1), gaussianSampler(0, M_PI)));
  }
};

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_PROTEIN_MOVER_SAMPLER_H_
