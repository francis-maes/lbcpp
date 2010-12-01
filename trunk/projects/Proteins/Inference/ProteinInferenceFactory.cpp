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
#include "DisulfideBondsInference.h"
#include "../Perception/ProteinPerception.h"
#include <lbcpp/Perception/Perception.h>
#include <lbcpp/Inference/DecoratorInference.h>
#include <lbcpp/ProbabilityDistribution/DiscreteProbabilityDistribution.h>

using namespace lbcpp;

ProteinInferenceFactory::ProteinInferenceFactory(ExecutionContext& context)
  : context(context)
{
}

ProteinInferenceFactory::~ProteinInferenceFactory()
  {if (perceptionRewriter) perceptionRewriter->clearCache();}

InferencePtr ProteinInferenceFactory::createInferenceStep(const String& targetName) const
{
  InferencePtr res = new ProteinInferenceStep(targetName, createTargetInference(targetName));
  res->getBatchLearner()->setPushIntoStackFlag(true);
  return res;
}

InferencePtr ProteinInferenceFactory::createTargetInference(const String& targetName) const
{
  if (targetName == T("secondaryStructure") || targetName == T("dsspSecondaryStructure")
        || targetName == T("structuralAlphabetSequence"))
    return createLabelSequenceInference(targetName);
  if (targetName == T("disorderRegions") || targetName == T("solventAccessibilityAt20p"))
    return createProbabilitySequenceInference(targetName);
  if (targetName.startsWith(T("contactMap")))
    return createContactMapInference(targetName);
  if (targetName == T("disulfideBonds"))
    return createDisulfideBondsInference(targetName);
  jassert(false);
  return InferencePtr();
}

InferencePtr ProteinInferenceFactory::createLabelSequenceInference(const String& targetName) const
{
  TypePtr targetType = getTargetType(targetName);
  EnumerationPtr elementsType = targetType->getTemplateArgument(0).dynamicCast<Enumeration>();
  jassert(elementsType);
  PerceptionPtr perception = createPerception(targetName, residuePerception);
  InferencePtr classifier = createMultiClassClassifier(targetName, perception, elementsType);
  return sharedParallelVectorInference(targetName, proteinLengthFunction(), classifier);
}

InferencePtr ProteinInferenceFactory::createProbabilitySequenceInference(const String& targetName) const
{
  PerceptionPtr perception = createPerception(targetName, residuePerception);
  InferencePtr classifier = createBinaryClassifier(targetName, perception);
  return sharedParallelVectorInference(targetName, proteinLengthFunction(), classifier);
}

InferencePtr ProteinInferenceFactory::createContactMapInference(const String& targetName) const
{
  PerceptionPtr perception = createPerception(targetName, residuePairPerception);
  InferencePtr classifier = createBinaryClassifier(targetName, perception);
  return new ContactMapInference(targetName, classifier);
}

InferencePtr ProteinInferenceFactory::createDisulfideBondsInference(const String& targetName) const
{
  PerceptionPtr perception = createPerception(targetName, residuePairPerception);
  InferencePtr classifier = createBinaryClassifier(targetName, perception);
  return new DisulfideBondsInference(targetName, classifier);
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
  addPerception(res, T("window"), targetName, windowPerception(probabilityType, 15));
  addPerception(res, T("histogram"), targetName, windowHistogramPerception(probabilityType, 15));
  return res;
}

PerceptionPtr ProteinInferenceFactory::createPositionSpecificScoringMatrixPerception() const
{
  PerceptionPtr pssmRowPerception = discreteProbabilityDistributionPerception(aminoAcidTypeEnumeration);
  ClassPtr aaDistributionClass = enumerationProbabilityDistributionClass(aminoAcidTypeEnumeration);

  String targetName(T("positionSpecificScoringMatrix"));
  CompositePerceptionPtr res = new ResidueCompositePerception();
  addPerception(res, T("window"), targetName, windowPerception(aaDistributionClass, 15, pssmRowPerception));
  addPerception(res, T("histogram"), targetName, windowHistogramPerception(aaDistributionClass, 15));
  return res;
}

PerceptionPtr ProteinInferenceFactory::createProteinPerception(const String& targetName) const
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
    int index = (int)getTargetIndex(histogramTargets[i]); jassert(index >= 0);
    String name = proteinClass->getObjectVariableShortName((size_t)index);
    TypePtr elementsType = proteinClass->getObjectVariableType((size_t)index)->getTemplateArgument(0); jassert(elementsType);
    addPerception(freq, name, histogramTargets[i], containerHistogramPerception(elementsType));
  }
  res->addPerception(T("histograms"), freq);
  return res;
}

