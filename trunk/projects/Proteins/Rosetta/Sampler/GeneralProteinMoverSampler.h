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
  GeneralSimpleResidueSampler()
  {
    samplers.push_back(gaussianSampler(0.5, 0.3));
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    size_t indexResidue = (size_t)juce::jlimit(0, (*inputs).getInteger() - 1, (int)std::floor(
        samplers[0]->sample(context, random).getDouble() * (*inputs).getInteger()));
    return indexResidue;
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& supervisionWeights = DenseDoubleVectorPtr())
  {
    VariableVectorPtr values = new VariableVector(0);
    for (size_t i = 0; i < trainingSamples->getNumElements(); i++)
    {
      size_t residue = trainingSamples->getElement(i).getInteger();
      size_t length = trainingInputs->getElement(i).getInteger();
      Variable temp((double)residue / (double)length);
      values->append(temp);
    }
    samplers[0]->learn(context, ContainerPtr(), values, DenseDoubleVectorPtr(), ContainerPtr(), ContainerPtr(), DenseDoubleVectorPtr());
  }

protected:
  friend class GeneralSimpleResidueSamplerClass;
};

typedef ReferenceCountedObjectPtr<GeneralSimpleResidueSampler> GeneralSimpleResidueSamplerPtr;

class GeneralResiduePairSampler : public CompositeSampler
{
public:
  GeneralResiduePairSampler()
  {
    samplers.push_back(new GeneralSimpleResidueSampler());
    samplers.push_back(new GeneralSimpleResidueSampler());
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    size_t indexResidueOne = (size_t)samplers[0]->sample(context, random, inputs).getInteger();
    size_t indexResidueTwo = (size_t)samplers[1]->sample(context, random, inputs).getInteger();

    if (std::abs((int)indexResidueOne - (int)indexResidueTwo) <= 1)
    {
      indexResidueOne = (size_t)juce::jlimit(0, (int)(*inputs).getInteger() - 1,
          (int)indexResidueOne - 2);
      indexResidueOne = (size_t)juce::jlimit(0, (int)(*inputs).getInteger() - 1,
          (int)indexResidueTwo + 2);
    }
    PairPtr residues = new Pair(indexResidueOne, indexResidueTwo);
    return residues;
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& supervisionWeights = DenseDoubleVectorPtr())
  {
    VariableVectorPtr valuesOne = new VariableVector(0);
    VariableVectorPtr valuesTwo = new VariableVector(0);
    PairPtr residues;
    for (size_t i = 0; i < trainingSamples->getNumElements(); i++)
    {
      residues = (trainingSamples->getElement(i)).getObjectAndCast<Pair> ();
      Variable temp1 = residues->getFirst();
      Variable temp2 = residues->getSecond();
      valuesOne->append(temp1);
      valuesTwo->append(temp2);
    }
    samplers[0]->learn(context, trainingInputs, valuesOne, DenseDoubleVectorPtr(), ContainerPtr(),
        ContainerPtr(), DenseDoubleVectorPtr());
    samplers[1]->learn(context, trainingInputs, valuesTwo, DenseDoubleVectorPtr(), ContainerPtr(),
        ContainerPtr(), DenseDoubleVectorPtr());
  }

protected:
  friend class GeneralResiduePairSamplerClass;
};

class GeneralProteinMoverSampler : public CompositeSampler
{
public:
  GeneralProteinMoverSampler()
  {
    createObjectSamplers();
    samplers.push_back(enumerationSampler(proteinMoverEnumerationEnumeration));
  }

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
  friend class GeneralProteinMoverSamplerClass;

  void createObjectSamplers()
  {
    samplers.push_back(objectCompositeSampler(phiPsiMoverClass, new GeneralSimpleResidueSampler(), gaussianSampler(0, 25), gaussianSampler(0, 25)));
    samplers.push_back(objectCompositeSampler(shearMoverClass, new GeneralSimpleResidueSampler(), gaussianSampler(0, 25), gaussianSampler(0, 25)));
    samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, new GeneralResiduePairSampler(), gaussianSampler(1, 1), gaussianSampler(0, 25)));
  }
};

typedef ReferenceCountedObjectPtr<GeneralProteinMoverSampler> GeneralProteinMoverSamplerPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_GENERAL_PROTEIN_MOVER_SAMPLER_H_
