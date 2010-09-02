/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceFactory.cpp    | Protein Inference Factory       |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 23:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinInferenceFactory.h"
#include "ProteinInference.h"
#include "ContactMapInference.h"
#include "../Perception/ProteinPerception.h"
#include <lbcpp/Function/Perception.h>
#include <lbcpp/Inference/DecoratorInference.h>
using namespace lbcpp;

ProteinInferenceFactory::ProteinInferenceFactory()
  : proteinClass(lbcpp::proteinClass())
  {}

InferencePtr ProteinInferenceFactory::createInferenceStep(const String& targetName) const
  {return new ProteinInferenceStep(targetName, createTargetInference(targetName));}

InferencePtr ProteinInferenceFactory::createTargetInference(const String& targetName) const
{
  if (targetName == T("secondaryStructure") || targetName == T("dsspSecondaryStructure")
        || targetName == T("structuralAlphabetSequence"))
    return createLabelSequenceInference(targetName);
  if (targetName == T("disorderRegions") || targetName == T("solventAccessibilityAt20p"))
    return createProbabilitySequenceInference(targetName);
  if (targetName.startsWith(T("contactMap")))
    return createContactMapInference(targetName);
  jassert(false);
  return InferencePtr();
}

InferencePtr ProteinInferenceFactory::createLabelSequenceInference(const String& targetName) const
{
  TypePtr targetType = getTargetType(targetName);
  EnumerationPtr elementsType = targetType->getTemplateArgument(0).dynamicCast<Enumeration>();
  jassert(elementsType);
  PerceptionPtr perception = createPerception(targetName, true, false);
  InferencePtr classifier = createMultiClassClassifier(targetName, perception, elementsType);
  return sharedParallelVectorInference(targetName, proteinLengthFunction(), classifier);
}

InferencePtr ProteinInferenceFactory::createProbabilitySequenceInference(const String& targetName) const
{
  PerceptionPtr perception = createPerception(targetName, true, false);
  InferencePtr classifier = createBinaryClassifier(targetName, perception);
  return sharedParallelVectorInference(targetName, proteinLengthFunction(), classifier);
}

InferencePtr ProteinInferenceFactory::createContactMapInference(const String& targetName) const
{
  PerceptionPtr perception = createPerception(targetName, false, true);
  InferencePtr classifier = createBinaryClassifier(targetName, perception);
  return new ContactMapInference(targetName, classifier);
}

size_t ProteinInferenceFactory::getTargetIndex(const String& targetName) const
{
  int targetIndex = proteinClass->findObjectVariable(targetName);
  jassert(targetIndex >= 0);
  return (size_t)targetIndex;
}

TypePtr ProteinInferenceFactory::getTargetType(const String& targetName) const
  {return proteinClass->getObjectVariableType(getTargetIndex(targetName));}

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

PerceptionPtr ProteinInferenceFactory::createProteinPerception() const
{
  CompositePerceptionPtr res = new ProteinCompositePerception();
  res->addPerception(T("LEN"), proteinLengthPerception());
  return res;
}

PerceptionPtr ProteinInferenceFactory::createResiduePerception(const String& targetName) const
{
  CompositePerceptionPtr res = new ResidueCompositePerception();
  res->addPerception(T("GLOBAL"), createProteinPerception());
  res->addPerception(T("POSITION"), positionResiduePerception());
  res->addPerception(T("INDEX"), indexResiduePerception());
  res->addPerception(T("AA"), createLabelSequencePerception(T("primaryStructure")));
  res->addPerception(T("PSSM"), createPositionSpecificScoringMatrixPerception());
  res->addPerception(T("SS3"), createLabelSequencePerception(T("secondaryStructure")));
  res->addPerception(T("SS8"), createLabelSequencePerception(T("dsspSecondaryStructure")));
  res->addPerception(T("SA20"), createProbabilitySequencePerception(T("solventAccessibilityAt20p")));
  res->addPerception(T("DR"), createProbabilitySequencePerception(T("disorderRegions")));
  res->addPerception(T("StAl"), createLabelSequencePerception(T("structuralAlphabetSequence")));
  return res;
}

PerceptionPtr ProteinInferenceFactory::createResiduePairPerception(const String& targetName) const
{
  PerceptionPtr residuePerception = createResiduePerception(targetName);

  CompositePerceptionPtr res = new ResiduePairCompositePerception();
  res->addPerception(T("GLOBAL"), createProteinPerception());
  res->addPerception(T("PT"), residuePerception);
  res->addPerception(T("SEP"), separationDistanceResiduePairPerception());
  return res;
}

PerceptionPtr ProteinInferenceFactory::createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const
{
  if (is1DTarget)
    return createResiduePerception(targetName);
  else if (is2DTarget)
    return createResiduePairPerception(targetName);
  jassert(false);
  return PerceptionPtr();
}

PerceptionPtr ProteinInferenceFactory::applyPerceptionOnProteinVariable(const String& variableName, PerceptionPtr variablePerception) const
{
   FunctionPtr selectVariableFunction = selectPairFieldsFunction(proteinClass->findObjectVariable(variableName));
   return Perception::compose(selectVariableFunction, variablePerception);
}
