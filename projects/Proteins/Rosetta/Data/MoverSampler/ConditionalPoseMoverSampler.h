/*-----------------------------------------.---------------------------------.
| Filename: ConditionalPoseMoverSampler.h  | Conditional Pose                |
| Author  : Alejandro Marcos Alvarez       | Mover Sampler                   |
| Started : Mar 9, 2012  8:11:20 AM        |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_CONDITIONALPOSEMOVERSAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_CONDITIONALPOSEMOVERSAMPLER_H_

# include "precompiled.h"
# include "BlindResiduePairSampler.h"
# include "BlindResidueSampler.h"

namespace lbcpp
{

class ConditionalPoseMoverSampler : public CompositeSampler
{
public:
  ConditionalPoseMoverSampler() : numResidues(0), hasBeenLearned(false) {}
  ConditionalPoseMoverSampler(size_t numResidues) : numResidues(numResidues), hasBeenLearned(false) {createObjectSamplers(numResidues);}


  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    jassert(hasBeenLearned);
    size_t index = (size_t)samplers.back()->sample(context, random, inputs).getInteger();
    jassert(index < samplers.size() - 1);
    return samplers[index]->sample(context, random, inputs);
  }

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs,
      std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
  {
    size_t n = samples->getNumElements();

    size_t numSamplers = samplers.size();
    subInputs.resize(numSamplers);
    subSamples.resize(numSamplers);
    subWeights.resize(numSamplers);

    VectorPtr classSamples = vector(poseMoverEnumerationEnumeration, n);
    for (size_t i = 0; i < numSamplers - 1; ++i)
    {
      subSamples[i] = new ObjectVector(poseMoverClass, 0);
      if (inputs)
        subInputs[i] = vector(inputs->getElementsType());
    }
    subInputs.back() = inputs;
    subSamples.back() = classSamples;

    for (size_t i = 0; i < n; ++i)
    {
      Variable element = samples->getElement(i);
      TypePtr type = element.getType();

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

      classSamples->setElement(i, Variable(target, poseMoverEnumerationEnumeration));

      subSamples[target].staticCast<Vector> ()->append(element);
      if (inputs)
        subInputs[target].staticCast<Vector> ()->append(inputs->getElement(i));
    }
  }

  void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights, const ContainerPtr& validationInputs,
      const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
  {
    std::vector<ContainerPtr> subTrainingInputs(samplers.size()), subTrainingSamples(samplers.size()), subTrainingWeights(samplers.size());
    makeSubExamples(trainingInputs, trainingSamples, trainingWeights, subTrainingInputs, subTrainingSamples, subTrainingWeights);
    jassert(subTrainingInputs.size() == samplers.size() && subTrainingSamples.size() == samplers.size());

    if (validationSamples)
    {
      std::vector<ContainerPtr> subValidationInputs(samplers.size()), subValidationSamples(samplers.size()), subValidationWeights(samplers.size());
      makeSubExamples(validationInputs, validationSamples, supervisionWeights, subValidationInputs, subValidationSamples, subValidationWeights);
      jassert(subValidationInputs.size() == samplers.size() && subValidationSamples.size() == samplers.size());

      for (size_t i = 0; i < samplers.size(); ++i)
        if (subTrainingSamples[i]->getNumElements())
          samplers[i]->learn(context, subTrainingInputs[i], subTrainingSamples[i], subTrainingWeights[i], subValidationInputs[i], subValidationSamples[i], subValidationWeights[i]);
    }
    else
    {
      for (size_t i = 0; i < samplers.size(); ++i)
        if (subTrainingSamples[i]->getNumElements())
          samplers[i]->learn(context, subTrainingInputs[i], subTrainingSamples[i]);
    }
    hasBeenLearned = true;
  }

  virtual DenseDoubleVectorPtr computeLogProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
  {
    jassert(hasBeenLearned);

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
  friend class ConditionalPoseMoverSamplerClass;

  void createObjectSamplers(size_t numResidues)
  {
    SamplerPtr moverSampler = maximumEntropySampler(poseMoverEnumerationEnumeration);

    // mover parameters
    // residues
    SamplerPtr phiPsiResidues = new BlindResidueSampler(numResidues);
    SamplerPtr shearResidues = new BlindResidueSampler(numResidues);
    SamplerPtr rbResidues = new BlindResiduePairSampler(numResidues);
    // other parameters
    SamplerPtr phiPsiDeltaPhi = clamperSampler(-180, 180, conditionalGaussianSampler());
    SamplerPtr phiPsiDeltaPsi = clamperSampler(-180, 180, conditionalGaussianSampler());
    SamplerPtr shearDeltaPhi = clamperSampler(-180, 180, conditionalGaussianSampler());
    SamplerPtr shearDeltaPsi = clamperSampler(-180, 180, conditionalGaussianSampler());
    SamplerPtr rbDeltaMag = clamperSampler(-5, 5, conditionalGaussianSampler());
    SamplerPtr rbDeltaAmp = clamperSampler(-180, 180, conditionalGaussianSampler());

    // add movers' samplers
    samplers.push_back(objectCompositeSampler(phiPsiMoverClass, phiPsiResidues, phiPsiDeltaPhi, phiPsiDeltaPsi));
    samplers.push_back(objectCompositeSampler(shearMoverClass, shearResidues, shearDeltaPhi, shearDeltaPsi));
    samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, rbResidues, rbDeltaMag, rbDeltaAmp));

    // and sampler to choose mover
    samplers.push_back(moverSampler);
  }

  size_t numResidues;
  bool hasBeenLearned;
};

typedef ReferenceCountedObjectPtr<ConditionalPoseMoverSampler> ConditionalPoseMoverSamplerPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_CONDITIONALPOSEMOVERSAMPLER_H_
