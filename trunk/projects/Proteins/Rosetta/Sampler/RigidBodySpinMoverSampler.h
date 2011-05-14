/*-----------------------------------------.---------------------------------.
| Filename: RigidBodySpinMoverSampler.h    | RigidBodySpinMoverSampler       |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 1 mai 2011  16:29:21           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_RIGID_BODY_SPIN_MOVER_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_RIGID_BODY_SPIN_MOVER_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "DualResidueSampler.h"

namespace lbcpp
{

class RigidBodySpinMoverSampler;
typedef ReferenceCountedObjectPtr<RigidBodySpinMoverSampler> RigidBodySpinMoverSamplerPtr;

class RigidBodySpinMoverSampler : public CompositeSampler
{
public:

  RigidBodySpinMoverSampler(size_t numResidue, double meanAmplitude, double stdAmplitude)
    : CompositeSampler(2), numResidue(numResidue)
  {
    // select residue
    samplers[0] = new DualResidueSampler(numResidue);
    // select amplitude
    samplers[1] = gaussianSampler(meanAmplitude, stdAmplitude);
  }

  RigidBodySpinMoverSampler() : numResidue(0) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    PairPtr residues = samplers[0]->sample(context, random, inputs).getObjectAndCast<Pair>();
    size_t firstResidue = (size_t)residues->getFirst().getInteger();
    size_t secondResidue = (size_t)residues->getSecond().getInteger();

    double amplitude = samplers[1]->sample(context, random, inputs).getDouble();
    RigidBodyMoverPtr mover = new RigidBodyMover(firstResidue, secondResidue, 0.0, amplitude);
    return Variable(mover);
  }

  /**
   * dataset = first : RigidBodySpinMoverPtr observed
   */
  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    if (dataset.size() < 1)
      return;

    std::vector<Variable> datasetResidues;
    std::vector<Variable> datasetAmplitude;
    for (size_t i = 0; i < dataset.size(); i++)
    {
      RigidBodyMoverPtr mover = dataset[i].getObjectAndCast<RigidBodyMover> ();
      PairPtr tempResidue = new Pair((size_t)mover->getIndexResidueOne(),
          (size_t)mover->getIndexResidueTwo());
      datasetResidues.push_back(tempResidue);
      datasetAmplitude.push_back(mover->getAmplitude());
    }
    samplers[0]->learn(context, datasetResidues);
    samplers[1]->learn(context, datasetAmplitude);
  }

protected:
  friend class RigidBodySpinMoverSamplerClass;
  size_t numResidue;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_RIGID_BODY_SPIN_MOVER_SAMPLER_H_
