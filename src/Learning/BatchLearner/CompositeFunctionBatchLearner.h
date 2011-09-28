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
# include <lbcpp/Execution/WorkUnit.h>

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

    if (context.isMultiThread())
      return trainMultiThread(context, function, trainingStates, validationStates);
    else
      return trainSingleThread(context, function, trainingStates, validationStates);
  }

protected:
  /*
  ** Single Threaded
  */
  bool trainSingleThread(ExecutionContext& context, const CompositeFunctionPtr& function, std::vector<ObjectPtr>& trainingStates, std::vector<ObjectPtr>& validationStates) const
  {
    std::vector<bool> variableIsReady;
    initializeVariableReadyFlags(function, variableIsReady);
    std::vector<size_t> referenceCounts;
    makeInitialReferenceCounts(function, referenceCounts);

    // train each sub function iteratively
    for (size_t step = 0; step < function->getNumSteps(); ++step)
      if (function->getStepType(step) == CompositeFunction::functionStep)
      {
        if (!trainFunctionStep(context, function, step, trainingStates, validationStates, variableIsReady, referenceCounts))
          return false;
      }
    
    return true;
  }

  bool ensureVariableIsReady(ExecutionContext& context, const CompositeFunctionPtr& function, size_t step, std::vector<bool>& variableIsReady,
                             std::vector<ObjectPtr>& trainingStates, std::vector<ObjectPtr>& validationStates,
                             std::vector<size_t>& referenceCounts) const
  {
    if (variableIsReady[step])
      return true;
  
    size_t subFunctionIndex = function->getStepArgument(step);
    //const FunctionPtr& subFunction = function->getSubFunction(subFunctionIndex);
    //jassert(subFunction);
    
    const std::vector<size_t>& inputs = function->getSubFunctionInputs(subFunctionIndex);
    for (size_t i = 0; i < inputs.size(); ++i)
      ensureVariableIsReady(context, function, inputs[i], variableIsReady, trainingStates, validationStates, referenceCounts);
    
    if (!computeSubFunction(context, function, step, trainingStates, validationStates))
      return false;

    decrementReferenceCountsAndFreeVariables(context, function, inputs, referenceCounts, trainingStates, validationStates);
    variableIsReady[step] = true;
    return true;
  }
  
  bool trainFunctionStep(ExecutionContext& context, const CompositeFunctionPtr& function, size_t step,
                         std::vector<ObjectPtr>& trainingStates, std::vector<ObjectPtr>& validationStates,
                         std::vector<bool>& variableIsReady, std::vector<size_t>& referenceCounts) const
  {
    size_t subFunctionIndex = function->getStepArgument(step);
    const FunctionPtr& subFunction = function->getSubFunction(subFunctionIndex);

    if (subFunction->hasBatchLearner())
    {
      // ensure that all inputs are computed
      const std::vector<size_t>& inputs = function->getSubFunctionInputs(function->getStepArgument(step));
      for (size_t i = 0; i < inputs.size(); ++i)
        ensureVariableIsReady(context, function, inputs[i], variableIsReady, trainingStates, validationStates, referenceCounts);
      
      if (!learnSubFunction(context, function, step, trainingStates, validationStates))
        return false;
    }  
    return true;
  }

  bool computeSubFunction(ExecutionContext& context, const CompositeFunctionPtr& function, size_t step, std::vector<ObjectPtr>& trainingStates, std::vector<ObjectPtr>& validationStates) const
  {
    size_t subFunctionIndex = function->getStepArgument(step);
    const FunctionPtr& subFunction = function->getSubFunction(subFunctionIndex);

    bool doScope = true;

    if (doScope)
    {
      String description = T("Computing ") + subFunction->getOutputVariable()->getName() + T(" with ");
      description += String((int)trainingStates.size()) + T(" train examples");
      if (validationStates.size())
        description += T(" and ") + String((int)validationStates.size()) + T(" validation examples");
      context.enterScope(description);
    }

    computeSubFunction(context, function, step, trainingStates);
    if (validationStates.size())
      computeSubFunction(context, function, step, validationStates);

    if (doScope)
      context.leaveScope(Variable());
    return true;
  }

  /*
  ** Multi Threaded
  */
  bool trainMultiThread(ExecutionContext& context, const CompositeFunctionPtr& function, std::vector<ObjectPtr>& trainingStates, std::vector<ObjectPtr>& validationStates) const
  {
    std::vector<bool> variableIsReady;
    initializeVariableReadyFlags(function, variableIsReady);
    std::vector<size_t> referenceCounts;
    makeInitialReferenceCounts(function, referenceCounts);
    
    std::vector<bool> variableIsReadyToLearn = variableIsReady;
    size_t numVariablesToCompute = function->getNumSubFunctions();
    
    // do learning blocks until all sub functions have been learned
    while (numVariablesToCompute > 0)
    {
      // find new steps that are ready to be learned
      std::vector<size_t> readySteps;
      for (size_t i = 0; i < variableIsReadyToLearn.size(); ++i)
        if (!variableIsReadyToLearn[i] && areAllInputsReady(function, i, variableIsReadyToLearn))
          readySteps.push_back(i);
      if (readySteps.empty())
      {
        context.errorCallback(T("No ready steps. Cyclic dependency ?"));
        return false;
      }

      // train these steps
      if (!trainFunctionSteps(context, function, readySteps, trainingStates, validationStates, variableIsReady, referenceCounts))
        return false;

      // update variableIsReady flags and step reference counts
      for (size_t i = 0; i < readySteps.size(); ++i)
        variableIsReadyToLearn[readySteps[i]] = true;
      numVariablesToCompute -= readySteps.size();
    }
    return true;
  }

  bool areAllInputsReady(const CompositeFunctionPtr& function, size_t stepIndex, const std::vector<bool>& variableIsReadyFlags) const
  {
    size_t functionIndex = function->getStepArgument(stepIndex);
    std::vector<size_t> inputs = function->getSubFunctionInputs(functionIndex);
    for (size_t i = 0; i < inputs.size(); ++i)
    {
      jassert(inputs[i] < variableIsReadyFlags.size());
      if (!variableIsReadyFlags[inputs[i]])
        return false;
    }
    return true;
  }

  struct TrainFunctionStepWorkUnit : public WorkUnit
  {
    TrainFunctionStepWorkUnit(const CompositeFunctionBatchLearner* pthis, const CompositeFunctionPtr& function, size_t step,
                              std::vector<ObjectPtr>& trainingStates, std::vector<ObjectPtr>& validationStates,
                              std::vector<bool>& variableIsReady, std::vector<size_t>& referenceCounts, bool& res)
      : pthis(pthis), function(function), step(step),
        trainingStates(trainingStates), validationStates(validationStates),
        variableIsReady(variableIsReady), referenceCounts(referenceCounts), res(res) {}

    const CompositeFunctionBatchLearner* pthis;
    CompositeFunctionPtr function;
    size_t step;
    std::vector<ObjectPtr>& trainingStates;
    std::vector<ObjectPtr>& validationStates;
    std::vector<bool>& variableIsReady;
    std::vector<size_t>& referenceCounts;
    bool& res;

    virtual Variable run(ExecutionContext& context)
    {
      bool ok = pthis->trainFunctionStep(context, function, step, trainingStates, validationStates, variableIsReady, referenceCounts);
      if (!ok)
        res = false;
      return ok;
    }
  };

  bool trainFunctionSteps(ExecutionContext& context, const CompositeFunctionPtr& function, const std::vector<size_t>& steps,
                          std::vector<ObjectPtr>& trainingStates, std::vector<ObjectPtr>& validationStates,
                          std::vector<bool>& variableIsReady, std::vector<size_t>& referenceCounts) const
  {
    if (steps.size() == 1)
      return trainFunctionStep(context, function, steps[0], trainingStates, validationStates, variableIsReady, referenceCounts);

    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Training steps"), steps.size());
    workUnit->setProgressionUnit(T("Tasks"));
    bool res = true;
    for (size_t i = 0; i < steps.size(); ++i)
      workUnit->setWorkUnit(i, new TrainFunctionStepWorkUnit(this, function, steps[i], trainingStates, validationStates, variableIsReady, referenceCounts, res));
    context.run(workUnit, false);
    return res;
  }

  /*
  ** Low level
  */
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

    return trainSubFunction(context, subFunction, subTrainingData, subValidationData);
  }

  void computeSubFunction(ExecutionContext& context, const CompositeFunctionPtr& function, size_t stepNumber, std::vector<ObjectPtr>& states) const
  {
    for (size_t i = 0; i < states.size(); ++i)
      function->updateState(context, stepNumber, states[i]);
  }

  /*
  ** Variable ready flags
  */
  // return the number of non-ready flags
  void initializeVariableReadyFlags(const CompositeFunctionPtr& function, std::vector<bool>& variableIsReady) const
  {
    variableIsReady.resize(function->getNumSteps());
    for (size_t i = 0; i < variableIsReady.size(); ++i)
      variableIsReady[i] = (function->getStepType(i) != CompositeFunction::functionStep);
  }
  
  /*
  ** Steps reference count
  */
  void makeInitialReferenceCounts(const CompositeFunctionPtr& function, std::vector<size_t>& res) const
  {
    res.resize(function->getNumSteps(), 0);
    for (size_t i = 0; i < function->getNumSubFunctions(); ++i)
    {
      const std::vector<size_t>& inputs = function->getSubFunctionInputs(i);
      for (size_t j = 0; j < inputs.size(); ++j)
        res[inputs[j]]++;
    }
  }

  void decrementReferenceCountsAndFreeVariables(ExecutionContext& context, const CompositeFunctionPtr& function, const std::vector<size_t>& indicesToDecrement,
                                                std::vector<size_t>& referenceCounts,
                                                std::vector<ObjectPtr>& trainingStates, std::vector<ObjectPtr>& validationStates) const
  {
    for (size_t i = 0; i < indicesToDecrement.size(); ++i)
    {
      size_t index = indicesToDecrement[i];
      size_t& refCount = referenceCounts[index];
      jassert(refCount > 0);
      --refCount;
      if (refCount == 0)
      {
       // context.informationCallback("Removing variable " + String((int)index));
        Variable missing = Variable::missingValue(function->getStateClass()->getMemberVariableType(index));
        for (size_t i = 0; i < trainingStates.size(); ++i)
          trainingStates[i]->setVariable(index, missing);
        for (size_t i = 0; i < validationStates.size(); ++i)
          validationStates[i]->setVariable(index, missing);
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_COMPOSITE_FUNCTION_H_
