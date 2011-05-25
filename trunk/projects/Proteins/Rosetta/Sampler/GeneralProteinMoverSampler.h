/*-----------------------------------------.---------------------------------.
| Filename: GeneralProteinMoverSampler.h   | GeneralProteinMoverSampler      |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 21 mai 2011  16:15:42          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_GENERAL_PROTEIN_MOVER_SAMPLER_H_
# define LBCPP_SAMPLER_GENERAL_PROTEIN_MOVER_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "../ProteinMover.h"

namespace lbcpp
{

class GeneralSimpleResidueSampler : public CompositeSampler
{
public:
  GeneralSimpleResidueSampler(size_t numResidues)
    : numResidues(numResidues)
    {samplers.push_back(enumerationSampler(proteinMoverEnumerationEnumeration));}
  GeneralSimpleResidueSampler() : numResidues(0) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
    {return samplers[0]->sample(context, random).getInteger();}

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& supervisionWeights = DenseDoubleVectorPtr())
    {}

protected:
  friend class GeneralSimpleResidueSamplerClass;

  size_t numResidues;
};

typedef ReferenceCountedObjectPtr<GeneralSimpleResidueSampler> GeneralSimpleResidueSamplerPtr;

class GeneralResiduePairSampler : public CompositeSampler
{
public:
  GeneralResiduePairSampler(size_t numResidues)
    : numResidues(numResidues)
  {
    samplers.push_back(enumerationSampler(proteinMoverEnumerationEnumeration));
    samplers.push_back(enumerationSampler(proteinMoverEnumerationEnumeration));
  }
  GeneralResiduePairSampler() : numResidues(0) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    size_t indexResidueOne = (size_t)samplers[0]->sample(context, random).getInteger();
    size_t indexResidueTwo = (size_t)samplers[1]->sample(context, random).getInteger();

    if (std::abs((int)indexResidueOne - (int)indexResidueTwo) <= 1)
    {
      indexResidueOne = (size_t)juce::jlimit(0, (int)numResidues - 1, (int)indexResidueOne - 2);
      indexResidueOne = (size_t)juce::jlimit(0, (int)numResidues - 1, (int)indexResidueTwo + 2);
    }
    PairPtr residues = new Pair(indexResidueOne, indexResidueTwo);
    return residues;
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& supervisionWeights = DenseDoubleVectorPtr())
    {}

protected:
  friend class GeneralResiduePairSamplerClass;

  size_t numResidues;
};

typedef ReferenceCountedObjectPtr<GeneralResiduePairSampler> GeneralResiduePairSamplerPtr;

class ClamperSampler : public CompositeSampler
{
public:
  ClamperSampler(double minValue, double maxValue)
    : CompositeSampler(1), minValue(minValue), maxValue(maxValue), mean((minValue + maxValue) / 2)
    {samplers[0] = conditionalGaussianSampler();}
  ClamperSampler() : CompositeSampler(1), minValue(0), maxValue(0), mean(0) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    double value = samplers[0]->sample(context, random, inputs).getDouble();

    value *= 0.5 * (maxValue - minValue);
    value += mean;
    value = juce::jlimit(minValue, maxValue, value);
    return value;
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& supervisionWeights = DenseDoubleVectorPtr())
  {
    double normalize = 1.0 / (0.5 * (maxValue - minValue));
    VectorPtr samples = new DenseDoubleVector(trainingSamples->getNumElements(), 0.0);
    for (size_t i = 0; i < trainingSamples->getNumElements(); i++)
    {
      double valueToSet = juce::jlimit(minValue, maxValue, trainingSamples->getElement(i).getDouble());
      valueToSet -= mean;
      valueToSet *= normalize;
      samples->setElement(i, valueToSet);
    }

    samplers[0]->learn(context, trainingInputs, samples, trainingWeights, validationInputs,
        validationSamples, supervisionWeights);
  }

protected:
  friend class ClamperSamplerClass;

  double minValue;
  double maxValue;
  double mean;
};

class GaussianSamplerWithoutLearn : public CompositeSampler
{
public:

  GaussianSamplerWithoutLearn(double mean, double std)
      : CompositeSampler(1)
      {samplers[0] = gaussianSampler(mean, std);}
  GaussianSamplerWithoutLearn() : CompositeSampler(1) {}

    virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
      {return samplers[0]->sample(context, random, inputs);}

    virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                  const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& supervisionWeights = DenseDoubleVectorPtr())
      {}

protected:
  friend class GaussianSamplerWithoutLearnClass;
};

class GeneralProteinMoverSampler : public CompositeSampler
{
public:
  GeneralProteinMoverSampler(size_t numResidues, size_t learningPolicy)
    : numResidues(numResidues), learningPolicy(learningPolicy)
    {createObjectSamplers(numResidues, learningPolicy);}
  GeneralProteinMoverSampler() : numResidues(0), learningPolicy(0) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    size_t index = (size_t)samplers.back()->sample(context, random, inputs).getInteger();
    jassert(index < samplers.size() - 1);
    return samplers[index]->sample(context, random, inputs);
  }

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples,
      const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs, std::vector<
          ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
  {
    size_t n = samples->getNumElements();

    size_t numSamplers = samplers.size();
    subInputs.resize(numSamplers);
    subSamples.resize(numSamplers);
    subWeights.resize(numSamplers);

    VectorPtr classSamples = vector(proteinMoverEnumerationEnumeration, n);
    for (size_t i = 0; i < numSamplers - 1; ++i)
    {
      subSamples[i] = new ObjectVector(proteinMoverClass, 0);
      if (inputs)
        subInputs[i] = vector(inputs->getElementsType());
    }
    subInputs.back() = inputs;
    subSamples.back() = classSamples;

    for (size_t i = 0; i < n; ++i)
    {
      Variable element = samples->getElement(i);
      TypePtr type = element.getType();

      size_t target;
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

      classSamples->setElement(i, Variable(target, proteinMoverEnumerationEnumeration));

      subSamples[target].staticCast<Vector> ()->append(element);
      if (inputs)
        subInputs[target].staticCast<Vector> ()->append(inputs->getElement(i));
    }
  }

  void learn(ExecutionContext& context, const ContainerPtr& trainingInputs,
      const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
      const ContainerPtr& validationInputs, const ContainerPtr& validationSamples,
      const DenseDoubleVectorPtr& supervisionWeights)
  {
    if (learningPolicy)
    {
      std::vector<ContainerPtr> subTrainingInputs(samplers.size()), subTrainingSamples(
          samplers.size()), subTrainingWeights(samplers.size());
      makeSubExamples(trainingInputs, trainingSamples, trainingWeights, subTrainingInputs,
          subTrainingSamples, subTrainingWeights);
      jassert(subTrainingInputs.size() == samplers.size() && subTrainingSamples.size() == samplers.size());

      if (validationSamples)
      {
        std::vector<ContainerPtr> subValidationInputs(samplers.size()), subValidationSamples(
            samplers.size()), subValidationWeights(samplers.size());
        makeSubExamples(validationInputs, validationSamples, supervisionWeights,
            subValidationInputs, subValidationSamples, subValidationWeights);
        jassert(subValidationInputs.size() == samplers.size() && subValidationSamples.size() == samplers.size());

        for (size_t i = 0; i < samplers.size(); ++i)
          if (subTrainingSamples[i]->getNumElements())
            samplers[i]->learn(context, subTrainingInputs[i], subTrainingSamples[i],
                subTrainingWeights[i], subValidationInputs[i], subValidationSamples[i],
                subValidationWeights[i]);
      }
      else
      {
        for (size_t i = 0; i < samplers.size(); ++i)
          if (subTrainingSamples[i]->getNumElements())
            samplers[i]->learn(context, subTrainingInputs[i], subTrainingSamples[i]);
      }
    }
  }

protected:
  friend class GeneralProteinMoverSamplerClass;

  void createObjectSamplers(size_t numResidues, size_t learningPolicy)
  {
    // create samplers, by default corresponds to learningPolicy == 0 and 1
    // mover
    SamplerPtr moverSampler = enumerationSampler(proteinMoverEnumerationEnumeration);
    // mover parameters
    // residues
    SamplerPtr phiPsiResidues = new GeneralSimpleResidueSampler(numResidues);
    SamplerPtr shearResidues = new GeneralSimpleResidueSampler(numResidues);
    SamplerPtr rbResidues = new GeneralResiduePairSampler(numResidues);
    // other parameters
    SamplerPtr phiPsiDeltaPhi = new GaussianSamplerWithoutLearn(0, 25);
    SamplerPtr phiPsiDeltaPsi = new GaussianSamplerWithoutLearn(0, 25);
    SamplerPtr shearDeltaPhi = new GaussianSamplerWithoutLearn(0, 25);
    SamplerPtr shearDeltaPsi = new GaussianSamplerWithoutLearn(0, 25);
    SamplerPtr rbDeltaMag = new GaussianSamplerWithoutLearn(1, 1);
    SamplerPtr rbDeltaAmp = new GaussianSamplerWithoutLearn(0, 25);

    if (learningPolicy == 2)
    {
      moverSampler = maximumEntropySampler(proteinMoverEnumerationEnumeration);
    }
    else if (learningPolicy == 3)
    {
      moverSampler = maximumEntropySampler(proteinMoverEnumerationEnumeration);
      phiPsiDeltaPhi = new ClamperSampler(-180, 180);
      phiPsiDeltaPsi = new ClamperSampler(-180, 180);
      shearDeltaPhi = new ClamperSampler(-180, 180);
      shearDeltaPsi = new ClamperSampler(-180, 180);
      rbDeltaMag = new ClamperSampler(-5, 5);
      rbDeltaAmp = new ClamperSampler(-180, 180);
    }

    // add movers' samplers
    samplers.push_back(objectCompositeSampler(phiPsiMoverClass, phiPsiResidues, phiPsiDeltaPhi, phiPsiDeltaPsi));
    samplers.push_back(objectCompositeSampler(shearMoverClass, shearResidues, shearDeltaPhi, shearDeltaPsi));
    samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, rbResidues, rbDeltaMag, rbDeltaAmp));

    // and sampler to choose mover
    samplers.push_back(moverSampler);
  }

  size_t numResidues;
  size_t learningPolicy;
  // 0 -> no learning, all random
  // 1 -> learned the enumeration sampler for movers
  // 2 -> learned maxent for movers
  // 3 -> learned maxent for movers and conditionnal gaussian for angles and distances
};

typedef ReferenceCountedObjectPtr<GeneralProteinMoverSampler> GeneralProteinMoverSamplerPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_GENERAL_PROTEIN_MOVER_SAMPLER_H_
