/*-----------------------------------------.---------------------------------.
| Filename: ProteinFrame.cpp               | Protein Frame                   |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinFrame.h"
using namespace lbcpp;

class ProteinFrameClass : public DefaultClass
{
public:
  void addFunctionAndVariable(ExecutionContext& context, const FunctionPtr& function, const std::vector<size_t>& inputs, const String& outputName = String::empty)
  {

  }

  virtual bool initialize(ExecutionContext& context)
  {
    addMemberVariable(context, T("GenericVector<AminoAcidType>"), T("aa"));
    addMemberVariable(context, T("ObjectVector<EnumerationDistribution<AminoAcidType>>"), T("pssm"));

    //addFunctionAndVariable(context, machinFunction(), std::vector<size_t>(1, 0), T("aac"));
    return DefaultClass::initialize(context);
  }
};

ClassPtr lbcpp::proteinFrameClass = new ProteinFrameClass();
