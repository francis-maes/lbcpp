/*-----------------------------------------.---------------------------------.
| Filename: ProteinFrame.cpp               | Protein Frame                   |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinFrame.h"
using namespace lbcpp;

class ProteinFrameClass : public FrameClass
{
public:
  virtual void initializeFrame(ExecutionContext& context, const FramePtr& f)
  {
    const ProteinFramePtr& frame = f.staticCast<ProteinFrame>();
    const ProteinPtr& protein = frame->getProtein();
    frame->setVariable(context, 0, protein->getVariable(0));
    frame->setVariable(context, 1, protein->getVariable(1));
  }

  virtual bool initialize(ExecutionContext& context)
  {
    addNode(context, T("GenericVector<AminoAcidType>"), T("primaryStructure"), T("aa"), T("Primary Structure"));
    addNode(context, T("ObjectVector<EnumerationDistribution<AminoAcidType>>"), T("positionSpecificScoringMatrix"), T("pssm"), T("PSSM"));
    return FrameClass::initialize(context);
  }
};

FrameClassPtr lbcpp::proteinFrameClass = new FrameClass();
