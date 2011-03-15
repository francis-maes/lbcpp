/*-----------------------------------------.---------------------------------.
| Filename: BinaryDecisionTreeFunction.h   | Extra Tree Inference            |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_BINARY_DECISION_TREE_H_
# define LBCPP_FUNCTION_BINARY_DECISION_TREE_H_

# include "Data/BinaryDecisionTree.h"
# include <lbcpp/Distribution/Distribution.h>
//# include <lbcpp/Learning/LearnableFunction.h>

namespace lbcpp 
{

class BinaryDecisionTreeFunction : public Function
{
public:
  BinaryDecisionTreeFunction();

  virtual TypePtr getSupervisionType() const = 0;
  
  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)objectClass : getSupervisionType();}
  
  virtual String getOutputPostFix() const
    {return T("BinaryTree");}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    if (!tree || !tree->getNumNodes())
      return Variable::missingValue(getOutputType());
    return tree->makePrediction(context, inputs[0]);
  }

  /* BinaryDecisionTreeFunction */
  const BinaryDecisionTreePtr& getTree() const
    {return tree;}

  void setTree(const BinaryDecisionTreePtr& tree)
    {this->tree = tree;}

protected:
  friend class BinaryDecisionTreeFunctionClass;

  BinaryDecisionTreePtr tree;
};

extern ClassPtr binaryDecisionTreeFunctionClass;
typedef ReferenceCountedObjectPtr<BinaryDecisionTreeFunction> BinaryDecisionTreeFunctionPtr;

class RegressionBinaryDecisionTreeFunction : public BinaryDecisionTreeFunction
{
public:
  virtual TypePtr getSupervisionType() const
    {return doubleType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}
};

class BinaryClassificationBinaryDecisionTreeFunction : public BinaryDecisionTreeFunction
{
public:
  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}
};

class ClassificationBinaryDecisionTreeFunction : public BinaryDecisionTreeFunction
{
public:
  virtual TypePtr getSupervisionType() const
    {return enumValueType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return inputVariables[1]->getType();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_BINARY_DECISION_TREE_H_
