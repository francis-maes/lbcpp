/*-----------------------------------------.---------------------------------.
| Filename: ProteinMoverSampler.h          | ProteinMoverSampler             |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 30 avr. 2011  11:11:37         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PROTEIN_MOVER_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_PROTEIN_MOVER_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "EnumerationDiscreteSampler.h"
# include "PhiPsiMoverSampler.h"
# include "ShearMoverSampler.h"
# include "RigidBodyGeneralMoverSampler.h"
# include "RigidBodyTransMoverSampler.h"
# include "RigidBodySpinMoverSampler.h"
# include "../ProteinMover.h"
# include "../ProteinMover/PhiPsiMover.h"
# include "../ProteinMover/RigidBodySpinMover.h"
# include "../ProteinMover/ShearMover.h"
# include "../ProteinMover/RigidBodyGeneralMover.h"
# include "../ProteinMover/RigidBodyTransMover.h"

namespace lbcpp
{

enum ProteinMoverEnumeration
{
  phipsi = 0, shear, rigidbodytrans, rigidbodyspin, rigidbodygeneral,
};

extern EnumerationPtr proteinMoverEnumerationEnumeration;

class ProteinMoverSampler;
typedef ReferenceCountedObjectPtr<ProteinMoverSampler> ProteinMoverSamplerPtr;

class ProteinMoverSampler: public CompositeSampler
{
public:
  ProteinMoverSampler() :
    CompositeSampler(0), numMover(0)
  {
  }

  /**
   * Each variable of samplers is a Ptr to a sampler. Do not deallocate
   * a sampler before deallocating ProteinMoverSampler.
   */
  ProteinMoverSampler(size_t numMover, std::vector<Variable>& samplers) :
    CompositeSampler(numMover + 1), numMover(numMover)
  {
    // select mover
    sons[0] = Variable(new EnumerationDiscreteSampler(numMover));
    for (int i = 0; i < samplers.size(); i++)
      sons[i + 1] = Variable(samplers[i]);
  }

  ProteinMoverSampler(DenseDoubleVectorPtr& probabilitiesMover, std::vector<Variable>& samplers) :
    CompositeSampler(probabilitiesMover->getNumElements() + 1), numMover(
        probabilitiesMover->getNumElements())
  {
    // select mover
    sons[0] = Variable(new EnumerationDiscreteSampler(probabilitiesMover));
    for (int i = 0; i < samplers.size(); i++)
      sons[i + 1] = Variable(samplers[i]);
  }

  Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    size_t whichMover = sons[0].getObjectAndCast<EnumerationDiscreteSampler> ()->sample(context,
        random, inputs).getInteger();
    SamplerPtr sampler = sons[whichMover + 1].getObjectAndCast<Sampler> ();
    ProteinMoverPtr sampled = (sampler->sample(context, random, inputs)).getObjectAndCast<
        ProteinMover> ();

    Variable mover = Variable(sampled);

    return Variable(mover);
  }

  /**
   * dataset = first : ProteinMoverPtr observed
   *           second : not yet used
   */
  void learn(ExecutionContext& context, const RandomGeneratorPtr& random, const std::vector<
      std::pair<Variable, Variable> >& dataset)
  {
    std::vector<std::pair<Variable, Variable> > moverFrequencies;
    std::vector<std::pair<Variable, Variable> > phiPsiSamples;
    std::vector<std::pair<Variable, Variable> > shearSamples;
    std::vector<std::pair<Variable, Variable> > rigidBodyTransSamples;
    std::vector<std::pair<Variable, Variable> > rigidBodySpinSamples;
    std::vector<std::pair<Variable, Variable> > rigidBodyGeneralSamples;

    for (int i = 0; i < dataset.size(); i++)
    {
      ProteinMoverPtr t = dataset[i].first.getObjectAndCast<ProteinMover> ();

      if (t.isInstanceOf<PhiPsiMover> ())
      {
        moverFrequencies.push_back(std::pair<Variable, Variable>(Variable(phipsi), Variable()));
        phiPsiSamples.push_back(std::pair<Variable, Variable>(Variable(t), Variable()));
      }
      else if (t.isInstanceOf<ShearMover> ())
      {
        moverFrequencies.push_back(std::pair<Variable, Variable>(Variable(shear), Variable()));
        shearSamples.push_back(std::pair<Variable, Variable>(Variable(t), Variable()));
      }
      else if (t.isInstanceOf<RigidBodyTransMover> ())
      {
        moverFrequencies.push_back(std::pair<Variable, Variable>(Variable(rigidbodytrans),
            Variable()));
        rigidBodyTransSamples.push_back(std::pair<Variable, Variable>(Variable(t), Variable()));
      }
      else if (t.isInstanceOf<RigidBodySpinMover> ())
      {
        moverFrequencies.push_back(std::pair<Variable, Variable>(Variable(rigidbodyspin),
            Variable()));
        rigidBodySpinSamples.push_back(std::pair<Variable, Variable>(Variable(t), Variable()));
      }
      else if (t.isInstanceOf<RigidBodyGeneralMover> ())
      {
        moverFrequencies.push_back(std::pair<Variable, Variable>(Variable(rigidbodygeneral),
            Variable()));
        rigidBodyGeneralSamples.push_back(std::pair<Variable, Variable>(Variable(t), Variable()));
      }
    }

    sons[0].getObjectAndCast<Sampler> ()->learn(context, random, moverFrequencies);
    sons[1].getObjectAndCast<Sampler> ()->learn(context, random, phiPsiSamples);
    sons[2].getObjectAndCast<Sampler> ()->learn(context, random, shearSamples);
    sons[3].getObjectAndCast<Sampler> ()->learn(context, random, rigidBodyTransSamples);
    sons[4].getObjectAndCast<Sampler> ()->learn(context, random, rigidBodySpinSamples);
    sons[5].getObjectAndCast<Sampler> ()->learn(context, random, rigidBodyGeneralSamples);

  }

protected:
  friend class ProteinMoverSamplerClass;
  size_t numMover;
};

}; /* namespace lbcpp */


#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_MOVER_SAMPLER_H_
