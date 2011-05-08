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
# include "../ProteinMover/RigidBodySpinMover.h"

namespace lbcpp
{

class RigidBodySpinMoverSampler;
typedef ReferenceCountedObjectPtr<RigidBodySpinMoverSampler> RigidBodySpinMoverSamplerPtr;

class RigidBodySpinMoverSampler: public CompositeSampler
{
public:
  RigidBodySpinMoverSampler() :
    CompositeSampler(3), numResidue(0)
  {
  }

  RigidBodySpinMoverSampler(size_t numResidue, double meanAmplitude, double stdAmplitude) :
    CompositeSampler(3), numResidue(numResidue)
  {
    // select first residue
    sons[0] = Variable(new EnumerationDiscreteSampler(numResidue, 0.01));
    // select second residue
    sons[1] = Variable(new EnumerationDiscreteSampler(numResidue, 0.01));
    // select amplitude
    sons[2] = Variable(new GaussianContinuousSampler(meanAmplitude, stdAmplitude));
  }

  RigidBodySpinMoverSampler(const RigidBodySpinMoverSampler& sampler) :
    CompositeSampler(3), numResidue(sampler.numResidue)
  {
    sons[0] = Variable(new EnumerationDiscreteSampler(*sampler.sons[0].getObjectAndCast<
        EnumerationDiscreteSampler> ()));
    sons[1] = Variable(new EnumerationDiscreteSampler(*sampler.sons[1].getObjectAndCast<
        EnumerationDiscreteSampler> ()));
    sons[2] = Variable(new GaussianContinuousSampler(*sampler.sons[2].getObjectAndCast<
        GaussianContinuousSampler> ()));
  }

  SamplerPtr clone()
  {
    RigidBodySpinMoverSamplerPtr temp = new RigidBodySpinMoverSampler(*this);
    return temp;
  }

  Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    size_t firstResidue = sons[0].getObjectAndCast<EnumerationDiscreteSampler> ()->sample(context,
        random, inputs).getInteger();
    // TODO rajouter la dependance entre le premier residu et le reste
    size_t secondResidue = sons[1].getObjectAndCast<EnumerationDiscreteSampler> ()->sample(context,
        random, inputs).getInteger();
    // residues must have at least one other residue between them
    if (std::abs((int)(firstResidue - secondResidue)) <= 1)
    {
      if (firstResidue <= secondResidue)
      {
        firstResidue = (size_t)juce::jlimit(0, (int)numResidue - 1, (int)firstResidue - 1);
        secondResidue = (size_t)juce::jlimit(0, (int)numResidue - 1, (int)secondResidue + 1);
      }
      else
      {
        firstResidue = (size_t)juce::jlimit(0, (int)numResidue - 1, (int)firstResidue + 1);
        secondResidue = (size_t)juce::jlimit(0, (int)numResidue - 1, (int)secondResidue - 1);
      }
    }

    double amplitude = sons[2].getObjectAndCast<GaussianContinuousSampler> ()->sample(context,
        random, inputs).getDouble();
    RigidBodySpinMoverPtr mover = new RigidBodySpinMover(firstResidue, secondResidue, amplitude);
    return Variable(mover);
  }

  /**
   * dataset = first : RigidBodySpinMoverPtr observed
   *           second : not yet used
   */
  void learn(ExecutionContext& context, const RandomGeneratorPtr& random, const std::vector<
      std::pair<Variable, Variable> >& dataset)
  {
    if (dataset.size() == 0)
      return;

    std::vector<std::pair<Variable, Variable> > datasetFirstResidue;
    std::vector<std::pair<Variable, Variable> > datasetSecondResidue;
    std::vector<std::pair<Variable, Variable> > datasetAmplitude;
    for (int i = 0; i < dataset.size(); i++)
    {
      RigidBodySpinMoverPtr mover = dataset[i].first.getObjectAndCast<RigidBodySpinMover> ();
      datasetFirstResidue.push_back(std::pair<Variable, Variable>(Variable(
          mover->getIndexResidueOne()), Variable()));
      datasetSecondResidue.push_back(std::pair<Variable, Variable>(Variable(
          mover->getIndexResidueTwo()), Variable()));
      datasetAmplitude.push_back(std::pair<Variable, Variable>(Variable(mover->getAmplitude()),
          Variable()));
    }
    sons[0].getObjectAndCast<EnumerationDiscreteSampler> ()->learn(context, random,
        datasetFirstResidue);
    sons[1].getObjectAndCast<EnumerationDiscreteSampler> ()->learn(context, random,
        datasetSecondResidue);
    sons[2].getObjectAndCast<GaussianContinuousSampler> ()->learn(context, random, datasetAmplitude);
  }

protected:
  friend class RigidBodySpinMoverSamplerClass;
  size_t numResidue;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_RIGID_BODY_SPIN_MOVER_SAMPLER_H_
