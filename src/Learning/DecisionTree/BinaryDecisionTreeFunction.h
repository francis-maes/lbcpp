/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInference.h           | Extra Tree Inference            |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_BINARY_DECISION_TREE_H_
# define LBCPP_FUNCTION_BINARY_DECISION_TREE_H_

# include "Data/BinaryDecisionTree.h"
# include <lbcpp/Distribution/Distribution.h>
# include <lbcpp/Learning/LearnableFunction.h>

namespace lbcpp 
{

class BinaryDecisionTreeFunction : public LearnableFunction
{
public:
  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)objectVectorClass(objectClass) : anyType;}
  
  virtual String getOutputPostFix() const
    {return T("BinaryTree");}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    if (!parameters && parameters.staticCast<BinaryDecisionTree>()->getNumNodes())
      return Variable::missingValue(getOutputType());
    return parameters.staticCast<BinaryDecisionTree>()->makePrediction(context, inputs);
  }

  /* BinaryDecisionTreeFunction */
  BinaryDecisionTreePtr getTree() const
    {return parameters ? parameters.staticCast<BinaryDecisionTree>() : BinaryDecisionTreePtr();}

  void setTree(BinaryDecisionTreePtr tree)
    {parameters = tree;}

  /* Object */
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    LearnableFunction::clone(context, target);
    ReferenceCountedObjectPtr<BinaryDecisionTreeFunction> res = target.staticCast<BinaryDecisionTreeFunction>();
    if (parameters)
      res->parameters = parameters->clone(context);
  }
};

extern ClassPtr binaryDecisionTreeFunctionClass;
typedef ReferenceCountedObjectPtr<BinaryDecisionTreeFunction> BinaryDecisionTreeFunctionPtr;

class RegressionBinaryDecisionTreeFunction : public BinaryDecisionTreeFunction
{
public:
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {jassert(inputVariables[1]->getType()->inheritsFrom(doubleType)); return doubleType;}
};

class BinaryClassificationBinaryDecisionTreeFunction : public BinaryDecisionTreeFunction
{
public:
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {jassert(inputVariables[1]->getType()->inheritsFrom(sumType(booleanType, probabilityType))); return probabilityType;}
};

class ClassificationBinaryDecisionTreeFunction : public BinaryDecisionTreeFunction
{
public:
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {jassert(inputVariables[1]->getType()->inheritsFrom(enumValueType)); return inputVariables[1]->getType();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_BINARY_DECISION_TREE_H_
