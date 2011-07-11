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

namespace lbcpp
{

class StreamBasedOptimizerState : public OptimizerState
{
public:
  StreamBasedOptimizerState(ExecutionContext& context, const ObjectPtr& initialState, const std::vector<StreamPtr>& streams)
    : streams(streams)
  {
    if (initialState->getNumVariables() != streams.size())
      context.errorCallback(T("StreamBasedOptimizerState"), T("Invalid number of streams, Expected ") 
                            + String((int)initialState->getNumVariables()) 
                            + T(" found ") + streams.size());

    for (size_t i = 0; i < streams.size(); ++i)
      context.checkInheritance(streams[i]->getElementsType(), initialState->getVariableType(i));

    setBestVariable(initialState);
    setBestScore(DBL_MAX);
  }

  StreamBasedOptimizerState(const ObjectPtr& initialState = ObjectPtr())
    {setBestVariable(initialState);}

  size_t getNumStreams() const
    {return streams.size();}

  const StreamPtr& getStream(size_t index) const
    {jassert(index < streams.size()); return streams[index];}

  void appendStream(const StreamPtr& stream)
    {streams.push_back(stream);}
  
protected:
  friend class StreamBasedOptimizerStateClass;

  std::vector<StreamPtr> streams;
};

typedef ReferenceCountedObjectPtr<StreamBasedOptimizerState> StreamBasedOptimizerStatePtr;

class BestFirstSearchOptimizer : public Optimizer
{
public:
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {
    StreamBasedOptimizerStatePtr streams = optimizerState.dynamicCast<StreamBasedOptimizerState>();
    if (!streams)
    {
      context.errorCallback(T("BestFirstSearchOptimizer::optimize"), T("Expected StreamBasedOptimizerState"));
      return false;
    }
    
    const size_t numParameters = streams->getNumStreams();
    
    std::vector<size_t> parameterIndices(numParameters);
    for (size_t i = 0; i < numParameters; ++i)
      parameterIndices[i] = i;

    while (parameterIndices.size())
    {
      const size_t currentIteration = numParameters - parameterIndices.size();
      context.enterScope(T("Iteration ") + String((int)currentIteration));
      context.resultCallback(T("Iteration"), currentIteration);

      std::vector<std::pair<size_t, Variable> > experiments;
      // Generate candidates
      ObjectPtr baseObject = streams->getBestVariable().getObject();
      for (size_t i = 0; i < parameterIndices.size(); ++i)
      {
        StreamPtr stream = streams->getStream(parameterIndices[i]);
        stream->rewind();
        while (!stream->isExhausted())
        {
          ObjectPtr candidate = baseObject->clone(context);
          Variable value = stream->next();
          candidate->setVariable(parameterIndices[i], value);
          
          if (!optimizerContext->evaluate(candidate))
          {
            context.errorCallback(T("BestFirstSearchOptimizer::optimize"), T("Evaluation failed"));
            return false;
          }
          streams->incTotalNumberOfRequests();
          experiments.push_back(std::make_pair(parameterIndices[i], value));
        }
      }

      // Waiting results
      while (!optimizerContext->areAllRequestsProcessed())
      {
        context.progressCallback(new ProgressionState(streams->getNumberOfProcessedRequests(), experiments.size(), T("Evaluation")));
        streams->autoSaveToFile(context);
        Thread::sleep(optimizerContext->getTimeToSleep());
      }

      jassert(streams->getNumberOfProcessedRequests() == experiments.size());
      context.progressCallback(new ProgressionState(experiments.size(), experiments.size(), T("Evaluations")));

      // Push results into trace
      double bestScore = DBL_MAX;
      size_t bestExperiment = (size_t)-1;
      size_t previousParameter = (size_t)-1;
      double bestParameterScore = DBL_MAX;
      for (size_t i = 0; i < experiments.size(); ++i)
      {
        size_t currentParameter = experiments[i].first;
        const Variable& value = experiments[i].second;
        if (previousParameter != currentParameter)
        {
          if (i)
          {
            context.leaveScope(bestParameterScore);
            bestParameterScore = DBL_MAX;
          }
          context.enterScope(T("Parameter ") + baseObject->getVariableName(currentParameter));
        }

        const double score = getScoreFor(optimizerState, currentParameter, value);
        if (score < bestScore)
        {
          bestScore = score;
          bestExperiment = i;
        }
        
        if (score < bestParameterScore)
          bestParameterScore = score;

        context.enterScope(T("Value ") + value.toString());
        context.resultCallback(T("Value"), value);
        context.resultCallback(T("Score"), score);
        context.leaveScope(score);

        if (i == experiments.size() - 1)
          context.leaveScope(bestParameterScore);
        previousParameter = currentParameter;
      }

      if (!experiments.size())
        return DBL_MAX;

      context.resultCallback(T("Best score"), bestScore);
      context.resultCallback(T("Best parameter"), baseObject->getVariableName(experiments[bestExperiment].first));
      context.resultCallback(T("Best value"), experiments[bestExperiment].second);
      context.leaveScope(bestScore);

      optimizerState->flushProcessedRequests();
      // Stopping criterion : Stop the search if there are no more improvement
      if (bestScore >= optimizerState->getBestScore())
        break;

      // Built new parameter indices for the next iteration
      std::vector<size_t> nextParameterIndices(parameterIndices.size() - 1);
      for (size_t i = 0, j = 0; i < parameterIndices.size(); ++i)
        if (parameterIndices[i] != experiments[bestExperiment].first)
        {
          nextParameterIndices[j] = parameterIndices[i];
          ++j;
        }
      parameterIndices = nextParameterIndices;

      // Update state
      optimizerState->setBestScore(bestScore);
      baseObject->setVariable(experiments[bestExperiment].first, experiments[bestExperiment].second);
    }

    return optimizerState->getBestScore();
  }

protected:
  double getScoreFor(const OptimizerStatePtr& optimizerState, size_t parameter, Variable value) const
  {
    const std::vector<std::pair<double, Variable> >& results = optimizerState->getProcessedRequests();
    for (size_t i = 0; i < results.size(); ++i)
    {
      const ObjectPtr candidate = results[i].second.getObject();
      if (candidate->getVariable(parameter) == value)
        return results[i].first;
    }
    jassertfalse;
    return DBL_MAX;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_BEST_FIRST_SEARCH_H_
