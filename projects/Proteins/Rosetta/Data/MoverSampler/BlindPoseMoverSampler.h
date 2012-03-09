/*-----------------------------------------.---------------------------------.
| Filename: BlindPoseMoverSampler.h        | Blind Pose Mover Sampler        |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 6, 2012  3:52:42 PM        |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_BLINDPOSEMOVERSAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_BLINDPOSEMOVERSAMPLER_H_

# include "precompiled.h"
# include <lbcpp/Sampler/Sampler.h>

# include "BlindResiduePairSampler.h"
# include "BlindResidueSampler.h"
# include "../Mover/PoseMover.h"

namespace lbcpp
{

class BlindPoseMoverSampler : public CompositeSampler
{
public:
  BlindPoseMoverSampler() : numResidues(0) {}
  BlindPoseMoverSampler(size_t numResidues)
    : numResidues(numResidues)
    {createObjectSamplers(numResidues);}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    size_t index = (size_t)samplers.back()->sample(context, random, inputs).getInteger();
    jassert(index < samplers.size() - 1);
    return samplers[index]->sample(context, random, inputs);
  }

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs,
      std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
    {}

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights, const ContainerPtr& validationInputs,
      const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
    {}

  virtual DenseDoubleVectorPtr computeLogProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
  {
    DenseDoubleVectorPtr probabilities = new DenseDoubleVector(samples->getNumElements(), 0.0);
    ContainerPtr input = variableVector(1);
    ContainerPtr sample = variableVector(1);
    ContainerPtr moverType = variableVector(1);

    for (size_t i = 0; i < samples->getNumElements(); i++)
    {
      double tempProbability = 0.0;
      input->setElement(0, inputs->getElement(i));
      sample->setElement(0, samples->getElement(i));
      TypePtr type = sample->getElement(0).getType();

      size_t target = 0;
      if (type == phiPsiMoverClass)
        target = 0;
      else if (type == shearMoverClass)
        target = 1;
      else if (type == rigidBodyMoverClass)
        target = 2;
      else
      {
        jassert(false);
      }

      tempProbability += samplers[target]->computeLogProbabilities(input, sample)->getValue(0);
      moverType->setElement(0, Variable(target, poseMoverEnumerationEnumeration));

      tempProbability += samplers.back()->computeLogProbabilities(input, moverType)->getValue(0);

      probabilities->setValue(i, tempProbability);
    }

    return probabilities;
  }

protected:
  friend class BlindPoseMoverSamplerClass;

  void createObjectSamplers(size_t numResidues)
  {
    SamplerPtr moverSampler = enumerationSampler(poseMoverEnumerationEnumeration);
    // mover parameters
    // residues
    SamplerPtr phiPsiResidues = new BlindResidueSampler(numResidues);
    SamplerPtr shearResidues = new BlindResidueSampler(numResidues);
    SamplerPtr rbResidues = new BlindResiduePairSampler(numResidues);
    // other parameters
    SamplerPtr phiPsiDeltaPhi = gaussianSampler(0, 25);
    SamplerPtr phiPsiDeltaPsi = gaussianSampler(0, 25);
    SamplerPtr shearDeltaPhi = gaussianSampler(0, 25);
    SamplerPtr shearDeltaPsi = gaussianSampler(0, 25);
    SamplerPtr rbDeltaMag = gaussianSampler(1, 1);
    SamplerPtr rbDeltaAmp = gaussianSampler(0, 25);

    // add movers' samplers
    samplers.push_back(objectCompositeSampler(phiPsiMoverClass, phiPsiResidues, phiPsiDeltaPhi, phiPsiDeltaPsi));
    samplers.push_back(objectCompositeSampler(shearMoverClass, shearResidues, shearDeltaPhi, shearDeltaPsi));
    samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, rbResidues, rbDeltaMag, rbDeltaAmp));

    // and sampler to choose mover
    samplers.push_back(moverSampler);
  }

  size_t numResidues;
};

typedef ReferenceCountedObjectPtr<BlindPoseMoverSampler> BlindPoseMoverSamplerPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_BLINDPOSEMOVERSAMPLER_H_
