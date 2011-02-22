/*-----------------------------------------.---------------------------------.
| Filename: CompositeFunctionBatchLearner.h| Composite Function learner      |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 19:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_BATCH_LEARNER_COMPOSITE_FUNCTION_H_
# define LBCPP_LEARNING_BATCH_LEARNER_COMPOSITE_FUNCTION_H_

# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Core/CompositeFunction.h>

namespace lbcpp
{

class CompositeFunctionBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return compositeFunctionClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const CompositeFunctionPtr& function = f.staticCast<CompositeFunction>();

    // make initial frames
    std::vector<ObjectPtr> trainingStates;
    makeInitialStates(function, trainingData, trainingStates);
    std::vector<ObjectPtr> validationStates;
    if (validationData.size())
      makeInitialStates(function, validationData, validationStates);

    // for each function
    for (size_t step = 0; step < function->getNumSteps(); ++step)
      if (function->getStepType(step) == CompositeFunction::functionStep)
      {
        size_t subFunctionIndex = function->getStepArgument(step);
        const FunctionPtr& subFunction = function->getSubFunction(subFunctionIndex);
        if (subFunction->hasBatchLearner() && !learnSubFunction(context, function, step, trainingStates, validationStates))
          return false;

        computeSubFunction(context, function, step, trainingStates);
        if (validationStates.size())
          computeSubFunction(context, function, step, validationStates);
      }
    return true;
  }

protected:
  void makeInitialStates(const CompositeFunctionPtr& function, const std::vector<ObjectPtr>& data, std::vector<ObjectPtr>& res) const
  {
    size_t n = data.size();
    res.resize(n);
    for (size_t i = 0; i < n; ++i)
      res[i] = function->makeInitialState(data[i]);
  }

  ObjectVectorPtr makeSubInputs(const CompositeFunctionPtr& function, size_t stepNumber, const std::vector<ObjectPtr>& states) const
  {
    //const FunctionPtr& subFunction = function->getSubFunction(function->getStepArgument(stepNumber));
    const DynamicClassPtr& subInputsClass = function->getInputsClass();

    ObjectVectorPtr res = new ObjectVector(subInputsClass, states.size());
    std::vector<ObjectPtr>& v = res->getObjects();
    for (size_t i = 0; i < states.size(); ++i)
      v[i] = function->makeSubInputsObject(stepNumber, states[i]);
    return res;
  }

  bool learnSubFunction(ExecutionContext& context, const CompositeFunctionPtr& function, size_t stepNumber,
                          const std::vector<ObjectPtr>& trainingStates, const std::vector<ObjectPtr>& validationStates) const
  {
    const FunctionPtr& subFunction = function->getSubFunction(function->getStepArgument(stepNumber));

    ObjectVectorPtr subTrainingData = makeSubInputs(function, stepNumber, trainingStates);
    ObjectVectorPtr subValidationData;
    if (validationStates.size())
      subValidationData = makeSubInputs(function, stepNumber, validationStates);

    // todo: factorize this code somewhere:
    String description = T("Learning ") + subFunction->getOutputVariable()->getName() + T(" with ");
    description += String((int)subTrainingData->getNumElements()) + T(" train examples");
    if (subValidationData)
      description += T(" and ") + String((int)subValidationData->getNumElements()) + T(" validation examples");
    // -
    return subFunction->train(context, subTrainingData, subValidationData, description);
  }

  void computeSubFunction(ExecutionContext& context, const CompositeFunctionPtr& function, size_t stepNumber, std::vector<ObjectPtr>& states) const
  {
    for (size_t i = 0; i < states.size(); ++i)
      function->updateState(context, stepNumber, states[i]);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_COMPOSITE_FUNCTION_H_