PerceptionPtr ProteinInferenceFactory::createResiduePerception(const String& targetName) const
{
  CompositePerceptionPtr res = new ResidueCompositePerception();
  if (!targetName.startsWith(T("contactMap")) && targetName != T("disulfideBonds"))
    res->addPerception(T("protein"), createProteinPerception(targetName));
  addPerception(res, T("position"), T("primaryStructure"), boundsProximityPerception());
  res->addPerception(T("aa"), createLabelSequencePerception(T("primaryStructure")));
  res->addPerception(T("pssm"), createPositionSpecificScoringMatrixPerception());
  res->addPerception(T("ss3"), createLabelSequencePerception(T("secondaryStructure")));
  res->addPerception(T("ss8"), createLabelSequencePerception(T("dsspSecondaryStructure")));
  res->addPerception(T("sa20"), createProbabilitySequencePerception(T("solventAccessibilityAt20p")));
  res->addPerception(T("dr"), createProbabilitySequencePerception(T("disorderRegions")));
  res->addPerception(T("stal"), createLabelSequencePerception(T("structuralAlphabetSequence")));
  res->addPerception(T("dsb"), disulfideBondsResiduePerception());
  return res;
}

PerceptionPtr ProteinInferenceFactory::createResiduePairPerception(const String& targetName) const
{
  PerceptionPtr residuePerception = createResiduePerception(targetName);

  CompositePerceptionPtr res = new ResiduePairCompositePerception();
  res->addPerception(T("protein"), createProteinPerception(targetName));
  res->addPerception(T("residues"), residuePerception);
  res->addPerception(T("separationDistance"), separationDistanceResiduePairPerception());
  res->addPerception(T("disulfideBond"), disulfideBondsResiduePairPerception());

  CompositePerceptionPtr freq = new ResiduePairCompositePerception();
  static const juce::tchar* histogramTargets[] = {
    T("primaryStructure"), T("positionSpecificScoringMatrix"), T("secondaryStructure"),
    T("dsspSecondaryStructure"), T("solventAccessibilityAt20p"), T("disorderRegions"),
    T("structuralAlphabetSequence")
  };
  for (size_t i = 0; i < sizeof (histogramTargets) / sizeof (const juce::tchar* ); ++i)
  {
    int index = (int)getTargetIndex(histogramTargets[i]); jassert(index >= 0);
    String name = proteinClass->getObjectVariableShortName((size_t)index);
    TypePtr elementsType = proteinClass->getObjectVariableType((size_t)index)->getTemplateArgument(0); jassert(elementsType);
    addPerception(freq, name, histogramTargets[i], segmentHistogramPerception(elementsType));
  }
  res->addPerception(T("centralHistograms"), freq);

  return res;
}

PerceptionPtr ProteinInferenceFactory::createPerception(const String& targetName, PerceptionType type) const
{
  PerceptionPtr res;
  if (type == proteinPerception)
    res = createProteinPerception(targetName);
  else if (type == residuePerception)
    res = createResiduePerception(targetName);
  else if (type == residuePairPerception)
    res = createResiduePairPerception(targetName);
  else
    {jassert(false); return PerceptionPtr();}

  if (!perceptionRewriter)
  {
    PerceptionRewriterPtr& rewriter = const_cast<ProteinInferenceFactory* >(this)->perceptionRewriter;
    rewriter = new PerceptionRewriter(true);
    getPerceptionRewriteRules(rewriter);
  }
  return perceptionRewriter->getNumRules() ? perceptionRewriter->rewrite(res) : res;
}

void ProteinInferenceFactory::addPerception(CompositePerceptionPtr composite, const String& name, const String& targetName, PerceptionPtr perception) const
{
  FunctionPtr selectFunction;
  int index = proteinClass->findObjectVariable(targetName);
  TypePtr inputType = perception->getInputType();
  if (inputType->inheritsFrom(pairClass(anyType, anyType)))
    selectFunction = selectPairVariablesFunction(index, -1, pairClass(proteinClass, inputType->getTemplateArgument(1)));
  else
    selectFunction = selectVariableFunction(index);

  composite->addPerception(name, composePerception(selectFunction, perception));
}
