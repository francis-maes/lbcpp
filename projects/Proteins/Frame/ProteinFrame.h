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
  //virtual void initializeFrame(ExecutionContext& context, const FramePtr& frame) = 0;

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

extern FrameClassPtr proteinFrameClass;

class ProteinFrame : public Frame
{
public:
  ProteinFrame(const ProteinPtr& protein)
    : Frame(proteinFrameClass), protein(protein)
  {
    //setNodeValue(
  }

  const ProteinPtr& getProtein() const
    {return protein;}

protected:
  ProteinPtr protein;
};

typedef ReferenceCountedObjectPtr<ProteinFrame> ProteinFramePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FRAME_H_
