/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.cpp                  | Implementation file of          |
| Author  : Arnaud Schoofs                 | Optimizer, OptimizerState and   |
| Started : 11/04/2011                     | OptimizerContext                |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Optimizer/Optimizer.h>
#include <lbcpp/Optimizer/OptimizerState.h>
using namespace lbcpp;

/*
** Optimizer <- Function
** OptimizerContext, OptimizerState -> Variable
*/
TypePtr Optimizer::getRequiredContextType() const
  {return optimizerContextClass;}

TypePtr Optimizer::getRequiredStateType() const
  {return optimizerStateClass;}

size_t Optimizer::getNumRequiredInputs() const
  {return 2;}

TypePtr Optimizer::getRequiredInputType(size_t index, size_t numInputs) const
  {return index == 0 ? getRequiredContextType() : getRequiredStateType();}

String Optimizer::getOutputPostFix() const
  {return T("Optimized");}

TypePtr Optimizer::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {return variableType;}

Variable Optimizer::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  OptimizerContextPtr optimizerContext = inputs[0].getObjectAndCast<OptimizerContext>();
  OptimizerStatePtr optimizerState = inputs[1].getObjectAndCast<OptimizerState>();
  optimizerState->initialize(); // in case OptimizerState is instantiated from a file and totalNumberOfRequests != totalNumberOfResults
  context.enterScope(T("Optimizing"));
  context.informationCallback(toString());
  optimizerContext->setPostEvaluationCallback((FunctionCallbackPtr) optimizerState.get());  // the FunctionCallback is the OptimizerState, it stores the results in processedRequests
  Variable output = optimize(context, optimizerContext, optimizerState);
  optimizerContext->removePostEvaluationCallback((FunctionCallbackPtr) optimizerState.get());
  context.resultCallback(T("bestParameters"), optimizerState->getBestVariable());
  context.resultCallback(T("bestScore"), optimizerState->getBestScore());
  context.leaveScope(optimizerState->getBestVariable());
  return output;  // bestVariable
}

/*
 ** OptimizerState
 ** this class can be accessed in reading and writing from different contexts -> synchronized using lock
 */
OptimizerState::OptimizerState(double autoSaveStateFrequency) 
  : totalNumberOfRequests(0), totalNumberOfResults(0), bestVariable(Variable()), bestScore(DBL_MAX), autoSaveStateFrequency(autoSaveStateFrequency), lastSaveTime(0) {}

void OptimizerState::initialize()
{
  // usefull if Optimizer is "restarted" with an existing OptimizerState (in pogress evaluations are lost)
  if (totalNumberOfRequests > totalNumberOfResults)
    totalNumberOfRequests = totalNumberOfResults;
}

void OptimizerState::autoSaveToFile(ExecutionContext& context, bool force)
{
  ScopedLock _(lock);
  double time = Time::getMillisecondCounter() / 1000.0;
  if (autoSaveStateFrequency > 0.0 && (force || (time - lastSaveTime >= autoSaveStateFrequency)))
  {
    lastSaveTime = time;
    if (context.getFile(T("optimizerState.xml")).existsAsFile())
      context.getFile(T("optimizerState.xml")).copyFileTo(context.getFile(T("optimizerState_backup.xml")));
    saveToFile(context, context.getFile(T("optimizerState.xml")));
  }
}

// Requests
size_t OptimizerState::getTotalNumberOfRequests() const
{
  ScopedLock _(lock);
  return totalNumberOfRequests;
}

void OptimizerState::incTotalNumberOfRequests()
{
  ScopedLock _(lock);
  totalNumberOfRequests++;
}

size_t OptimizerState::getTotalNumberOfResults() const
{
  ScopedLock _(lock);
  return totalNumberOfResults;
}

size_t OptimizerState::getNumberOfInProgressEvaluations() const
{
  ScopedLock _(lock);
  return getTotalNumberOfRequests() - getTotalNumberOfResults();
}

// Processeded requests
size_t OptimizerState::getNumberOfProcessedRequests() const
{
  ScopedLock _(lock);
  return processedRequests.size();
}

const std::vector< std::pair<double, Variable> >& OptimizerState::getProcessedRequests() const
{
  ScopedLock _(lock);
  return processedRequests;
}

void OptimizerState::flushProcessedRequests()
{
  ScopedLock _(lock);
  processedRequests.clear();
}

void OptimizerState::flushFirstProcessedRequests(size_t nb) 
{
  ScopedLock _(lock);
  processedRequests.erase(processedRequests.begin(), processedRequests.begin()+nb);
}

void OptimizerState::flushLastProcessedRequests(size_t nb)
{
  ScopedLock _(lock);
  processedRequests.erase(processedRequests.end()-nb, processedRequests.end());
}

// Best variable and score
const Variable& OptimizerState::getBestVariable() const
{
  ScopedLock _(lock);
  return bestVariable;
}

void OptimizerState::setBestVariable(const Variable& variable)
{
  ScopedLock _(lock);
  bestVariable = variable;
}

double OptimizerState::getBestScore() const
{
  ScopedLock _(lock);
  return bestScore;
}

void OptimizerState::setBestScore(double score)
{
  ScopedLock _(lock);
  bestScore = score;
}

// Critical Section
const CriticalSection& OptimizerState::getLock() const
  {return lock;}

// FunctionCallback
void OptimizerState::functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output) 
{
  ScopedLock _(lock);
  if (!output.isConvertibleToDouble())
  {
    context.warningCallback(T("OptimizerState::functionReturned"), T("Return value is not convertible to double"));
    processedRequests.push_back(std::make_pair(DBL_MAX, inputs[0]));  // DBL_MAX -> don't polute optimizer
  }
  else
    processedRequests.push_back(std::make_pair(output.toDouble(), inputs[0]));  // push into buffer
  totalNumberOfResults++;  
}

/*
 ** OptimizerContext
 */
OptimizerContext::OptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction, size_t timeToSleep)
  : context(context), objectiveFunction(objectiveFunction), validationFunction(validationFunction), timeToSleep(timeToSleep)
{
  jassert(objectiveFunction->getNumRequiredInputs() == 1);
  jassert(!validationFunction || validationFunction->getNumRequiredInputs() == 1);
}

void OptimizerContext::setPostEvaluationCallback(const FunctionCallbackPtr& callback)
  {objectiveFunction->addPostCallback(callback);} // by default the callback is done inside the Function

void OptimizerContext::removePostEvaluationCallback(const FunctionCallbackPtr& callback)
  {objectiveFunction->removePostCallback(callback);}

void OptimizerContext::waitUntilAllRequestsAreProcessed() const 
{
  while (!areAllRequestsProcessed())
    Thread::sleep(timeToSleep); // avoid busy waiting
}

size_t OptimizerContext::getTimeToSleep() const 
  {return timeToSleep;}

