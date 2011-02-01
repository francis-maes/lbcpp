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

class FrameVariableSignature : public VariableSignature
{
public:
  FrameVariableSignature(TypePtr type,
                  const String& name,
                  const String& shortName = String::empty,
                  const String& description = String::empty)
    : VariableSignature(type, name, shortName, description) {}

protected:
  OperatorPtr op;
};

class FrameClass : public Class
{
public:
  FrameClass(const String& name, TypePtr baseClass)
    : Class(name, baseClass) {}
  FrameClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
    : Class(templateType, templateArguments, baseClass) {}
  FrameClass() {}

  void addFunctionAndVariable(ExecutionContext& context, const FunctionPtr& function, const std::vector< std::vector<int> >& inputPaths, const String& outputName = String::empty)
  {
    // ...
  }

private:
  


};

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

class ProteinFrame : public Object
{
public:
  ProteinFrame(const ProteinPtr& protein);
  ProteinFrame() {}

  VectorPtr getPrimaryStructure() const
    {return primaryStructure;}

protected:
  friend class ProteinFrameClass;

  VectorPtr primaryStructure;
  ContainerPtr primaryStructureAccumulator;

  VectorPtr positionSpecificScoringMatrix;
  ContainerPtr positionSpecificScoringMatrixAccumulator;

  VectorPtr secondaryStructure;
  VectorPtr secondaryStructureLabels;
  ContainerPtr secondaryStructureSegments;

  VectorPtr residueFrames;
};

typedef ReferenceCountedObjectPtr<ProteinFrame> ProteinFramePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FRAME_H_
