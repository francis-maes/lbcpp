/*-----------------------------------------.---------------------------------.
| Filename: ProteinResidueFrame.cpp        | Protein Residue Frame           |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 14:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinResidueFrame.h"
using namespace lbcpp;

FrameClassPtr lbcpp::defaultProteinSingleResidueFrameClass(ExecutionContext& context)
{
  FrameClassPtr res(new FrameClass(T("ProteinResidueFrame"), objectClass));

  // inputs
  size_t aaIndex = res->addMemberVariable(context, aminoAcidTypeEnumeration, T("aminoAcid"));
  size_t pssmIndex = res->addMemberVariable(context, enumerationDistributionClass(aminoAcidTypeEnumeration), T("pssmRow"));
  size_t ss3Index = res->addMemberVariable(context, enumerationDistributionClass(secondaryStructureElementEnumeration), T("ss3"));

  // feature generators
  res->addMemberOperator(context, enumerationFeatureGenerator(), aaIndex, T("aminoAcidFeatures"));
  res->addMemberOperator(context, enumerationDistributionFeatureGenerator(), pssmIndex, T("pssmFeatures"));
  res->addMemberOperator(context, enumerationDistributionFeatureGenerator(), ss3Index, T("ss3Features"));

  return res->initialize(context) ? res : FrameClassPtr();
}

VectorPtr lbcpp::createProteinSingleResidueFrames(ExecutionContext& context, const FramePtr& proteinFrame, FrameClassPtr residueFrameClass)
{
  VectorPtr primaryStructure = proteinFrame->getVariable(0).getObjectAndCast<Vector>();
  VectorPtr pssm = proteinFrame->getVariable(1).getObjectAndCast<Vector>();
  jassert(primaryStructure && pssm);

  size_t n = primaryStructure->getNumElements();
  VectorPtr res = vector(residueFrameClass, n);

  VectorPtr ss3 = proteinFrame->getVariable(2).getObjectAndCast<Vector>();

  for (size_t i = 0; i < n; ++i)
  {
    FramePtr residueFrame(new Frame(residueFrameClass));
    residueFrame->setVariable(context, 0, primaryStructure->getElement(i));
    residueFrame->setVariable(context, 1, pssm->getElement(i));
    if (ss3)
      residueFrame->setVariable(context, 2, ss3->getElement(i));
    res->setElement(i, residueFrame);
  }
  return res;
}
