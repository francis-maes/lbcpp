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
# include "DualResidueSampler.h"

namespace lbcpp
{

class RigidBodyTransMoverSampler;
typedef ReferenceCountedObjectPtr<RigidBodyTransMoverSampler> RigidBodyTransMoverSamplerPtr;

class RigidBodyTransMoverSampler : public CompositeSampler
{
public:
  RigidBodyTransMoverSampler()
    : CompositeSampler(2), numResidue(0)
  {
  }

  RigidBodyTransMoverSampler(size_t numResidue, double meanMagnitude, double stdMagnitude)
    : CompositeSampler(2), numResidue(numResidue)
  {
    // select residue
    samplers[0] = new DualResidueSampler(numResidue, 2);
    // select magnitude
    samplers[1] = gaussianSampler(meanMagnitude, stdMagnitude);
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    MatrixPtr residues = samplers[0]->sample(context, random, inputs).getObjectAndCast<Matrix> ();
    size_t firstResidue = (size_t)(residues->getElement(0, 0).getDouble());
    size_t secondResidue = (size_t)(residues->getElement(1, 0).getDouble());

    double magnitude = samplers[1]->sample(context, random, inputs).getDouble();
    RigidBodyTransMoverPtr mover = new RigidBodyTransMover(firstResidue, secondResidue, magnitude);
    return Variable(mover);
  }

  /**
   * dataset = first : RigidBodyTransMoverPtr observed
   *           second : not yet used
   */
  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    if (dataset.size() < 1)
      return;

    std::vector<Variable> datasetResidues;
    std::vector<Variable> datasetMagnitude;
    for (size_t i = 0; i < dataset.size(); i++)
    {
      RigidBodyTransMoverPtr mover = dataset[i].getObjectAndCast<RigidBodyTransMover> ();
      MatrixPtr tempResidue = new DoubleMatrix(2, 1);
      tempResidue->setElement(0, 0, Variable((double)mover->getIndexResidueOne()));
      tempResidue->setElement(1, 0, Variable((double)mover->getIndexResidueTwo()));
      datasetResidues.push_back(tempResidue);
      datasetMagnitude.push_back(mover->getMagnitude());
    }

    samplers[0]->learn(context, datasetResidues);
    samplers[1]->learn(context, datasetMagnitude);
  }

protected:
  friend class RigidBodyTransMoverSamplerClass;
  size_t numResidue;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_RIGID_BODY_TRANS_MOVER_SAMPLER_H_
