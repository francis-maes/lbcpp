/*-----------------------------------------.---------------------------------.
| Filename: BestFirstSearchOptimizer.h     | Best First Search Optimizer     |
| Author  : Julien Becker                  |                                 |
| Started : 08/07/2011 15:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_BEST_FIRST_SEARCH_H_
# define LBCPP_OPTIMIZER_BEST_FIRST_SEARCH_H_

# include <lbcpp/Data/Stream.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Optimizer/OptimizerState.h>
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class BestFirstSearchParameter : public Object
{
public:
  BestFirstSearchParameter(size_t index = (size_t)-1)
    : index(index), isComputed(false) {}

  size_t getIndex() const
    {return index;}

  double getBestScore()
  {
    const_cast<BestFirstSearchParameter*>(this)->ensureIsComputed();
    return bestScore;
  }

  size_t getBestValue() const
  {
    const_cast<BestFirstSearchParameter*>(this)->ensureIsComputed();
    return bestValue;
  }

  double getScore(size_t index) const
    {jassert(index < scores.size()); return scores[index];}

  void setScore(size_t index, double value)
    {jassert(index < scores.size()); scores[index] = value;}

  void appendValue(double score)
    {scores.push_back(score);}

protected:
  friend class BestFirstSearchParameterClass;

  size_t index;
  std::vector<double> scores;

private:
  bool isComputed;

  size_t bestValue;
  double bestScore;

  void ensureIsComputed()
  {
    if (isComputed)
      return;

    size_t bestValue = (size_t)-1;
    double bestScore = DBL_MAX;
    for (size_t i = 0; i < scores.size(); ++i)
      if (scores[i] < bestScore)
      {
        bestValue = i;
        bestScore = scores[i];
      }

    this->bestValue = bestValue;
    this->bestScore = bestScore;

    isComputed = true;
  }
};

typedef ReferenceCountedObjectPtr<BestFirstSearchParameter> BestFirstSearchParameterPtr;

class BestFirstSearchIteration : public Object
{
public:
  BestFirstSearchIteration()
    : isComputed(false) {}

  size_t getNumParameters() const
    {return parameters.size();}

  size_t getBestParameter() const
  {
    const_cast<BestFirstSearchIteration*>(this)->ensureIsComputed();
    return bestParameter;
  }

  size_t getBestValue() const
  {
    const_cast<BestFirstSearchIteration*>(this)->ensureIsComputed();
    return bestValue;
  }

  double getBestScore() const
  {
    const_cast<BestFirstSearchIteration*>(this)->ensureIsComputed();
    return bestScore;
  }

  BestFirstSearchParameterPtr getParameter(size_t index) const
    {jassert(index < parameters.size()); return parameters[index];}

  void appendParameter(const BestFirstSearchParameterPtr& parameter)
    {parameters.push_back(parameter);}

protected:
  friend class BestFirstSearchIterationClass;

  std::vector<BestFirstSearchParameterPtr> parameters;

private:
  bool isComputed;

  size_t bestParameter;
  size_t bestValue;
  double bestScore;

  void ensureIsComputed()
  {
    if (isComputed)
      return;

    size_t bestParameter = (size_t)-1;
    size_t bestValue = (size_t)-1;
    double bestScore = DBL_MAX;
    for (size_t i = 0; i < parameters.size(); ++i)
      if (parameters[i]->getBestScore() < bestScore)
      {
        bestParameter = parameters[i]->getIndex();
        bestValue = parameters[i]->getBestValue();
        bestScore = parameters[i]->getBestScore();
      }

    this->bestParameter = bestParameter;
    this->bestValue = bestValue;
    this->bestScore = bestScore;

    isComputed = true;
  }
};

typedef ReferenceCountedObjectPtr<BestFirstSearchIteration> BestFirstSearchIterationPtr;

class StreamBasedOptimizerState : public OptimizerState, public ExecutionContextCallback
{
public:
  StreamBasedOptimizerState(ExecutionContext& context, const std::vector<StreamPtr>& streams)
    : OptimizerState(), streams(streams)
  {
    /*
    // Types checking
    if (initialState->getNumVariables() != streams.size())
      context.errorCallback(T("StreamBasedOptimizerState"), T("Invalid number of streams, Expected ") 
                            + String((int)initialState->getNumVariables()) 
                            + T(" found ") + streams.size());

    for (size_t i = 0; i < streams.size(); ++i)
      if (streams[i])
        context.checkInheritance(streams[i]->getElementsType(), initialState->getVariableType(i));
      */
  }

  const StreamPtr& getStream(size_t index) const
    {jassert(index < streams.size()); return streams[index];}

  void getNextParameters(std::vector<size_t>& result) const
  {
    std::map<size_t, bool> alreadySelectedParameters;
    for (size_t i = 0; i < iterations.size(); ++i)
      alreadySelectedParameters[iterations[i]->getBestParameter()] = true;

    for (size_t i = 0; i < streams.size(); ++i)
    {
      if (alreadySelectedParameters.count(i) == 1)
        continue;
      if (!streams[i] || (streams[i]->rewind() && streams[i]->isExhausted()))
        continue;

      result.push_back(i);
    }
  }

  size_t getNumIterations() const
    {return iterations.size();}

  BestFirstSearchIterationPtr getIteration(size_t index) const
    {jassert(index < iterations.size()); return iterations[index];}

  void appendIteration(const BestFirstSearchIterationPtr& iteration)
    {iterations.push_back(iteration);}

  ClassPtr getObjectClass() const
    {return getBestParameters().getObject()->getClass();}

  void mapWorkUnitToResult(const WorkUnitPtr& workUnit, size_t parameterIndex, size_t valueIndex)
  {
    ScopedLock _(lock);
    workUnitsToResults[workUnit] = std::make_pair(parameterIndex, valueIndex);
  }

  bool areAllWorkUnitsDone() const
    {return workUnitsToResults.size() == 0;}

  size_t getNumWorkUnitInProgress() const
    {return workUnitsToResults.size();}

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    ScopedLock _(lock);
    jassert(workUnitsToResults.count(workUnit) == 1);
    const std::pair<size_t, size_t> indices = workUnitsToResults[workUnit];
    const double value = result.isDouble() ? result.getDouble()
                       : (result.isInteger() ? (double)result.getInteger() : DBL_MAX);
    iterations[iterations.size() - 1]->getParameter(indices.first)->setScore(indices.second, value);
    workUnitsToResults.erase(workUnit);
  }

