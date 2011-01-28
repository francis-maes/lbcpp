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

class Frame : public Object
{
public:
  Frame(ClassPtr frameClass)
    : Object(frameClass) {}
  Frame() {}

  virtual ~Frame()
  {
    for (size_t i = 0; i < variables.size(); ++i)
      thisClass->getMemberVariableType(i)->destroy(variables[i].first);
  }
    
  std::pair<VariableValue, double>& getVariableValueReference(size_t index)
  {
    jassert(index < thisClass->getNumMemberVariables());
    if (variables.size() <= index)
    {
      size_t i = variables.size();
      variables.resize(index + 1);
      while (i < variables.size())
      {
        variables[i].first = thisClass->getMemberVariableType(i)->getMissingValue();
        variables[i].second = 0.0;
        ++i;
      }
    }
    return variables[index];
  }

  virtual Variable getVariable(size_t index) const
  {
    TypePtr type = thisClass->getMemberVariableType(index);
    if (index < variables.size())
      return Variable::copyFrom(type, variables[index].first);
    else
      return Variable::missingValue(type);
  }

  virtual void setVariable(ExecutionContext& context, size_t index, const Variable& value)
    {setVariable(index, value, Time::getMillisecondCounterHiRes());}

  void setVariable(size_t index, const Variable& value, double time)
  {
    std::pair<VariableValue, double>& v = getVariableValueReference(index);
    value.copyTo(v.first);
    v.second = time;
  }

private:
  std::vector< std::pair<VariableValue, double> > variables;
};

typedef ReferenceCountedObjectPtr<Frame> FramePtr;

extern ClassPtr proteinFrameClass;

class ProteinFrame : public Frame
{
public:
  ProteinFrame(const ProteinPtr& protein)
    : Frame(proteinFrameClass)
  {
    double time = Time::getMillisecondCounterHiRes();
    setVariable(0, protein->getPrimaryStructure(), time);
    setVariable(1, protein->getPositionSpecificScoringMatrix(), time);

  }
};

typedef ReferenceCountedObjectPtr<ProteinFrame> ProteinFramePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FRAME_H_
