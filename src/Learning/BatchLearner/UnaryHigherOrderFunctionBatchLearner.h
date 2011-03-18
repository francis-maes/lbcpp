/*-----------------------------------------.---------------------------------.
| Filename: UnaryHigherOrderFunctionBatc..h| A Learner for unary higher      |
| Author  : Francis Maes                   |   order functions               |
| Started : 18/03/2011 13:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_BATCH_LEARNER_UNARY_HIGHER_ORDER_FUNCTION_H_
# define LBCPP_LEARNING_BATCH_LEARNER_UNARY_HIGHER_ORDER_FUNCTION_H_

# include <lbcpp/Learning/BatchLearner.h>

namespace lbcpp
{

class UnaryHigherOrderFunctionBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return unaryHigherOrderFunctionClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const UnaryHigherOrderFunctionPtr& function = f.staticCast<UnaryHigherOrderFunction>();
    const FunctionPtr& baseFunction = function->getBaseFunction();
    jassert(baseFunction);

    ObjectVectorPtr subTrainingData = makeSubExamples(function, baseFunction, trainingData);
    ObjectVectorPtr subValidationData = makeSubExamples(function, baseFunction, validationData);
    return trainSubFunction(context, baseFunction, subTrainingData, subValidationData);
  }

protected:
  ObjectVectorPtr makeSubExamples(const UnaryHigherOrderFunctionPtr& function, const FunctionPtr& baseFunction, const std::vector<ObjectPtr>& examples) const
  {
    if (!examples.size())
      return ObjectVectorPtr();

    ClassPtr inputsClass = baseFunction->getInputsClass();
    ObjectVectorPtr res = new ObjectVector(inputsClass, computeNumSubExamples(function, examples));
    std::vector<ObjectPtr>& subExamples = res->getObjects();
    size_t index = 0;
    for (size_t i = 0; i < examples.size(); ++i)
      function->appendSubInputs(examples[i], subExamples, index);
    jassert(index == subExamples.size());
    return res;
  }

  size_t computeNumSubExamples(const UnaryHigherOrderFunctionPtr& function, const std::vector<ObjectPtr>& inputs) const
  {
    size_t res = 0;
    for (size_t i = 0; i < inputs.size(); ++i)
      res += function->getNumSubInputs(inputs[i]);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_UNARY_HIGHER_ORDER_FUNCTION_H_
