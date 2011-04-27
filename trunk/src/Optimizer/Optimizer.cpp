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
** Optimizer
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
  context.enterScope(T("Optimizing ..."));
  optimizerContext->setPostEvaluationCallback((FunctionCallbackPtr) optimizerState.get());
  Variable output = optimize(context, optimizerContext, optimizerState);
  optimizerContext->removePostEvaluationCallback((FunctionCallbackPtr) optimizerState.get());
  context.resultCallback(T("bestParameter"), optimizerState->getBestVariable());
  context.resultCallback(T("bestScore"), optimizerState->getBestScore());
  context.leaveScope(optimizerState->getBestScore());
  return output;
}

/*
 ** OptimizerState
 */
OptimizerState::OptimizerState() 
  : totalNumberOfRequests(0), totalNumberOfEvaluations(0), bestVariable(Variable()), bestScore(DBL_MAX) {}

// Distribution
const DistributionPtr& OptimizerState::getDistribution() const
  {return distribution;}

void OptimizerState::setDistribution(ExecutionContext& context, const DistributionPtr& newDistribution)
{
  distribution = newDistribution;
  context.resultCallback(T("distribution"), newDistribution);
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

size_t OptimizerState::getTotalNumberOfEvaluations() const
{
  ScopedLock _(lock);
  return totalNumberOfEvaluations;
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
  processedRequests.push_back(std::make_pair(output.toDouble(), inputs[0]));
  totalNumberOfEvaluations++;
}

/*
 ** OptimizerContext
 */
OptimizerContext::OptimizerContext(const FunctionPtr& objectiveFunction)
  : objectiveFunction(objectiveFunction) {jassert(objectiveFunction->getNumRequiredInputs() == 1);}

bool OptimizerContext::evaluate(ExecutionContext& context, const std::vector<Variable>& parametersVector)
{
  bool ok = true;
  for (size_t i = 0; i < parametersVector.size(); ++i)
    ok &= evaluate(context, parametersVector[i]);
  return ok;
}

void OptimizerContext::setPostEvaluationCallback(const FunctionCallbackPtr& callback)
  {objectiveFunction->addPostCallback(callback);}

void OptimizerContext::removePostEvaluationCallback(const FunctionCallbackPtr& callback)
  {objectiveFunction->removePostCallback(callback);}
