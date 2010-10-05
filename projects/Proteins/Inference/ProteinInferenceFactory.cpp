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
  TypePtr elementsType = getTargetType(targetName)->getTemplateArgument(0);
  CompositePerceptionPtr res = new ResidueCompositePerception();
  addPerception(res, T("window"), targetName, windowPerception(elementsType, 15));
  addPerception(res, T("histogram"), targetName, windowHistogramPerception(elementsType, 15));
  return res;
}

PerceptionPtr ProteinInferenceFactory::createProbabilitySequencePerception(const String& targetName) const
{
  CompositePerceptionPtr res = new ResidueCompositePerception();
  addPerception(res, T("window"), targetName, windowPerception(probabilityType(), 15));
  addPerception(res, T("histogram"), targetName, windowHistogramPerception(probabilityType(), 15));
  return res;
}

PerceptionPtr ProteinInferenceFactory::createPositionSpecificScoringMatrixPerception() const
{
  PerceptionPtr pssmRowPerception = discreteProbabilityDistributionPerception(aminoAcidTypeEnumeration());
  ClassPtr aaDistributionClass = discreteProbabilityDistributionClass(aminoAcidTypeEnumeration());

  String targetName(T("positionSpecificScoringMatrix"));
  CompositePerceptionPtr res = new ResidueCompositePerception();
  addPerception(res, T("window"), targetName, windowPerception(aaDistributionClass, 15, pssmRowPerception));
  addPerception(res, T("histogram"), targetName, windowHistogramPerception(aaDistributionClass, 15));
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
  res->addPerception(T("length"), proteinLengthPerception());

  CompositePerceptionPtr freq = new ProteinCompositePerception();
  static const juce::tchar* histogramTargets[] = {
    T("primaryStructure"), T("positionSpecificScoringMatrix"), T("secondaryStructure"),
    T("dsspSecondaryStructure"), T("solventAccessibilityAt20p"), T("disorderRegions"),
    T("structuralAlphabetSequence")
  };
  for (size_t i = 0; i < sizeof (histogramTargets) / sizeof (const juce::tchar* ); ++i)
  {
    int index = getTargetIndex(histogramTargets[i]); jassert(index >= 0);
    String name = Protein::getTargetShortName((size_t)index);
    TypePtr elementsType = proteinClass->getObjectVariableType((size_t)index)->getTemplateArgument(0); jassert(elementsType);
    addPerception(freq, name, histogramTargets[i], containerHistogramPerception(elementsType));
  }
  res->addPerception(T("histograms"), freq);
  return res;
}

PerceptionPtr ProteinInferenceFactory::createResiduePerception(const String& targetName) const
{
  CompositePerceptionPtr res = new ResidueCompositePerception();
  res->addPerception(T("protein"), createProteinPerception());
  addPerception(res, T("position"), T("primaryStructure"), boundsProximityPerception());
  res->addPerception(T("aa"), createLabelSequencePerception(T("primaryStructure")));
  res->addPerception(T("pssm"), createPositionSpecificScoringMatrixPerception());
  res->addPerception(T("ss3"), createLabelSequencePerception(T("secondaryStructure")));
  res->addPerception(T("ss8"), createLabelSequencePerception(T("dsspSecondaryStructure")));
  res->addPerception(T("sa20"), createProbabilitySequencePerception(T("solventAccessibilityAt20p")));
  res->addPerception(T("dr"), createProbabilitySequencePerception(T("disorderRegions")));
  res->addPerception(T("stal"), createLabelSequencePerception(T("structuralAlphabetSequence")));
  //res->addPerception(T("PairOfSeq"), createPairsSequencesPerception());
  return res;
}

PerceptionPtr ProteinInferenceFactory::createResiduePairPerception(const String& targetName) const
{
  PerceptionPtr residuePerception = createResiduePerception(targetName);

  CompositePerceptionPtr res = new ResiduePairCompositePerception();
  res->addPerception(T("protein"), createProteinPerception());
  res->addPerception(T("residues"), residuePerception);
  res->addPerception(T("separationDistance"), separationDistanceResiduePairPerception());

  CompositePerceptionPtr freq = new ResiduePairCompositePerception();
  static const juce::tchar* histogramTargets[] = {
    T("primaryStructure"), T("positionSpecificScoringMatrix"), T("secondaryStructure"),
    T("dsspSecondaryStructure"), T("solventAccessibilityAt20p"), T("disorderRegions"),
    T("structuralAlphabetSequence")
  };
  for (size_t i = 0; i < sizeof (histogramTargets) / sizeof (const juce::tchar* ); ++i)
  {
    int index = getTargetIndex(histogramTargets[i]); jassert(index >= 0);
    String name = Protein::getTargetShortName((size_t)index);
    TypePtr elementsType = proteinClass->getObjectVariableType((size_t)index)->getTemplateArgument(0); jassert(elementsType);
    addPerception(freq, name, histogramTargets[i], histogramPerception(elementsType));
  }
  res->addPerception(T("centralHistograms"), freq);

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

void ProteinInferenceFactory::addPerception(CompositePerceptionPtr composite, const String& name, const String& targetName, PerceptionPtr perception) const
{
  FunctionPtr selectFunction;
  int index = proteinClass->findObjectVariable(targetName);
  if (perception->getInputType()->inheritsFrom(pairType(anyType(), anyType())))
    selectFunction = selectPairVariablesFunction(index);
  else
    selectFunction = selectVariableFunction(index);

  composite->addPerception(name, Perception::compose(selectFunction, perception));
}
