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
# include "DualResidueSampler.h"

namespace lbcpp
{

class RigidBodySpinMoverSampler;
typedef ReferenceCountedObjectPtr<RigidBodySpinMoverSampler> RigidBodySpinMoverSamplerPtr;

class RigidBodySpinMoverSampler: public CompositeSampler
{
public:
  RigidBodySpinMoverSampler() :
    CompositeSampler(2), numResidue(0)
  {
  }

  RigidBodySpinMoverSampler(size_t numResidue, double meanAmplitude, double stdAmplitude) :
    CompositeSampler(2), numResidue(numResidue)
  {
    // select residue
    sons[0] = Variable(new DualResidueSampler(numResidue, 2));
    // select amplitude
    sons[1] = Variable(new GaussianContinuousSampler(meanAmplitude, stdAmplitude));
  }

  RigidBodySpinMoverSampler(const RigidBodySpinMoverSampler& sampler) :
    CompositeSampler(2), numResidue(sampler.numResidue)
  {
    sons[0] = Variable(new DualResidueSampler(
        *sampler.sons[0].getObjectAndCast<DualResidueSampler> ()));
    sons[1] = Variable(new GaussianContinuousSampler(*sampler.sons[1].getObjectAndCast<
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
    MatrixPtr residues = sons[0].getObjectAndCast<DualResidueSampler> ()->sample(context, random,
        inputs).getObjectAndCast<Matrix> ();
    size_t firstResidue = (int)(residues->getElement(0, 0).getDouble());
    size_t secondResidue = (int)(residues->getElement(1, 0).getDouble());

    double amplitude = sons[1].getObjectAndCast<GaussianContinuousSampler> ()->sample(context,
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
    if (dataset.size() < 1)
      return;

    std::vector<std::pair<Variable, Variable> > datasetResidues;
    std::vector<std::pair<Variable, Variable> > datasetAmplitude;
    for (int i = 0; i < dataset.size(); i++)
    {
      RigidBodySpinMoverPtr mover = dataset[i].first.getObjectAndCast<RigidBodySpinMover> ();
      MatrixPtr tempResidue = new DoubleMatrix(2, 1);
      tempResidue->setElement(0, 0, Variable((double)mover->getIndexResidueOne()));
      tempResidue->setElement(1, 0, Variable((double)mover->getIndexResidueTwo()));
      datasetResidues.push_back(std::pair<Variable, Variable>(Variable(tempResidue), Variable()));
      datasetAmplitude.push_back(std::pair<Variable, Variable>(Variable(mover->getAmplitude()),
          Variable()));
    }
    sons[0].getObjectAndCast<DualResidueSampler> ()->learn(context, random, datasetResidues);
    sons[1].getObjectAndCast<GaussianContinuousSampler> ()->learn(context, random, datasetAmplitude);
  }

protected:
  friend class RigidBodySpinMoverSamplerClass;
  size_t numResidue;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_RIGID_BODY_SPIN_MOVER_SAMPLER_H_
