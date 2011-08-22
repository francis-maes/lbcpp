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

class StreamBasedOptimizerState : public OptimizerState
{
public:
  StreamBasedOptimizerState(ExecutionContext& context, const ObjectPtr& initialState, const std::vector<StreamPtr>& streams)
    : OptimizerState(initialState), streams(streams)
  {
    // Types checking
    if (initialState->getNumVariables() != streams.size())
      context.errorCallback(T("StreamBasedOptimizerState"), T("Invalid number of streams, Expected ") 
                            + String((int)initialState->getNumVariables()) 
                            + T(" found ") + streams.size());

    for (size_t i = 0; i < streams.size(); ++i)
      if (streams[i])
        context.checkInheritance(streams[i]->getElementsType(), initialState->getVariableType(i));
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

protected:
  friend class StreamBasedOptimizerStateClass;

  std::vector<StreamPtr> streams;
  std::vector<BestFirstSearchIterationPtr> iterations;

  StreamBasedOptimizerState() {}
};

typedef ReferenceCountedObjectPtr<StreamBasedOptimizerState> StreamBasedOptimizerStatePtr;

class BestFirstSearchOptimizer : public Optimizer
{
public:
  BestFirstSearchOptimizer(const ObjectPtr& initialParameters, const std::vector<StreamPtr>& streams, const File& optimizerStateFile)
    : Optimizer(optimizerStateFile), initialParameters(initialParameters), streams(streams) {}

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context) const
    {return new StreamBasedOptimizerState(context, initialParameters, streams);}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction) const
  {
    StreamBasedOptimizerStatePtr state = optimizerState.dynamicCast<StreamBasedOptimizerState>();
    jassert(state);

    pushPreviousInterationsIntoStack(context, state);

    while (true)
    {
      std::vector<size_t> parameterIndices;
      state->getNextParameters(parameterIndices);
      if (parameterIndices.size() == 0)
        break;
      
      context.enterScope(T("Iteration ") + String((int)state->getNumIterations()));
      context.resultCallback(T("Iteration"), state->getNumIterations());
      
      CompositeWorkUnitPtr workUnits = new CompositeWorkUnit(T("BestFirstSearch - Iteration ")  + String((int)state->getNumIterations()));
      // Generate candidates
      ObjectPtr baseObject = state->getBestParameters().getObject();
      for (size_t i = 0; i < parameterIndices.size(); ++i)
      {
        StreamPtr stream = state->getStream(parameterIndices[i]);
        stream->rewind();
        while (!stream->isExhausted())
        {
          ObjectPtr candidate = baseObject->clone(context);
          Variable value = stream->next();
          candidate->setVariable(parameterIndices[i], value);

          workUnits->addWorkUnit(new FunctionWorkUnit(objectiveFunction, candidate));
        }
      }
      // Get results
      ContainerPtr results = context.run(workUnits, false).getObjectAndCast<Container>(context);
      jassert(results);
      std::cout << results->toString() << std::endl;
      // Update state
      // TODO: add a callback when sending work units, so, we will be able to save intermediate results
      // instead of wait that the full iteration is done. Thus, we need to be able to detect unfinished
      // iteration and restore missing experiments.
      BestFirstSearchIterationPtr iteration = new BestFirstSearchIteration();
      size_t resultIndex = 0;
      for (size_t i = 0; i < parameterIndices.size(); ++i)
      {
        BestFirstSearchParameterPtr parameter = new BestFirstSearchParameter(parameterIndices[i]);
        StreamPtr stream = state->getStream(parameterIndices[i]);
        stream->rewind();
        for (size_t j = 0; !stream->isExhausted(); ++j)
        {
          const Variable v = results->getElement(resultIndex);
          jassert(v.isDouble());
          const double value = v.isDouble() ? v.getDouble() : DBL_MAX;
          parameter->appendValue(value);
          stream->next();
          ++resultIndex;
        }
        iteration->appendParameter(parameter);
      }

      state->appendIteration(iteration);
      pushIterationIntoStack(context, state, iteration);

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

  ObjectPtr initialParameters;
  std::vector<StreamPtr> streams;

  BestFirstSearchOptimizer() {}

  Variable getParameterValue(const StreamBasedOptimizerStatePtr& state, size_t parameterIndex, size_t valueIndex) const
  {
    jassert(parameterIndex != (size_t)-1 && valueIndex != (size_t)-1);
    StreamPtr stream = state->getStream(parameterIndex);
    stream->rewind();
    for (size_t i = valueIndex; i != 0; --i)
      stream->next();
    return stream->next();
  }

  void pushPreviousInterationsIntoStack(ExecutionContext& context, const StreamBasedOptimizerStatePtr& state) const
  {
    const size_t n = state->getNumIterations();
    for (size_t i = 0; i < n; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));
      context.resultCallback(T("Iteration"), i);
      pushIterationIntoStack(context, state, state->getIteration(i));
    }
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