protected:
  friend class StreamBasedOptimizerStateClass;

  std::vector<StreamPtr> streams;
  std::vector<BestFirstSearchIterationPtr> iterations;

  StreamBasedOptimizerState() {}

private:
  CriticalSection lock;
  std::map<WorkUnitPtr, std::pair<size_t, size_t> > workUnitsToResults;
};

typedef ReferenceCountedObjectPtr<StreamBasedOptimizerState> StreamBasedOptimizerStatePtr;

class BestFirstSearchOptimizer : public Optimizer
{
public:
  BestFirstSearchOptimizer(const std::vector<StreamPtr>& streams, const File& optimizerStateFile)
    : Optimizer(optimizerStateFile), streams(streams) {}

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context) const
    {return new StreamBasedOptimizerState(context, streams);}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    const FunctionPtr& objectiveFunction = problem->getObjective();
    StreamBasedOptimizerStatePtr state = optimizerState.staticCast<StreamBasedOptimizerState>();
    jassert(state);
    
    if (!state->getBestParameters().exists())
    {
      ObjectPtr initialParameters = problem->getInitialGuess().getObject();
      state->setBestParameters(initialParameters);
    }

    const double missingValue = Variable::missingValue(doubleType).getDouble();

    for (size_t numIteration = 0; true; ++numIteration)
    {
      // Create iteration
      BestFirstSearchIterationPtr iteration = numIteration < state->getNumIterations()
                                            ? state->getIteration(numIteration)
                                            : createIteration(state);

      if (!iteration) // No more parameter to test
        break;

      context.enterScope(T("Iteration ") + String((int)state->getNumIterations()));
      context.resultCallback(T("Iteration"), state->getNumIterations());

      // Generate candidates
      ObjectPtr baseObject = state->getBestParameters().getObject();
      size_t numPushedWorkUnit = 0;
      const size_t n = iteration->getNumParameters();
      for (size_t i = 0; i < n; ++i)
      {
        BestFirstSearchParameterPtr parameter = iteration->getParameter(i);
        StreamPtr stream = state->getStream(parameter->getIndex());
        stream->rewind();
        for (size_t valueIndex = 0; !stream->isExhausted(); ++valueIndex)
        {
          const Variable value = stream->next();
          if (parameter->getScore(valueIndex) != missingValue)
            continue;

          ObjectPtr candidate = baseObject->clone(context);
          candidate->setVariable(parameter->getIndex(), value);

          WorkUnitPtr workUnit = new FunctionWorkUnit(objectiveFunction, candidate);
          state->mapWorkUnitToResult(workUnit, i, valueIndex);

          context.pushWorkUnit(workUnit, state.get(), false);
          ++numPushedWorkUnit;
        }
      }

      // Waiting results
      size_t previousNumWorkUnitInProgress = (size_t)-1;
      while (!state->areAllWorkUnitsDone())
      {
        const size_t currentNumWorkUnitInProgress = state->getNumWorkUnitInProgress();
        context.progressCallback(new ProgressionState(numPushedWorkUnit - currentNumWorkUnitInProgress, numPushedWorkUnit, T("Evaluation")));
        if (currentNumWorkUnitInProgress != previousNumWorkUnitInProgress)
        {
          saveOptimizerState(context, state);
          previousNumWorkUnitInProgress = currentNumWorkUnitInProgress;
        }
        juce::Thread::sleep(500);
      }
      context.progressCallback(new ProgressionState(numPushedWorkUnit, numPushedWorkUnit, T("Evaluation")));

      // Update state
      pushIterationIntoStack(context, state, iteration);

      if (numPushedWorkUnit == 0) // No need to update and/or save state
        continue;

      if (iteration->getBestScore() >= state->getBestScore())
        break;

      state->setBestScore(iteration->getBestScore());
      baseObject->setVariable(iteration->getBestParameter(), getParameterValue(state, iteration->getBestParameter(), iteration->getBestValue()));

      saveOptimizerState(context, state);
    }

    return state;
  }

