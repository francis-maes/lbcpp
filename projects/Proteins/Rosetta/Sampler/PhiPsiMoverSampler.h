/*-----------------------------------------.---------------------------------.
| Filename: PhiPsiMoverSampler.h           | PhiPsiMoverSampler              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 1 mai 2011  16:28:05           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PHI_PSI_MOVER_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_PHI_PSI_MOVER_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "../ProteinMover/PhiPsiMover.h"
# include "EnumerationDiscreteSampler.h"
# include "GaussianContinuousSampler.h"

namespace lbcpp
{

class PhiPsiMoverSampler;
typedef ReferenceCountedObjectPtr<PhiPsiMoverSampler> PhiPsiMoverSamplerPtr;

class PhiPsiMoverSampler: public CompositeSampler
{
public:
  PhiPsiMoverSampler() :
    CompositeSampler(3)
  {
  }

  PhiPsiMoverSampler(size_t numResidue, double meanPhi, double stdPhi, double meanPsi, double stdPsi) :
    CompositeSampler(3)
  {
    // select residue
    sons[0] = Variable(new EnumerationDiscreteSampler(numResidue, 0.01));
    //select phi
    sons[1] = Variable(new GaussianContinuousSampler(meanPhi, stdPhi));
    // select psi
    sons[2] = Variable(new GaussianContinuousSampler(meanPsi, stdPsi));
  }

  PhiPsiMoverSampler(const PhiPsiMoverSampler& sampler) :
    CompositeSampler(3)
  {
    sons[0] = new EnumerationDiscreteSampler(*sampler.sons[0].getObjectAndCast<
        EnumerationDiscreteSampler> ());
    sons[1] = new GaussianContinuousSampler(*sampler.sons[1].getObjectAndCast<
        GaussianContinuousSampler> ());
    sons[2] = new GaussianContinuousSampler(*sampler.sons[2].getObjectAndCast<
        GaussianContinuousSampler> ());
  }

  Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    size_t residue = sons[0].getObjectAndCast<EnumerationDiscreteSampler> ()->sample(context,
        random, inputs).getInteger();
    // TODO rajouter la dependance entre le residu et les angles
    double phi = sons[1].getObjectAndCast<GaussianContinuousSampler> ()->sample(context, random,
        inputs).getDouble();
    double psi = sons[2].getObjectAndCast<GaussianContinuousSampler> ()->sample(context, random,
        inputs).getDouble();
    PhiPsiMoverPtr mover = new PhiPsiMover(residue, phi, psi);
    return Variable(mover);
  }

  /**
   * dataset = first : PhiPsiMoverPtr observed
   *           second : not yet used
   */
  void learn(ExecutionContext& context, const RandomGeneratorPtr& random, const std::vector<
      std::pair<Variable, Variable> >& dataset)
  {
    std::vector<std::pair<Variable, Variable> > datasetResidue;
    std::vector<std::pair<Variable, Variable> > datasetPhi;
    std::vector<std::pair<Variable, Variable> > datasetPsi;
    for (int i = 0; i < dataset.size(); i++)
    {
      PhiPsiMoverPtr mover = dataset[i].first.getObjectAndCast<PhiPsiMover> ();
      datasetResidue.push_back(std::pair<Variable, Variable>(Variable(mover->getResidueIndex()),
          Variable()));
      datasetPhi.push_back(
          std::pair<Variable, Variable>(Variable(mover->getDeltaPhi()), Variable()));
      datasetPsi.push_back(
          std::pair<Variable, Variable>(Variable(mover->getDeltaPsi()), Variable()));
    }
    sons[0].getObjectAndCast<EnumerationDiscreteSampler> ()->learn(context, random, datasetResidue);
    sons[1].getObjectAndCast<GaussianContinuousSampler> ()->learn(context, random, datasetPhi);
    sons[2].getObjectAndCast<GaussianContinuousSampler> ()->learn(context, random, datasetPsi);
  }

protected:
  friend class PhiPsiMoverSamplerClass;
};

}; /* namespace lbcpp */



#endif //! LBCPP_PROTEINS_ROSETTA_PHI_PSI_MOVER_SAMPLER_H_
