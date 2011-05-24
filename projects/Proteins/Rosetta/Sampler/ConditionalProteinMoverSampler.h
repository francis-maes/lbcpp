/*-----------------------------------------.---------------------------------.
| Filename: ConditionalProtein..Sampler.h  | ConditionalProteinMoverSampler  |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 22 mai 2011  10:31:16          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_CONDITIONAL_PROTEIN_MOVER_SAMPLER_H_
# define LBCPP_SAMPLER_CONDITIONAL_PROTEIN_MOVER_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "../ProteinMover.h"
# include "../RosettaSandBox.h"
# include "GeneralProteinMoverSampler.h"

namespace lbcpp
{

class ConditionalSimpleResidueSampler : public CompositeSampler
{
public:
  ConditionalSimpleResidueSampler()
  {
    samplers.push_back(conditionalGaussianSampler());
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    jassert(inputs != NULL);
    size_t numResidues = (size_t)((inputs[0].getObjectAndCast<DenseDoubleVector>())->getValue(0));
    jassert(numResidues);
    double value = samplers[0]->sample(context, random, inputs).getDouble();
    size_t indexResidue = (size_t)juce::jlimit(0, (int)numResidues - 1, (int)std::floor(value));
    return indexResidue;
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& supervisionWeights = DenseDoubleVectorPtr())
  {
    VectorPtr training2 = new DenseDoubleVector(trainingSamples->getNumElements(), 0.0);
    for (size_t i = 0; i < trainingSamples->getNumElements(); i++)
      training2->setElement(i, trainingSamples->getElement(i).toDouble());

    samplers[0]->learn(context, trainingInputs, training2, trainingWeights, validationInputs,
        validationSamples, supervisionWeights);
  }

protected:
  friend class ConditionalSimpleResidueSamplerClass;
};

typedef ReferenceCountedObjectPtr<ConditionalSimpleResidueSampler> ConditionalSimpleResidueSamplerPtr;

class ConditionalResiduePairSampler : public CompositeSampler
{
public:
  ConditionalResiduePairSampler()
  {
    samplers.push_back(new ConditionalSimpleResidueSampler());
    samplers.push_back(new ConditionalSimpleResidueSampler());
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    size_t numResidues = (size_t)((inputs[0].getObjectAndCast<DenseDoubleVector>())->getValue(0));
    size_t indexResidueOne = (size_t)(samplers[0]->sample(context, random, inputs).getInteger());
    size_t indexResidueTwo = (size_t)(samplers[1]->sample(context, random, inputs).getInteger());

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
  {
    VectorPtr valuesOne = new DenseDoubleVector(trainingSamples->getNumElements(), 0.0);
    VectorPtr valuesTwo = new DenseDoubleVector(trainingSamples->getNumElements(), 0.0);
    PairPtr residues;
    for (size_t i = 0; i < trainingSamples->getNumElements(); i++)
    {
      residues = (trainingSamples->getElement(i)).getObjectAndCast<Pair> ();
      Variable temp1 = residues->getFirst();
      Variable temp2 = residues->getSecond();
      valuesOne->setElement(i, temp1.toDouble());
      valuesTwo->setElement(i, temp2.toDouble());
    }
    samplers[0]->learn(context, trainingInputs, valuesOne, trainingWeights, validationInputs,
        validationSamples, supervisionWeights);
    samplers[1]->learn(context, trainingInputs, valuesTwo, trainingWeights, validationInputs,
        validationSamples, supervisionWeights);
  }

protected:
  friend class ConditionalResiduePairSamplerClass;
};

typedef ReferenceCountedObjectPtr<ConditionalResiduePairSampler> ConditionalResiduePairSamplerPtr;

class ConditionalProteinMoverSampler : public CompositeSampler
{
public:
  ConditionalProteinMoverSampler(size_t level)
  {
    createObjectSamplers(level);
    samplers.push_back(maximumEntropySampler(proteinMoverEnumerationEnumeration));
  }

  ConditionalProteinMoverSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    size_t index = (size_t)samplers.back()->sample(context, random, inputs).getInteger();
    jassert(index < samplers.size() - 1);
    return samplers[index]->sample(context, random, inputs);
  }

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
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
        jassert(false);
      classSamples->setElement(i, Variable(target, proteinMoverEnumerationEnumeration));

      subSamples[target].staticCast<Vector>()->append(element);
      if (inputs)
        subInputs[target].staticCast<Vector>()->append(inputs->getElement(i));
    }
  }

protected:
  friend class ConditionalProteinMoverSamplerClass;

  void createObjectSamplers(size_t level)
    {
      if (level == 0)
      {
//        samplers.push_back(objectCompositeSampler(phiPsiMoverClass, new GeneralSimpleResidueSampler(), gaussianSampler(0, 25), gaussianSampler(0, 25)));
//        samplers.push_back(objectCompositeSampler(shearMoverClass, new GeneralSimpleResidueSampler(), gaussianSampler(0, 25), gaussianSampler(0, 25)));
//        samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, new GeneralResiduePairSampler(), gaussianSampler(1, 1), gaussianSampler(0, 25)));
      }
      else if (level == 1)
      {
        samplers.push_back(objectCompositeSampler(phiPsiMoverClass, new ConditionalSimpleResidueSampler(), gaussianSampler(0, 25), gaussianSampler(0, 25)));
        samplers.push_back(objectCompositeSampler(shearMoverClass, new ConditionalSimpleResidueSampler(), gaussianSampler(0, 25), gaussianSampler(0, 25)));
        samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, new ConditionalResiduePairSampler(), gaussianSampler(1, 1), gaussianSampler(0, 25)));
      }
      else
      {
        samplers.push_back(objectCompositeSampler(phiPsiMoverClass, new ConditionalSimpleResidueSampler(), conditionalGaussianSampler(), conditionalGaussianSampler()));
        samplers.push_back(objectCompositeSampler(shearMoverClass, new ConditionalSimpleResidueSampler(), conditionalGaussianSampler(), conditionalGaussianSampler()));
        samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, new ConditionalResiduePairSampler(), conditionalGaussianSampler(), conditionalGaussianSampler()));
      }
    }
  };

typedef ReferenceCountedObjectPtr<ConditionalProteinMoverSampler> ConditionalProteinMoverSamplerPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_CONDITIONAL_PROTEIN_MOVER_SAMPLER_H_
