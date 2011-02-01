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

/*
namespace lbcpp
{

class ProteinFrameClass : public FrameClass
{
public:
  virtual bool initialize(ExecutionContext& context)
  {
    addMemberVariable(context, T("GenericVector<AminoAcidType>"), T("aa"));
    addMemberVariable(context, T("ObjectVector<EnumerationDistribution<AminoAcidType>>"), T("pssm"));

    //addFunctionAndVariable(context, machinFunction(), std::vector<size_t>(1, 0), T("aac"));
    return FrameClass::initialize(context);
  }
};

ClassPtr proteinFrameClass = new ProteinFrameClass();

}; /* namespace lbcpp *

*/

FrameClassPtr defaultProteinFrameClass(ExecutionContext& context)
{
  FrameClassPtr res(new FrameClass(T("ProteinFrame"), objectClass));

  // primaryStructure
  size_t aaIndex = res->addMemberVariable(context, vectorClass(aminoAcidTypeEnumeration), T("primaryStructure"), T("AA"));
  res->addMemberOperator(context, accumulateOperator(), aaIndex, T("primaryStructureAccumulator"), T("AAc"));

  // pssm
  size_t pssmIndex = res->addMemberVariable(context, vectorClass(enumerationDistributionClass(aminoAcidTypeEnumeration)), T("positionSpecificScoringMatrix"), T("PSSM"));
  res->addMemberOperator(context, accumulateOperator(), pssmIndex, T("positionSpecificScoringMatrixAccumulator"), T("PSSMc"));

  // secondary structure
  size_t ss3Index = res->addMemberVariable(context, vectorClass(enumerationDistributionClass(secondaryStructureElementEnumeration)), T("secondaryStructure"), T("SS3"));
  size_t ss3LabelsIndex = res->addMemberOperator(context, discretizeOperator(), ss3Index, T("secondaryStructureLabels"), T("SS3d"));
  res->addMemberOperator(context, segmentContainerOperator(), ss3LabelsIndex, T("secondaryStructureSegments"), T("SS3ds"));

  return res->initialize(context) ? res : FrameClassPtr();
}

ProteinFrame::ProteinFrame(const ProteinPtr& protein)
  //: Object(proteinFrameClass)
{
  // primary Structure
  primaryStructure = protein->getPrimaryStructure();
  size_t n = primaryStructure->getNumElements();
/*
  FunctionPtr aaAccumulator = accumulateOperator(primaryStructure->getClass());
  jassert(aaAccumulator);
  primaryStructureAccumulator = aaAccumulator->computeFunction(defaultExecutionContext(), primaryStructure).getObjectAndCast<Container>();*/

  // pssm
  positionSpecificScoringMatrix = protein->getPositionSpecificScoringMatrix();
  /*FunctionPtr pssmAccumulator = accumulateOperator(positionSpecificScoringMatrix->getClass());
  jassert(pssmAccumulator);
  positionSpecificScoringMatrixAccumulator = pssmAccumulator->computeFunction(defaultExecutionContext(), positionSpecificScoringMatrix).getObjectAndCast<Container>();*/

  // secondary structure
  ContainerPtr inputSecondaryStructure = protein->getSecondaryStructure();
  if (inputSecondaryStructure)
  {
    size_t n = primaryStructure->getNumElements();
    secondaryStructure = vector(enumerationDistributionClass(secondaryStructureElementEnumeration), n);
    for (size_t i = 0; i < n; ++i)
    {
      EnumerationDistributionPtr distribution = new EnumerationDistribution(secondaryStructureElementEnumeration);
      distribution->setProbability((size_t)inputSecondaryStructure->getElement(i).getInteger(), 1.0);
      secondaryStructure->setElement(i, distribution);
    }

    /*FunctionPtr discretizeOperator = lbcpp::discretizeOperator(secondaryStructure->getClass(), true);
    jassert(discretizeOperator);
    secondaryStructureLabels = discretizeOperator->computeFunction(defaultExecutionContext(), secondaryStructure).getObject();

    FunctionPtr segmentOperator = lbcpp::segmentOperator(secondaryStructureLabels->getClass());
    jassert(segmentOperator);
    secondaryStructureSegments = segmentOperator->computeFunction(defaultExecutionContext(), secondaryStructureLabels).getObject();*/
  }

  residueFrames = vector(proteinResidueFrameClass, n);
  for (size_t i = 0; i < n; ++i)
    residueFrames->setElement(i, new ProteinResidueFrame(this, i));
}
