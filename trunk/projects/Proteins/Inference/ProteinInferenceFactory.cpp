/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceFactory.cpp    | Protein Inference Factory       |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 23:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinInferenceFactory.h"
#include "ProteinInference.h"
#include "../Perception/ProteinPerception.h"
#include "../Perception/ResiduePerception.h"
#include "../Perception/ResiduePairPerception.h"
#include <lbcpp/Data/Perception.h>
#include <lbcpp/Inference/DecoratorInference.h>
using namespace lbcpp;

#include <lbcpp/Inference/ParallelInference.h>
# include "../../../src/Inference/ReductionInference/SharedParallelVectorInference.h"

class ProteinVectorTargetInference : public SharedParallelVectorInference
{
public:
  ProteinVectorTargetInference(const String& targetName, PerceptionPtr perception, InferencePtr elementInference)
    : SharedParallelVectorInference(targetName, perception, elementInference)
    {}

  ProteinVectorTargetInference() {}

  virtual size_t getOutputSize(const Variable& input) const
    {return input.getObjectAndCast<Protein>()->getLength();}
};

//////////////////////////////

ProteinInferenceFactory::ProteinInferenceFactory()
  : proteinClass(lbcpp::proteinClass())
{
  LBCPP_DECLARE_ABSTRACT_CLASS(SharedParallelVectorInference, SharedParallelInference);
    LBCPP_DECLARE_CLASS(ProteinVectorTargetInference, SharedParallelVectorInference);
}

InferencePtr ProteinInferenceFactory::createInferenceStep(const String& targetName) const
  {return new ProteinInferenceStep(targetName, createTargetInference(targetName));}

InferencePtr ProteinInferenceFactory::createTargetInference(const String& targetName) const
{
  if (targetName == T("secondaryStructure") || targetName == T("dsspSecondaryStructure")
        || targetName == T("structuralAlphabetSequence"))
    return createLabelSequenceInference(targetName);
  if (targetName == T("disorderRegions") || targetName == T("solventAccesibilityAt20p"))
    return createProbabilitySequenceInference(targetName);
  jassert(false);
  return InferencePtr();
}

InferencePtr ProteinInferenceFactory::createLabelSequenceInference(const String& targetName) const
{
  TypePtr targetType = getTargetType(targetName);
  EnumerationPtr elementsType = targetType->getTemplateArgument(0).dynamicCast<Enumeration>();
  jassert(elementsType);
  PerceptionPtr perception = createPerception(targetName, true, false);
  InferencePtr classifier = createMultiClassClassifier(targetName, perception->getOutputType(), elementsType);
  return InferencePtr(new ProteinVectorTargetInference(targetName, perception, classifier));
}

InferencePtr ProteinInferenceFactory::createProbabilitySequenceInference(const String& targetName) const
{
  PerceptionPtr perception = createPerception(targetName, true, false);
  InferencePtr classifier = createBinaryClassifier(targetName, perception->getOutputType());
  return InferencePtr(new ProteinVectorTargetInference(targetName, perception, classifier));
}

size_t ProteinInferenceFactory::getTargetIndex(const String& targetName) const
{
  int targetIndex = proteinClass->findStaticVariable(targetName);
  jassert(targetIndex >= 0);
  return (size_t)targetIndex;
}

TypePtr ProteinInferenceFactory::getTargetType(const String& targetName) const
  {return proteinClass->getStaticVariableType(getTargetIndex(targetName));}

PerceptionPtr ProteinInferenceFactory::createLabelSequencePerception(const String& targetName) const
{
  TypePtr targetType = getTargetType(targetName);
  return applyPerceptionOnProteinVariable(targetName,
    windowPerception(targetType->getTemplateArgument(0), 15));
}

PerceptionPtr ProteinInferenceFactory::createProbabilitySequencePerception(const String& targetName) const
{
  return applyPerceptionOnProteinVariable(targetName, windowPerception(probabilityType(), 15));
}

PerceptionPtr ProteinInferenceFactory::createPositionSpecificScoringMatrixPerception() const
{
  TypePtr pssmRowType = discreteProbabilityDistributionClass(aminoAcidTypeEnumeration());

  PerceptionPtr pssmRowPerception = identityPerception(pssmRowType);
  return applyPerceptionOnProteinVariable(T("positionSpecificScoringMatrix"),
    windowPerception(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration()), 15, pssmRowPerception));
}

PerceptionPtr ProteinInferenceFactory::createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const
{
  if (is1DTarget)
  {
    CompositePerceptionPtr res = new ResidueCompositePerception();
    res->addPerception(T("AA"), createLabelSequencePerception(T("primaryStructure")));
    res->addPerception(T("PSSM"), createPositionSpecificScoringMatrixPerception());
    res->addPerception(T("SS3"), createLabelSequencePerception(T("secondaryStructure")));
    res->addPerception(T("SS8"), createLabelSequencePerception(T("dsspSecondaryStructure")));
    res->addPerception(T("SA20"), createProbabilitySequencePerception(T("solventAccesibilityAt20p")));
    res->addPerception(T("DR"), createProbabilitySequencePerception(T("disorderRegions")));
    res->addPerception(T("StAl"), createLabelSequencePerception(T("structuralAlphabetSequence")));
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
