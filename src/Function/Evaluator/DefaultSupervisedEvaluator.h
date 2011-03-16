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

class DefaultSupervisedEvaluator : public ProxyEvaluator
{
public: 
  virtual EvaluatorPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    TypePtr functionInputsType = Container::getTemplateParameter(inputVariables[1]->getType());
    if (!functionInputsType)
      return EvaluatorPtr();
    TypePtr supervisionType = functionInputsType->getLastMemberVariable()->getType();
    if (!supervisionType)
      return EvaluatorPtr();
    
    if (supervisionType == doubleType)
      return regressionEvaluator();
    if (supervisionType == booleanType || supervisionType == probabilityType)
      return binaryClassificationEvaluator();
    if (supervisionType->inheritsFrom(enumValueType) || supervisionType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
      return classificationEvaluator();
    else if (supervisionType->inheritsFrom(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration)))
      return rankingEvaluator();
    //if (supervisionType->inheritsFrom(objectClass) && supervisionType == predictedType)
    //  return multiLabelClassificationEvaluator();
    return EvaluatorPtr();
  }
};

};

#endif //!LBCPP_FUNCTION_DEFAULT_EVALUATOR_H_
