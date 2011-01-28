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

extern ClassPtr cumulativeScoreVectorClass(TypePtr elementsType);

class CumulativeScoreVector : public Object
{
public:
  CumulativeScoreVector(TypePtr elementsType)
    : Object(cumulativeScoreVectorClass(elementsType)) {}
  CumulativeScoreVector() {}

  std::vector<double>& getAccumulatedScores(size_t index)
    {jassert(index < accumulators.size()); return accumulators[index];}

  size_t getNumElements() const
    {return accumulators.size();}

  void beginCompute(size_t length, size_t numScores)
  {
    accumulators.clear();
    accumulators.resize(length);
    accumulators[0].resize(numScores);
  }

  std::vector<double>& computeStep(size_t i)
  {
   std::vector<double>& scores = accumulators[i];
    if (i > 0)
      scores = accumulators[i - 1];
    return scores;
  }

  lbcpp_UseDebuggingNewOperator

private:
  std::vector< std::vector<double> > accumulators; // index -> label -> count
};

typedef ReferenceCountedObjectPtr<CumulativeScoreVector> CumulativeScoreVectorPtr;

class AccumulateContainerFunction : public Function
{
public:
  AccumulateContainerFunction(TypePtr elementsType)
    : elementsType(elementsType) {}

  virtual TypePtr getInputType() const
    {return containerClass(elementsType);}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return cumulativeScoreVectorClass(inputType);}

  virtual void accumulate(ExecutionContext& context, const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const = 0;

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    if (!checkType(context, input, containerClass(anyType)) || !checkExistence(context, input))
      return Variable();

    const ContainerPtr& container = input.getObjectAndCast<Container>();
    CumulativeScoreVectorPtr res(new CumulativeScoreVector(container->getElementsType()));
    accumulate(context, container, res);
    return res;
  };

protected:
  TypePtr elementsType;
};

// enum values + missing
class AccumulateEnumerationContainerFunction : public AccumulateContainerFunction
{
public:
  AccumulateEnumerationContainerFunction(EnumerationPtr enumeration)
    : AccumulateContainerFunction(enumeration) {}

  virtual void accumulate(ExecutionContext& context, const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    const EnumerationPtr& enumeration = elementsType.staticCast<Enumeration>();

    size_t n = container->getNumElements();
    res->beginCompute(n, enumeration->getNumElements() + 1);
    for (size_t i = 0; i < n; ++i)
    {
      std::vector<double>& scores = res->computeStep(i);
      scores[container->getElement(i).getInteger()] += 1.0;
    }
  }
};

// enum values + missing + entropy
class AccumulateEnumerationDistributionContainerFunction : public AccumulateContainerFunction
{
public:
  AccumulateEnumerationDistributionContainerFunction(EnumerationPtr enumeration)
    : AccumulateContainerFunction(enumerationDistributionClass(enumeration)), enumeration(enumeration) {}

  virtual void accumulate(ExecutionContext& context, const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    res->beginCompute(n, enumeration->getNumElements() + 2);
    for (size_t i = 0; i < n; ++i)
    {
      std::vector<double>& scores = res->computeStep(i);
      
      EnumerationDistributionPtr distribution = container->getElement(i).getObjectAndCast<EnumerationDistribution>();
      jassert(distribution);
      for (size_t j = 0; j <= enumeration->getNumElements(); ++j)
        scores[j] += distribution->compute(context, Variable(j, enumeration));
      scores.back() += distribution->computeEntropy();
    }
  }

protected:
  EnumerationPtr enumeration;
};

// value sum, missing value count
class AccumulateDoubleContainerFunction : public AccumulateContainerFunction
{
public:
  AccumulateDoubleContainerFunction() : AccumulateContainerFunction(doubleType) {}

  virtual void accumulate(ExecutionContext& context, const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    res->beginCompute(n, 2);
    for (size_t i = 0; i < n; ++i)
    {
      std::vector<double>& scores = res->computeStep(i);
      Variable element = container->getElement(i);
      if (element.exists())
        scores[0] += element.getDouble();
      else
        scores[1] += 1.0;
    }
  }
};

inline FunctionPtr accumulateFunction(TypePtr inputType)
{
  if (inputType->inheritsFrom(containerClass(anyType)))
  {
    TypePtr elementsType = inputType->getTemplateArgument(0); // FIXME: cast into container base class !!!!
    if (elementsType.dynamicCast<Enumeration>())
      return new AccumulateEnumerationContainerFunction(elementsType);
    else if (elementsType->inheritsFrom(doubleType))
      return new AccumulateDoubleContainerFunction();
    else if (elementsType->inheritsFrom(enumerationDistributionClass(anyType)))
    {
      EnumerationPtr enumeration = elementsType->getTemplateArgument(0).dynamicCast<Enumeration>();
      jassert(enumeration);
      return new AccumulateEnumerationDistributionContainerFunction(enumeration);
    }
  }

  return FunctionPtr();
}

//////////////////////////////////////////////

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
