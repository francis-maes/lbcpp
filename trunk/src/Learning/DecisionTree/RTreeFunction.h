/*-----------------------------------------.---------------------------------.
| Filename: RTreeFunction.h                | Wrapper of ExtraTrees           |
| Author  : Julien Becker                  | implemented by Pierre Geurts    |
| Started : 16/02/2011 11:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_R_TREE_FUNCTION_H_
# define LBCPP_DECISION_TREE_R_TREE_FUNCTION_H_

# include <lbcpp/Learning/LearnableFunction.h>

namespace lbcpp
{

class RTreeFunction : public LearnableFunction
{
public:
  RTreeFunction(size_t numTrees,
                size_t numAttributeSamplesPerSplit,
                size_t minimumSizeForSplitting);
  RTreeFunction()
    {parametersClass = objectClass;} // FIXME: create constructor LearnableFunction(ClassPtr)

  /* RTreeFunction */
  void setTrees(ObjectPtr trees)
    {parameters = trees;}

  size_t getNumTrees() const
    {return numTrees;}

  size_t getNumAttributeSamplesPerSplit() const
    {return numAttributeSamplesPerSplit;}

  size_t getMinimumSizeForSplitting() const
    {return minimumSizeForSplitting;}

  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)objectVectorClass(objectClass) : anyType;}

  virtual String getOutputPostFix() const
    {return T("RTree");}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

protected:
  friend class RTreeFunctionClass;
  
  size_t numTrees;
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;
};

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
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {jassert(inputVariables[1]->getType()->inheritsFrom(enumValueType)); return inputVariables[1]->getType();}
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
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {jassert(inputVariables[1]->getType()->inheritsFrom(doubleType)); return doubleType;}
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
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {jassert(inputVariables[1]->getType()->inheritsFrom(sumType(booleanType, probabilityType))); return probabilityType;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_DECISION_TREE_R_TREE_FUNCTION_H_
