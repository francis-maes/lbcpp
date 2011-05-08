/*-----------------------------------------.---------------------------------.
| Filename: ShearMoverSampler.h            | ShearMoverSampler               |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 1 mai 2011  16:28:24           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_SHEAR_MOVER_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_SHEAR_MOVER_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "../ProteinMover/ShearMover.h"
# include "SimpleResidueSampler.h"
# include "GaussianContinuousSampler.h"

namespace lbcpp
{

class ShearMoverSampler;
typedef ReferenceCountedObjectPtr<ShearMoverSampler> ShearMoverSamplerPtr;

class ShearMoverSampler: public CompositeSampler
{
public:
  ShearMoverSampler() :
    CompositeSampler(3)
  {
  }

  ShearMoverSampler(size_t numResidue, double meanPhi, double stdPhi, double meanPsi, double stdPsi) :
    CompositeSampler(3)
  {
    // select residue
    sons[0] = Variable(new SimpleResidueSampler(numResidue, (int)0.05 * numResidue));
    //select phi
    sons[1] = Variable(new GaussianContinuousSampler(meanPhi, stdPhi));
    // select psi
    sons[2] = Variable(new GaussianContinuousSampler(meanPsi, stdPsi));
  }

  ShearMoverSampler(const ShearMoverSampler& sampler) :
    CompositeSampler(3)
  {
    sons[0] = Variable(new SimpleResidueSampler(*sampler.sons[0].getObjectAndCast<
        SimpleResidueSampler> ()));
    sons[1] = Variable(new GaussianContinuousSampler(*sampler.sons[1].getObjectAndCast<
        GaussianContinuousSampler> ()));
    sons[2] = Variable(new GaussianContinuousSampler(*sampler.sons[2].getObjectAndCast<
        GaussianContinuousSampler> ()));
  }

  SamplerPtr clone()
  {
    ShearMoverSamplerPtr temp = new ShearMoverSampler(*this);
    return temp;
  }

  Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    size_t residue = sons[0].getObjectAndCast<SimpleResidueSampler> ()->sample(context, random,
        inputs).getInteger();
    // TODO rajouter la dependance entre le residu et les angles
    double phi = sons[1].getObjectAndCast<GaussianContinuousSampler> ()->sample(context, random,
        inputs).getDouble();
    double psi = sons[2].getObjectAndCast<GaussianContinuousSampler> ()->sample(context, random,
        inputs).getDouble();
    ShearMoverPtr mover = new ShearMover(residue, phi, psi);
    return Variable(mover);
  }

  /**
   * dataset = first : ShearMoverPtr observed
   *           second : not yet used
   */
  void learn(ExecutionContext& context, const RandomGeneratorPtr& random, const std::vector<
      std::pair<Variable, Variable> >& dataset)
  {
    if (dataset.size() == 0)
      return;

    std::vector<std::pair<Variable, Variable> > datasetResidue;
    std::vector<std::pair<Variable, Variable> > datasetPhi;
    std::vector<std::pair<Variable, Variable> > datasetPsi;
    for (int i = 0; i < dataset.size(); i++)
    {
      ShearMoverPtr mover = dataset[i].first.getObjectAndCast<ShearMover> ();
      datasetResidue.push_back(std::pair<Variable, Variable>(Variable(mover->getResidueIndex()),
          Variable()));
      datasetPhi.push_back(
          std::pair<Variable, Variable>(Variable(mover->getDeltaPhi()), Variable()));
      datasetPsi.push_back(
          std::pair<Variable, Variable>(Variable(mover->getDeltaPsi()), Variable()));
    }
    sons[0].getObjectAndCast<SimpleResidueSampler> ()->learn(context, random, datasetResidue);
    sons[1].getObjectAndCast<GaussianContinuousSampler> ()->learn(context, random, datasetPhi);
    sons[2].getObjectAndCast<GaussianContinuousSampler> ()->learn(context, random, datasetPsi);
  }

protected:
  friend class ShearMoverSamplerClass;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_SHEAR_MOVER_SAMPLER_H_
