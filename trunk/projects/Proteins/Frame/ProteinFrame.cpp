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
