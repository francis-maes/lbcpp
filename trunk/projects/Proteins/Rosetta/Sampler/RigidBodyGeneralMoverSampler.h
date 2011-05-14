/*-----------------------------------------.---------------------------------.
| Filename: RigidBodyGeneralMoverSampler.h | RigidBodyGeneralMoverSampler    |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 1 mai 2011  16:29:04           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_RIGID_BODY_GENERAL_MOVER_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_RIGID_BODY_GENERAL_MOVER_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "../ProteinMover/RigidBodyMover.h"
# include "DualResidueSampler.h"

namespace lbcpp
{

class RigidBodyGeneralMoverSampler;
typedef ReferenceCountedObjectPtr<RigidBodyGeneralMoverSampler> RigidBodyGeneralMoverSamplerPtr;

class RigidBodyGeneralMoverSampler : public CompositeSampler
{
public:
  RigidBodyGeneralMoverSampler(size_t numResidue, double meanMagnitude, double stdMagnitude,
      double meanAmplitude, double stdAmplitude)
    : CompositeSampler(3), numResidue(numResidue)
  {
    // select residue
    samplers[0] = new DualResidueSampler(numResidue);
    // select magnitude
    samplers[1] = gaussianSampler(meanMagnitude, stdMagnitude);
    // select amplitude
    samplers[2] = gaussianSampler(meanAmplitude, stdAmplitude);
  }
  RigidBodyGeneralMoverSampler() : numResidue(0) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    PairPtr residues = samplers[0]->sample(context, random, inputs).getObjectAndCast<Pair>();
    size_t firstResidue = (size_t)residues->getFirst().getInteger();
    size_t secondResidue = (size_t)residues->getSecond().getInteger();

    double magnitude = samplers[1]->sample(context, random, inputs).getDouble();
    double amplitude = samplers[2]->sample(context, random, inputs).getDouble();
    return new RigidBodyMover(firstResidue, secondResidue, magnitude, amplitude);
  }

  /**
   * dataset = first : RigidBodyGeneralMoverPtr observed
   */
  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    if (dataset.size() < 2)
      return;

    std::vector<Variable> datasetResidues;
    std::vector<Variable> datasetMagnitude;
    std::vector<Variable> datasetAmplitude;
    for (size_t i = 0; i < dataset.size(); i++)
    {
      RigidBodyMoverPtr mover = dataset[i].getObjectAndCast<RigidBodyMover> ();
      PairPtr tempResidue = new Pair((size_t)mover->getIndexResidueOne(),
          (size_t)mover->getIndexResidueTwo());
      datasetResidues.push_back(tempResidue);
      datasetMagnitude.push_back(mover->getMagnitude());
      datasetAmplitude.push_back(mover->getAmplitude());
    }
    samplers[0]->learn(context, datasetResidues);
    samplers[1]->learn(context, datasetMagnitude);
    samplers[2]->learn(context, datasetAmplitude);
  }

protected:
  friend class RigidBodyGeneralMoverSamplerClass;
  size_t numResidue;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_RIGID_BODY_GENERAL_MOVER_SAMPLER_H_
