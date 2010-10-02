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
  CompositePerceptionPtr res = new ResidueCompositePerception();
  res->addPerception(T("WINDOW"), applyPerceptionOnProteinVariable(targetName, windowPerception(targetType->getTemplateArgument(0), 15)));
  res->addPerception(T("HISTOGRAM"), applyWindowOnPerception(targetName, 15, histogramPerception(targetType->getTemplateArgument(0))));
  return res;
}

PerceptionPtr ProteinInferenceFactory::createProbabilitySequencePerception(const String& targetName) const
{
  CompositePerceptionPtr res = new ResidueCompositePerception();
  res->addPerception(T("WINDOW"), applyPerceptionOnProteinVariable(targetName, windowPerception(probabilityType(), 15)));
  res->addPerception(T("HISTOGRAM"), applyWindowOnPerception(targetName, 15, histogramPerception(probabilityType())));
  return res;
}

PerceptionPtr ProteinInferenceFactory::createPositionSpecificScoringMatrixPerception() const
{
  TypePtr pssmRowType = discreteProbabilityDistributionClass(aminoAcidTypeEnumeration());

  PerceptionPtr pssmRowPerception = identityPerception(pssmRowType);
  
  CompositePerceptionPtr res = new ResidueCompositePerception();
  res->addPerception(T("WINDOW"), applyPerceptionOnProteinVariable(T("positionSpecificScoringMatrix"),
                                                                   windowPerception(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration()), 15, pssmRowPerception)));
  res->addPerception(T("HISTOGRAM"), applyWindowOnPerception(T("positionSpecificScoringMatrix"), 15, histogramPerception(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration()))));
  return res;
}

PerceptionPtr ProteinInferenceFactory::createPairsSequencesPerception() const
{
  const size_t nbSequences = 6;
  const String sequences[nbSequences] = {
    T("primaryStructure"),
    T("secondaryStructure"),
    T("dsspSecondaryStructure"),
    T("solventAccessibility"),
    T("disorderRegions"),
    T("structuralAlphabetSequence")
  };
  
  const String shortNames[nbSequences] = {
    T("AA"),
    T("SS3"),
    T("SS8"),
    T("SA"),
    T("DR"),
    T("StAl")
  };
  
  CompositePerceptionPtr res = new ResidueCompositePerception();
  
  for (size_t i = 0; i < nbSequences; ++i)
    for (size_t j = i + 1; j < nbSequences; ++j)
      res->addPerception(shortNames[i] + T(" & ") + shortNames[j], createPairSequencesPerception(sequences[i], sequences[j], 5));
  return res;
}

PerceptionPtr ProteinInferenceFactory::createProteinPerception() const
{
  CompositePerceptionPtr res = new ProteinCompositePerception();
  res->addPerception(T("LEN"), proteinLengthPerception());
  CompositePerceptionPtr freq = new ProteinCompositePerception();
  freq->addPerception(T("AA"), createHistogramPerception(T("primaryStructure")));
  freq->addPerception(T("PSSM"), createHistogramPerception(T("positionSpecificScoringMatrix")));
  freq->addPerception(T("SS3"), createHistogramPerception(T("secondaryStructure")));
  freq->addPerception(T("SS8"), createHistogramPerception(T("dsspSecondaryStructure")));
  freq->addPerception(T("SA20"), createHistogramPerception(T("solventAccessibilityAt20p")));
  freq->addPerception(T("DR"), createHistogramPerception(T("disorderRegions")));
  freq->addPerception(T("StAl"), createHistogramPerception(T("structuralAlphabetSequence")));
  res->addPerception(T("HISTOGRAM"), freq);
  return res;
}

PerceptionPtr ProteinInferenceFactory::createResiduePerception(const String& targetName) const
{
  CompositePerceptionPtr res = new ResidueCompositePerception();
  res->addPerception(T("GLOBAL"), createProteinPerception());
  res->addPerception(T("POSITION"), positionResiduePerception());
//  res->addPerception(T("INDEX"), indexResiduePerception());
  res->addPerception(T("TERMINUS"), terminusProximityResiduePerception(10));
  res->addPerception(T("AA"), createLabelSequencePerception(T("primaryStructure")));
  res->addPerception(T("PSSM"), createPositionSpecificScoringMatrixPerception());
  res->addPerception(T("SS3"), createLabelSequencePerception(T("secondaryStructure")));
  res->addPerception(T("SS8"), createLabelSequencePerception(T("dsspSecondaryStructure")));
  res->addPerception(T("SA20"), createProbabilitySequencePerception(T("solventAccessibilityAt20p")));
  res->addPerception(T("DR"), createProbabilitySequencePerception(T("disorderRegions")));
  res->addPerception(T("StAl"), createLabelSequencePerception(T("structuralAlphabetSequence")));
  res->addPerception(T("PairOfSeq"), createPairsSequencesPerception());
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
  PerceptionPtr res;
  if (is1DTarget)
    res = createResiduePerception(targetName);
  else if (is2DTarget)
    res = createResiduePairPerception(targetName);
  else
    {jassert(false); return PerceptionPtr();}

  if (!perceptionRewriter)
  {
    PerceptionRewriterPtr& rewriter = const_cast<ProteinInferenceFactory* >(this)->perceptionRewriter;
    rewriter = new PerceptionRewriter();
    getPerceptionRewriteRules(rewriter);
  }
  return perceptionRewriter->getNumRules() ? perceptionRewriter->rewrite(res) : res;
}

PerceptionPtr ProteinInferenceFactory::applyPerceptionOnProteinVariable(const String& variableName, PerceptionPtr variablePerception) const
{
   FunctionPtr selectVariableFunction = selectPairFieldsFunction(proteinClass->findObjectVariable(variableName));
   return Perception::compose(selectVariableFunction, variablePerception);
}

PerceptionPtr ProteinInferenceFactory::applyWindowOnPerception(const String& variableName, size_t windowSize, PerceptionPtr perception) const
{
  FunctionPtr windowFunction = windowToIndicesFunction(windowSize);
  return applyPerceptionOnProteinVariable(variableName, Perception::compose(windowFunction, perception));
}

PerceptionPtr ProteinInferenceFactory::applyPerceptionOnEntireProteinVariable(const String& variableName, PerceptionPtr perception) const
{
  FunctionPtr variableFunction = proteinToVariableFunction(proteinClass->findObjectVariable(variableName));
  return Perception::compose(variableFunction, Perception::compose(variableToIndicesFunction(), perception));
}
                      
PerceptionPtr ProteinInferenceFactory::createHistogramPerception(const String& targetName) const
{
  TypePtr targetType = getTargetType(targetName);
  return applyPerceptionOnEntireProteinVariable(targetName, histogramPerception(targetType->getTemplateArgument(0)));
}

PerceptionPtr ProteinInferenceFactory::createPairSequencesPerception(const String& firstTargetName, const String& secondTargetName, size_t windowSize) const
{
  int index1 = proteinClass->findObjectVariable(firstTargetName);
  int index2 = proteinClass->findObjectVariable(secondTargetName);
  jassert(index1 != -1 && index2 != -1);
  jassert(proteinClass->getObjectVariableType(index1)->inheritsFrom(containerClass(anyType())));
  jassert(proteinClass->getObjectVariableType(index2)->inheritsFrom(containerClass(anyType())));

  return Perception::compose(
    residueToSelectPairSequencesFunction(index1, index2),
    biContainerPerception(windowSize, biVariablePerception(proteinClass->getObjectVariableType(index1)->getTemplateArgument(0),
                                                           proteinClass->getObjectVariableType(index2)->getTemplateArgument(0)))
  );
}
