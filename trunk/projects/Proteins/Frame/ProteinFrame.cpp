/*-----------------------------------------.---------------------------------.
| Filename: ProteinFrame.cpp               | Protein Frame                   |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinFrame.h"
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
