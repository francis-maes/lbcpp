/*-----------------------------------------.---------------------------------.
| Filename: RTreeFunction.h                | Wrapper of ExtraTrees           |
| Author  : Julien Becker                  | implemented by Pierre Geurts    |
| Started : 16/02/2011 11:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_R_TREE_FUNCTION_H_
# define LBCPP_DECISION_TREE_R_TREE_FUNCTION_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class RTreeFunction : public Function
{
public:
  RTreeFunction(size_t numTrees,
                size_t numAttributeSamplesPerSplit,
                size_t minimumSizeForSplitting);
  RTreeFunction();

  /* RTreeFunction */
  void setTrees(ObjectPtr trees)
    {this->trees = trees;}

  size_t getNumTrees() const
    {return numTrees;}

  size_t getNumAttributeSamplesPerSplit() const
    {return numAttributeSamplesPerSplit;}

  size_t getMinimumSizeForSplitting() const
    {return minimumSizeForSplitting;}
  
  virtual TypePtr getSupervisionType() const = 0;

  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)containerClass(anyType) : getSupervisionType();}

  virtual String getOutputPostFix() const
    {return T("RTree");}  

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

protected:
  friend class RTreeFunctionClass;
  
  ObjectPtr trees;
  size_t numTrees;
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;
};

extern ClassPtr rTreeFunctionClass;

class ClassificationRTreeFunction : public RTreeFunction
{
public:
  ClassificationRTreeFunction(size_t numTrees,
                              size_t numAttributeSamplesPerSplit,
                              size_t minimumSizeForSplitting)
    : RTreeFunction(numTrees,
                    numAttributeSamplesPerSplit,
                    minimumSizeForSplitting) {}
  ClassificationRTreeFunction() {}
  
  virtual TypePtr getSupervisionType() const
    {return sumType(enumValueType, doubleVectorClass(enumValueType, probabilityType));}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (inputVariables[1]->getType()->inheritsFrom(enumValueType))
      return denseDoubleVectorClass(inputVariables[1]->getType(), probabilityType);
    return inputVariables[1]->getType();
  }
};
  
class RegressionRTreeFunction : public RTreeFunction
{
public:
  RegressionRTreeFunction(size_t numTrees,
                          size_t numAttributeSamplesPerSplit,
                          size_t minimumSizeForSplitting)
    : RTreeFunction(numTrees,
                    numAttributeSamplesPerSplit,
                    minimumSizeForSplitting) {}
  RegressionRTreeFunction() {}
  
  virtual TypePtr getSupervisionType() const
    {return doubleType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}
};

class BinaryRTreeFunction : public RTreeFunction
{
public:
  BinaryRTreeFunction(size_t numTrees,
                      size_t numAttributeSamplesPerSplit,
                      size_t minimumSizeForSplitting)
    : RTreeFunction(numTrees,
                    numAttributeSamplesPerSplit,
                    minimumSizeForSplitting) {}
  BinaryRTreeFunction() {}
  
  virtual TypePtr getSupervisionType() const
    {return sumType(probabilityType, booleanType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}
};

class ExtraTreeLearningMachine : public ProxyFunction
{
public:
  ExtraTreeLearningMachine(size_t numTrees,
                           size_t numAttributeSamplesPerSplit,
                           size_t minimumSizeForSplitting)
    : numTrees(numTrees), 
      numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
      minimumSizeForSplitting(minimumSizeForSplitting) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual String getOutputPostFix() const
    {return T("Prediction");}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)containerClass() : anyType;}

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    TypePtr inputsType = inputVariables[0]->getType();
    TypePtr supervisionType = inputVariables[1]->getType();

    if (supervisionType == doubleType)
      return new RegressionRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
    else if (supervisionType == probabilityType || supervisionType == booleanType)
      return new BinaryRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
    else if (supervisionType->inheritsFrom(enumValueType) || supervisionType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
      return new ClassificationRTreeFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);
    else
    {
      jassertfalse;
      return FunctionPtr();
    }
  }

protected:
  friend class ExtraTreeLearningMachineClass;

  size_t numTrees;
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;

  ExtraTreeLearningMachine() {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_DECISION_TREE_R_TREE_FUNCTION_H_