protected:
  friend class BestFirstSearchOptimizerClass;

  std::vector<StreamPtr> streams;

  BestFirstSearchOptimizer() {}

  BestFirstSearchIterationPtr createIteration(const StreamBasedOptimizerStatePtr& state) const
  {
    std::vector<size_t> parameterIndices;
    state->getNextParameters(parameterIndices);
    if (parameterIndices.size() == 0)
      return BestFirstSearchIterationPtr();

    BestFirstSearchIterationPtr iteration = new BestFirstSearchIteration();
    state->appendIteration(iteration);
    
    for (size_t i = 0; i < parameterIndices.size(); ++i)
    {
      BestFirstSearchParameterPtr parameter = new BestFirstSearchParameter(parameterIndices[i]);
      iteration->appendParameter(parameter);

      StreamPtr stream = state->getStream(parameterIndices[i]);
      for (stream->rewind(); !stream->isExhausted(); stream->next())
        parameter->appendValue(Variable::missingValue(doubleType).getDouble());
    }

    return iteration;
  }

  Variable getParameterValue(const StreamBasedOptimizerStatePtr& state, size_t parameterIndex, size_t valueIndex) const
  {
    jassert(parameterIndex != (size_t)-1 && valueIndex != (size_t)-1);
    StreamPtr stream = state->getStream(parameterIndex);
    stream->rewind();
    for (size_t i = valueIndex; i != 0; --i)
      stream->next();
    return stream->next();
  }
  
  void pushIterationIntoStack(ExecutionContext& context, const StreamBasedOptimizerStatePtr& state, const BestFirstSearchIterationPtr& iteration) const
  {
    const size_t n = iteration->getNumParameters();
    const ClassPtr objClass = state->getObjectClass();
    for (size_t i = 0; i < n; ++i)
    {
      BestFirstSearchParameterPtr parameter = iteration->getParameter(i);
      context.enterScope(T("Parameter ") + objClass->getMemberVariableName(parameter->getIndex()));
      pushParameterValuesIntoStack(context, state, parameter);
      context.leaveScope(parameter->getBestScore());
    }

    context.resultCallback(T("Best score"), iteration->getBestScore());
    context.resultCallback(T("Best parameter"), objClass->getMemberVariableName(iteration->getBestParameter()));
    context.resultCallback(T("Best value"), getParameterValue(state, iteration->getBestParameter(), iteration->getBestValue()));
    context.leaveScope(iteration->getBestScore());
  }

  void pushParameterValuesIntoStack(ExecutionContext& context, const StreamBasedOptimizerStatePtr& state, const BestFirstSearchParameterPtr& parameter) const
  {
    StreamPtr stream = state->getStream(parameter->getIndex());
    stream->rewind();
    size_t i = 0;
    while (!stream->isExhausted())
    {
      Variable value = stream->next();
      context.enterScope(T("Value ") + value.toString());
      context.resultCallback(T("Value"), value);
      const double score = parameter->getScore(i);
      context.resultCallback(T("Score"), score);
      context.leaveScope(score);
      ++i;
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_BEST_FIRST_SEARCH_H_
