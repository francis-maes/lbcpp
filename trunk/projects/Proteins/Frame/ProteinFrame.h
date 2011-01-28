/*-----------------------------------------.---------------------------------.
| Filename: ProteinFrame.h                 | Protein Frame                   |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 14:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_FRAME_H_
# define LBCPP_PROTEIN_FRAME_H_

# include <lbcpp/lbcpp.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class FrameClass;
typedef ReferenceCountedObjectPtr<FrameClass> FrameClassPtr;
class Frame;
typedef ReferenceCountedObjectPtr<Frame> FramePtr;

class FrameClass : public Class
{
public:
  virtual void initializeFrame(ExecutionContext& context, const FramePtr& frame) = 0;

  void addNode(ExecutionContext& context, const String& typeName, const String& name, const String& shortName, const String& description)
  {

  }  
};

class Frame : public Object
{
public:
  Frame(FrameClassPtr type)
    : Object(type) {}
};


class ProteinFrame : public Frame
{
public:
  const ProteinPtr& getProtein() const
    {return protein;}

protected:
  ProteinPtr protein;
};

typedef ReferenceCountedObjectPtr<ProteinFrame> ProteinFramePtr;

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

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FRAME_H_
