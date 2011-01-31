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

class FrameClass : public DefaultClass
{
public:
  FrameClass(const String& name, const String& baseClass)
    : DefaultClass(name, baseClass) {}
  FrameClass(const String& className, TypePtr baseType)
    : DefaultClass(className, baseType) {}
  FrameClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseType)
    : DefaultClass(templateType, templateArguments, baseType) {}

  void addFunctionAndVariable(ExecutionContext& context, const FunctionPtr& function, const std::vector< std::vector<int> >& inputPaths, const String& outputName = String::empty)
  {
    // ...
  }  
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

class AccumulateLabelSequenceFunction : public Function
{
public:
  AccumulateLabelSequenceFunction(EnumerationPtr labelsType)
    : labelsType(labelsType) {}

  virtual TypePtr getInputType() const
    {return containerClass(labelsType);}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return containerClass(enumBasedDoubleVectorClass(labelsType));}

protected:
  EnumerationPtr labelsType;
};


class ProteinFrame : public Object
{
public:
  ProteinFrame(const ProteinPtr& protein)
    //: Object(proteinFrameClass)
  {
    // primary Structure
    primaryStructure = protein->getPrimaryStructure();

    FunctionPtr aaAccumulator = accumulateOperator(primaryStructure->getClass());
    jassert(aaAccumulator);
    primaryStructureAccumulator = aaAccumulator->computeFunction(defaultExecutionContext(), primaryStructure).getObjectAndCast<Container>();
  
    // pssm
    positionSpecificScoringMatrix = protein->getPositionSpecificScoringMatrix();
    FunctionPtr pssmAccumulator = accumulateOperator(positionSpecificScoringMatrix->getClass());
    jassert(pssmAccumulator);
    positionSpecificScoringMatrixAccumulator = pssmAccumulator->computeFunction(defaultExecutionContext(), positionSpecificScoringMatrix).getObjectAndCast<Container>();

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

      FunctionPtr discretizeOperator = lbcpp::discretizeOperator(secondaryStructure->getClass(), true);
      jassert(discretizeOperator);
      secondaryStructureLabels = discretizeOperator->computeFunction(defaultExecutionContext(), secondaryStructure).getObject();
    }

    //double time = Time::getMillisecondCounterHiRes();
    //setVariable(0, protein->getPrimaryStructure(), time);
    //setVariable(1, protein->getPositionSpecificScoringMatrix(), time);
  }
  ProteinFrame() {}

protected:
  friend class ProteinFrameClass;

  VectorPtr primaryStructure;
  ContainerPtr primaryStructureAccumulator;

  VectorPtr positionSpecificScoringMatrix;
  ContainerPtr positionSpecificScoringMatrixAccumulator;

  VectorPtr secondaryStructure;
  VectorPtr secondaryStructureLabels;
};

typedef ReferenceCountedObjectPtr<ProteinFrame> ProteinFramePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FRAME_H_
