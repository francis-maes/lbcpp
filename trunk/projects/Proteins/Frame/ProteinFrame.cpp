/*-----------------------------------------.---------------------------------.
| Filename: ProteinFrame.cpp               | Protein Frame                   |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinFrame.h"
#include "ProteinResidueFrame.h"
using namespace lbcpp;

FrameClassPtr ProteinFrameFactory::createProteinFrameClass(ExecutionContext& context)
{
  FrameClassPtr res = new FrameClass(T("ProteinFrame"), objectClass);
  proteinFrame = res;
  
  // inputs
  size_t lengthIndex = res->addMemberVariable(context, positiveIntegerType, T("length"));
  size_t aaIndex = res->addMemberVariable(context, vectorClass(aminoAcidTypeEnumeration), T("primaryStructure"), T("AA"));
  size_t pssmIndex = res->addMemberVariable(context, vectorClass(enumerationDistributionClass(aminoAcidTypeEnumeration)), T("positionSpecificScoringMatrix"), T("PSSM"));
  size_t ss3Index = res->addMemberVariable(context, vectorClass(enumerationDistributionClass(secondaryStructureElementEnumeration)), T("secondaryStructure"), T("SS3"));

  // primary residue features
  FrameClassPtr primaryResidueFrameClass = createPrimaryResidueFrameClass(context, res);
  if (!primaryResidueFrameClass)
    return FrameClassPtr();
  size_t contextFreeResidueFeatures = res->addMemberOperator(context, generateVectorFunction(new FrameBasedFunction(primaryResidueFrameClass)), (size_t)-1, lengthIndex, T("primaryResidueFeatures"));
  size_t contextFreeResidueFeaturesSum = res->addMemberOperator(context, accumulateContainerFunction(), contextFreeResidueFeatures, T("primaryResidueFeaturesAcc"));
  
  // global features
  FrameClassPtr proteinGlobalFrameClass = createProteinGlobalFrameClass(context, res);
  if (!proteinGlobalFrameClass)
    return FrameClassPtr();
  res->addMemberOperator(context, new FrameBasedFunction(proteinGlobalFrameClass), (size_t)-1, T("globalFeatures"));

  // residue features
  FrameClassPtr residueFrameClass = createResidueFrameClass(context, res);
  if (!residueFrameClass)
    return FrameClassPtr();
  res->addMemberOperator(context, generateVectorFunction(new FrameBasedFunction(residueFrameClass)), (size_t)-1, lengthIndex, T("residueFeatures"));

  return res->initialize(context) ? res : FrameClassPtr();
}

FrameClassPtr ProteinFrameFactory::createPrimaryResidueFrameClass(ExecutionContext& context, const FrameClassPtr& proteinFrameClass)
{
  FrameClassPtr res = new FrameClass(T("PrimaryResidueFrame"), objectClass);
  
  // inputs
  size_t proteinFrameIndex = res->addMemberVariable(context, proteinFrameClass, T("proteinFrame"));
  size_t positionIndex = res->addMemberVariable(context, positiveIntegerType, T("position"));

  // retrieve the amino acid, the pssm row and the ss3 prediction
  size_t aaIndex = res->addMemberOperator(context, getVariableFunction(T("primaryStructure")), proteinFrameIndex);
  aaIndex = res->addMemberOperator(context, getElementFunction(), aaIndex, positionIndex, T("aa"));

  size_t pssmIndex = res->addMemberOperator(context, getVariableFunction(T("positionSpecificScoringMatrix")), proteinFrameIndex);
  pssmIndex = res->addMemberOperator(context, getElementFunction(), pssmIndex, positionIndex, T("pssm"));

  size_t ss3Index = res->addMemberOperator(context, getVariableFunction(T("secondaryStructure")), proteinFrameIndex);
  ss3Index = res->addMemberOperator(context, getElementFunction(), ss3Index, positionIndex, T("ss3"));

  // feature generators
  std::vector<size_t> featureIndices;

  featureIndices.push_back(res->addMemberOperator(context, enumerationFeatureGenerator(), aaIndex));
  featureIndices.push_back(res->addMemberOperator(context, enumerationDistributionFeatureGenerator(), pssmIndex));
  size_t pssmEntropyIndex = res->addMemberOperator(context, distributionEntropyFunction(), pssmIndex);
  featureIndices.push_back(res->addMemberOperator(context, defaultPositiveDoubleFeatureGenerator(10, -1.0, 4.0), pssmEntropyIndex));
  featureIndices.push_back(res->addMemberOperator(context, enumerationDistributionFeatureGenerator(), ss3Index));
  size_t ss3EntropyIndex = res->addMemberOperator(context, distributionEntropyFunction(), ss3Index);
  featureIndices.push_back(res->addMemberOperator(context, defaultPositiveDoubleFeatureGenerator(10, -1.0, 4.0), ss3EntropyIndex));

  // all features
  res->addMemberOperator(context, concatenateFeatureGenerator(false), featureIndices, T("primaryResidueFeatures"));

  return res->initialize(context) ? res : FrameClassPtr();
}

FrameClassPtr ProteinFrameFactory::createResidueFrameClass(ExecutionContext& context, const FrameClassPtr& proteinFrameClass)
{
  FrameClassPtr res = new FrameClass(T("ResidueFrame"), objectClass);

  size_t proteinFrameIndex = res->addMemberVariable(context, proteinFrameClass, T("proteinFrame"));
  size_t positionIndex = res->addMemberVariable(context, positiveIntegerType, T("position"));
  
  size_t primaryResidueFeaturesIndex = res->addMemberOperator(context, getVariableFunction(T("primaryResidueFeatures")), proteinFrameIndex);
  size_t primaryResidueFeaturesAccIndex = res->addMemberOperator(context, getVariableFunction(T("primaryResidueFeaturesAcc")), proteinFrameIndex);

  std::vector<size_t> featureIndices;
  featureIndices.push_back(res->addMemberOperator(context, windowFeatureGenerator(15), primaryResidueFeaturesIndex, positionIndex));
  featureIndices.push_back(res->addMemberOperator(context, accumulatorLocalMeanFunction(10), primaryResidueFeaturesAccIndex, positionIndex, T("localMean10")));
  featureIndices.push_back(res->addMemberOperator(context, accumulatorLocalMeanFunction(50), primaryResidueFeaturesAccIndex, positionIndex, T("localMean50")));
  featureIndices.push_back(res->addMemberOperator(context, getVariableFunction(T("globalFeatures")), proteinFrameIndex, T("global")));
  
  // all features
  res->addMemberOperator(context, concatenateFeatureGenerator(true), featureIndices, T("AllFeatures")); 

  return res->initialize(context) ? res : FrameClassPtr();
}

FrameClassPtr ProteinFrameFactory::createProteinGlobalFrameClass(ExecutionContext& context, const FrameClassPtr& proteinFrameClass)
{
  FrameClassPtr res = new FrameClass(T("ProteinGlobal"), objectClass);

  size_t proteinFrameIndex = res->addMemberVariable(context, proteinFrameClass, T("proteinFrame"));
  size_t primaryResidueFeaturesAccIndex = res->addMemberOperator(context, getVariableFunction(T("primaryResidueFeaturesAcc")), proteinFrameIndex);
  res->addMemberOperator(context, accumulatorGlobalMeanFunction(), primaryResidueFeaturesAccIndex);

  return res->initialize(context) ? res : FrameClassPtr();
}

FramePtr ProteinFrameFactory::createFrame(const ProteinPtr& protein) const
{
  FramePtr res(new Frame(proteinFrame));
  res->setVariable(0, protein->getLength());
  res->setVariable(1, protein->getPrimaryStructure());
  res->setVariable(2, protein->getPositionSpecificScoringMatrix());
  
  ContainerPtr inputSecondaryStructure = protein->getSecondaryStructure();
  if (inputSecondaryStructure)
  {
    size_t n = inputSecondaryStructure->getNumElements();
    VectorPtr secondaryStructure = vector(enumerationDistributionClass(secondaryStructureElementEnumeration), n);
    for (size_t i = 0; i < n; ++i)
    {
      EnumerationDistributionPtr distribution = new EnumerationDistribution(secondaryStructureElementEnumeration);
      distribution->setProbability((size_t)inputSecondaryStructure->getElement(i).getInteger(), 1.0);
      secondaryStructure->setElement(i, distribution);
    }
    res->setVariable(3, secondaryStructure);
  }
  return res;
}
