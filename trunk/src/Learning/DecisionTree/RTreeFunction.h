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
  RTreeFunction();

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
    {return index == 0 ? (TypePtr)objectClass : anyType;}

  virtual String getOutputPostFix() const
    {return T("RTree");}  

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

protected:
  friend class RTreeFunctionClass;
  
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
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return inputVariables[1]->getType();}
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
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_DECISION_TREE_R_TREE_FUNCTION_H_
