/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceFactory.cpp    | Protein Inference Factory       |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 23:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinInferenceFactory.h"
#include <lbcpp/Data/Perception.h>
#include <lbcpp/Inference/DecoratorInference.h>
using namespace lbcpp;

#include <lbcpp/Inference/ParallelInference.h>
class SharedParallelContainerInference : public SharedParallelInference
{
public:
  SharedParallelContainerInference(const String& name, InferencePtr subInference)
    : SharedParallelInference(name, subInference)
    {}
  
  SharedParallelContainerInference() {}
  
  virtual size_t getNumSubInferences(const Variable& input) const = 0;
  virtual ContainerPtr getSupervisionTargets(const Variable& supervision) const = 0;
  virtual Variable getSubInput(const Variable& input, size_t index) const = 0;

  virtual Variable createOutput(const Variable& input) const = 0;
  virtual void addResultToOutput(Variable& output, size_t index, const Variable& subInferenceOutput) const = 0;

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    size_t n = getNumSubInferences(input);

    ContainerPtr supervisionTargets;
    if (supervision)
    {
      supervisionTargets = getSupervisionTargets(supervision);
      jassert(supervisionTargets->size() == n);
    }

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(n);
    if (supervisionTargets)
      for (size_t i = 0; i < n; ++i)
        res->addSubInference(subInference, getSubInput(input, i), supervisionTargets->getVariable(i));
    else
      for (size_t i = 0; i < n; ++i)
        res->addSubInference(subInference, getSubInput(input, i), Variable());
    return res;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    size_t n = state->getNumSubInferences();

    Variable res = createOutput(state->getInput());
    bool atLeastOnePrediction = false;
    for (size_t i = 0; i < n; ++i)
    {
      Variable subOutput = state->getSubOutput(i);
      if (subOutput)
      {
        atLeastOnePrediction = true;
        addResultToOutput(res, i, subOutput);
      }
    }
    return atLeastOnePrediction ? res : Variable::missingValue(res.getType());
  }
};

class SequenceProteinTargetInference : public SharedParallelContainerInference
{
public:
  SequenceProteinTargetInference(const String& targetName, InferencePtr elementInference, PerceptionPtr perception)
    : SharedParallelContainerInference(targetName, elementInference), perception(perception)
  {
    int index = proteinClass()->findStaticVariable(targetName);
    jassert(index >= 0);
    targetIndex = (size_t)index;
    outputType = proteinClass()->getStaticVariableType(targetIndex);

  }
  SequenceProteinTargetInference() : targetIndex(0) {}

  virtual TypePtr getInputType() const
    {return proteinClass();}

  virtual TypePtr getSupervisionType() const
    {return outputType;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return outputType;}

  virtual size_t getNumSubInferences(const Variable& input) const
    {return input.getObjectAndCast<Protein>()->getLength();}

  virtual ContainerPtr getSupervisionTargets(const Variable& supervision) const
    {return supervision[targetIndex].getObjectAndCast<Container>();}

  virtual Variable getSubInput(const Variable& input, size_t index) const
    {return perception->compute(Variable::pair(input, index));}

  virtual Variable createOutput(const Variable& input) const
    {return input.getObjectAndCast<Protein>()->createEmptyTarget(targetIndex);}

  virtual void addResultToOutput(Variable& output, size_t index, const Variable& subInferenceOutput) const
  {
    Variable result = subInferenceOutput;
    if (subInferenceOutput.isObject())
    {
      DiscreteProbabilityDistributionPtr distribution = subInferenceOutput.dynamicCast<DiscreteProbabilityDistribution>();
      if (distribution)
        result = distribution->sample(RandomGenerator::getInstance());
    }
    output.getObject()->setVariable(index, result);
  }

protected:
  size_t targetIndex;
  PerceptionPtr perception;
  TypePtr outputType;
};

//////////////////////////////

ProteinInferenceFactory::ProteinInferenceFactory()
  : proteinClass(lbcpp::proteinClass())
{
  LBCPP_DECLARE_ABSTRACT_CLASS(SharedParallelContainerInference, SharedParallelInference);
    LBCPP_DECLARE_CLASS(SequenceProteinTargetInference, SharedParallelContainerInference);
}

InferencePtr ProteinInferenceFactory::createInference(const String& targetName) const
  {return addToProteinInference(createTargetInference(targetName), targetName);}

InferencePtr ProteinInferenceFactory::createTargetInference(const String& targetName) const
{
  if (targetName == T("secondaryStructure") || targetName == T("dsspSecondaryStructure")
        || targetName == T("structuralAlphabetSequence"))
    return createSequenceLabelingInference(targetName);
  jassert(false);
  return InferencePtr();
}

InferencePtr ProteinInferenceFactory::createSequenceLabelingInference(const String& targetName) const
{
  TypePtr targetType = getTargetType(targetName);
  EnumerationPtr elementsType = targetType->getTemplateArgument(0).dynamicCast<Enumeration>();
  jassert(elementsType);
  PerceptionPtr perception = createPerception(targetName, true, false);
  InferencePtr classifier = createMultiClassClassifier(targetName, perception->getOutputType(), elementsType);
  return InferencePtr(new SequenceProteinTargetInference(targetName, classifier, perception));
}

size_t ProteinInferenceFactory::getTargetIndex(const String& targetName) const
{
  int targetIndex = proteinClass->findStaticVariable(targetName);
  jassert(targetIndex >= 0);
  return (size_t)targetIndex;
}

TypePtr ProteinInferenceFactory::getTargetType(const String& targetName) const
  {return proteinClass->getStaticVariableType(getTargetIndex(targetName));}

InferencePtr ProteinInferenceFactory::addToProteinInference(InferencePtr targetInference, const String& targetName) const
  {return postProcessInference(targetInference, setFieldFunction(getTargetIndex(targetName)));}

void ProteinInferenceFactory::createPrimaryStructureResiduePerception(CompositePerceptionPtr res) const
  {res->addPerception(T("AA"), applyPerceptionOnProteinVariable(T("primaryStructure"), windowPerception(aminoAcidTypeEnumeration(), 15)));}

void ProteinInferenceFactory::createPositionSpecificScoringMatrixResiduePerception(CompositePerceptionPtr res) const
{
  TypePtr pssmRowType = discreteProbabilityDistributionClass(aminoAcidTypeEnumeration());

  PerceptionPtr pssmRowPerception = identityPerception(pssmRowType);
  res->addPerception(T("PSSM"), applyPerceptionOnProteinVariable(T("positionSpecificScoringMatrix"),
    windowPerception(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration()), 15, pssmRowPerception)));
}

PerceptionPtr ProteinInferenceFactory::createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const
{
  if (is1DTarget)
  {
    CompositePerceptionPtr res = new ResidueCompositePerception();
    createPrimaryStructureResiduePerception(res);
    createPositionSpecificScoringMatrixResiduePerception(res);
    return res;
  }
  jassert(false);
  return PerceptionPtr();
}

PerceptionPtr ProteinInferenceFactory::applyPerceptionOnProteinVariable(const String& variableName, PerceptionPtr variablePerception) const
{
   FunctionPtr selectVariableFunction = selectPairFieldsFunction(proteinClass->findStaticVariable(variableName));
   return Perception::compose(selectVariableFunction, variablePerception);
}
