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
# include "../ProteinMover/ShearMover.h"
# include "../ProteinMover/RigidBodyMover.h"

namespace lbcpp
{

enum ProteinMoverEnumeration
{
  phipsi = 0,
  shear,
  rigidbodytrans,
  rigidbodyspin,
  rigidbodygeneral,
  numberOfMovers
};

extern EnumerationPtr proteinMoverEnumerationEnumeration;

class ProteinMoverSampler;
typedef ReferenceCountedObjectPtr<ProteinMoverSampler> ProteinMoverSamplerPtr;

class ProteinMoverSampler : public CompositeSampler
{
public:
  ProteinMoverSampler()
    : CompositeSampler(0), numMover(0)
  {
  }

  /**
   * Each variable of samplers is a Ptr to a sampler. Do not deallocate
   * a sampler before deallocating ProteinMoverSampler.
   */
  ProteinMoverSampler(size_t numMover, std::vector<Variable>& samplers)
    : CompositeSampler(numMover + 1), numMover(numMover)
  {
    // select mover
    samplers[0] = new EnumerationDiscreteSampler(proteinMoverEnumerationEnumeration, numMover, 1.0 / numMover);
    whichMover = std::vector<size_t>(numberOfMovers, -1);
    for (size_t i = 0; i < samplers.size(); i++)
    {
      SamplerPtr t = samplers[i].getObjectAndCast<Sampler>();
      samplers[i + 1] = t;

      if (t.isInstanceOf<PhiPsiMoverSampler> ())
        whichMover[phipsi] = i;
      else if (t.isInstanceOf<ShearMoverSampler> ())
        whichMover[shear] = i;
      else if (t.isInstanceOf<RigidBodyTransMoverSampler> ())
        whichMover[rigidbodytrans] = i;
      else if (t.isInstanceOf<RigidBodySpinMoverSampler> ())
        whichMover[rigidbodyspin] = i;
      else if (t.isInstanceOf<RigidBodyGeneralMoverSampler> ())
        whichMover[rigidbodygeneral] = i;
    }
  }

  ProteinMoverSampler(DenseDoubleVectorPtr& probabilitiesMover, std::vector<Variable>& samplers)
    : CompositeSampler(probabilitiesMover->getNumElements() + 1),
      numMover(probabilitiesMover->getNumElements())
  {
    // select mover
    samplers[0] = new EnumerationDiscreteSampler(proteinMoverEnumerationEnumeration, numMover);
    whichMover = std::vector<size_t>(numberOfMovers, -1);
    for (size_t i = 0; i < samplers.size(); i++)
    {
      SamplerPtr t = samplers[i].getObjectAndCast<Sampler> ();
      samplers[i + 1] = t;
      
      if (t.isInstanceOf<PhiPsiMoverSampler> ())
        whichMover[phipsi] = i;
      else if (t.isInstanceOf<ShearMoverSampler> ())
        whichMover[shear] = i;
      else if (t.isInstanceOf<RigidBodyTransMoverSampler> ())
        whichMover[rigidbodytrans] = i;
      else if (t.isInstanceOf<RigidBodySpinMoverSampler> ())
        whichMover[rigidbodyspin] = i;
      else if (t.isInstanceOf<RigidBodyGeneralMoverSampler> ())
        whichMover[rigidbodygeneral] = i;
    }
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    size_t indexMover = samplers[0]->sample(context, random, inputs).getInteger();
    SamplerPtr sampler = samplers[indexMover + 1];
    return sampler->sample(context, random, inputs);
  }

  /**
   * dataset = first : ProteinMoverPtr observed
   *           second : not yet used
   */
  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    if (dataset.size() < 1)
      return;

    std::vector<Variable> moverFrequencies;
    std::vector<std::vector<Variable> > samples(numMover);

    for (size_t i = 0; i < dataset.size(); i++)
    {
      ProteinMoverPtr t = dataset[i].getObjectAndCast<ProteinMover> ();

      if (t.isInstanceOf<PhiPsiMover> ())
      {
        if (whichMover[phipsi] >= 0)
        {
          moverFrequencies.push_back(whichMover[phipsi]);
          samples[whichMover[phipsi]].push_back(t);
        }
      }
      else if (t.isInstanceOf<ShearMover> ())
      {
        if (whichMover[shear] >= 0)
        {
          moverFrequencies.push_back(whichMover[shear]);
          samples[whichMover[shear]].push_back(t);
        }
      }
      else if (t.isInstanceOf<RigidBodyMover> ())
      {
        if (whichMover[rigidbodygeneral] >= 0)
        {
          moverFrequencies.push_back(whichMover[rigidbodygeneral]);
          samples[whichMover[rigidbodygeneral]].push_back(t);
        }
      }
    }

    samplers[0]->learn(context, moverFrequencies);
    for (size_t i = 0; i < numMover; i++)
      samplers[i + 1]->learn(context, samples[i]);
  }

protected:
  friend class ProteinMoverSamplerClass;
  size_t numMover;
  std::vector<size_t> whichMover;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_MOVER_SAMPLER_H_
