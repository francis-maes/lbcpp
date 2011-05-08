/*-----------------------------------------.---------------------------------.
| Filename: RigidBodyTransMoverSampler.h   | RigidBodyTransMoverSampler      |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 1 mai 2011  16:28:43           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_RIGID_BODY_TRANS_MOVER_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_RIGID_BODY_TRANS_MOVER_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "../ProteinMover/RigidBodyTransMover.h"

namespace lbcpp
{

class RigidBodyTransMoverSampler;
typedef ReferenceCountedObjectPtr<RigidBodyTransMoverSampler> RigidBodyTransMoverSamplerPtr;

class RigidBodyTransMoverSampler: public CompositeSampler
{
public:
  RigidBodyTransMoverSampler() :
    CompositeSampler(3), numResidue(0)
  {
  }

  RigidBodyTransMoverSampler(size_t numResidue, double meanMagnitude, double stdMagnitude) :
    CompositeSampler(3), numResidue(numResidue)
  {
    // select first residue
    sons[0] = Variable(new EnumerationDiscreteSampler(numResidue, 0.01));
    // select second residue
    sons[1] = Variable(new EnumerationDiscreteSampler(numResidue, 0.01));
    // select magnitude
    sons[2] = Variable(new GaussianContinuousSampler(meanMagnitude, stdMagnitude));
  }

  RigidBodyTransMoverSampler(const RigidBodyTransMoverSampler& sampler) :
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
    RigidBodyTransMoverSamplerPtr temp = new RigidBodyTransMoverSampler(*this);
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

    double magnitude = sons[2].getObjectAndCast<GaussianContinuousSampler> ()->sample(context,
        random, inputs).getDouble();
    RigidBodyTransMoverPtr mover = new RigidBodyTransMover(firstResidue, secondResidue, magnitude);
    return Variable(mover);
  }

  /**
   * dataset = first : RigidBodyTransMoverPtr observed
   *           second : not yet used
   */
  void learn(ExecutionContext& context, const RandomGeneratorPtr& random, const std::vector<
      std::pair<Variable, Variable> >& dataset)
  {
    if (dataset.size() == 0)
      return;

    std::vector<std::pair<Variable, Variable> > datasetFirstResidue;
    std::vector<std::pair<Variable, Variable> > datasetSecondResidue;
    std::vector<std::pair<Variable, Variable> > datasetMagnitude;
    for (int i = 0; i < dataset.size(); i++)
    {
      RigidBodyTransMoverPtr mover = dataset[i].first.getObjectAndCast<RigidBodyTransMover> ();
      datasetFirstResidue.push_back(std::pair<Variable, Variable>(Variable(
          mover->getIndexResidueOne()), Variable()));
      datasetSecondResidue.push_back(std::pair<Variable, Variable>(Variable(
          mover->getIndexResidueTwo()), Variable()));
      datasetMagnitude.push_back(std::pair<Variable, Variable>(Variable(mover->getMagnitude()),
          Variable()));
    }
    sons[0].getObjectAndCast<EnumerationDiscreteSampler> ()->learn(context, random,
        datasetFirstResidue);
    sons[1].getObjectAndCast<EnumerationDiscreteSampler> ()->learn(context, random,
        datasetSecondResidue);
    sons[2].getObjectAndCast<GaussianContinuousSampler> ()->learn(context, random, datasetMagnitude);
  }

protected:
  friend class RigidBodyTransMoverSamplerClass;
  size_t numResidue;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_RIGID_BODY_TRANS_MOVER_SAMPLER_H_
