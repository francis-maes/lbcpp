/*-----------------------------------------.---------------------------------.
| Filename: DefaultEvaluator.h             | Default evaluator               |
| Author  : Julien Becker                  |                                 |
| Started : 22/02/2011 15:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_DEFAULT_EVALUATOR_H_
# define LBCPP_FUNCTION_DEFAULT_EVALUATOR_H_

# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class DefaultEvaluator : public ProxyFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(anyType);}
  
  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    TypePtr predictedType = inputVariables[0]->getType()->getTemplateArgument(0);
    TypePtr supervisionType = inputVariables[1]->getType()->getTemplateArgument(0);
    
    if (supervisionType == doubleType)
      return regressionErrorEvaluator();
    if (supervisionType == booleanType || supervisionType == probabilityType)
      return binaryClassificationConfusionEvaluator();
    if (supervisionType->inheritsFrom(enumValueType))
      return classificationAccuracyEvaluator();
    if (supervisionType->inheritsFrom(objectClass) && supervisionType == predictedType)
      return multiLabelClassificationEvaluator();
    return FunctionPtr();
  }
};

};

#endif //!LBCPP_FUNCTION_DEFAULT_EVALUATOR_H_
