/*
 *  Optimizer.cpp
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 11/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

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
  optimizerContext->setPostEvaluationCallback((FunctionCallbackPtr) optimizerState.get());  // TODO arnaud : casting OK ?
  Variable output = optimize(context, optimizerContext, optimizerState);
  optimizerContext->removePostEvaluationCallback((FunctionCallbackPtr) optimizerState.get()); // TODO arnaud : casting OK ?
  return output;
}

/*
 ** OptimizerState
 */
OptimizerState::OptimizerState() : totalNumberOfRequests(0), totalNumberOfEvaluations(0), bestScore(DBL_MAX) {}

// Requests
size_t OptimizerState::getTotalNumberOfRequests() const
  {return totalNumberOfRequests;}

void OptimizerState::incTotalNumberOfRequests()
  {totalNumberOfRequests++;}

size_t OptimizerState::getTotalNumberOfEvaluations() const
  {return totalNumberOfEvaluations;}

// Processeded requests
size_t OptimizerState::getNumberOfProcessedRequests() const
  {return processedRequests.size();}

const std::vector< std::pair<double, Variable> >& OptimizerState::getProcessedRequests() const
  {return processedRequests;}

void OptimizerState::flushProcessedRequests()
  {processedRequests.clear();}

// Best variable and score
const Variable& OptimizerState::getBestVariable() const
  {return bestVariable;}

void OptimizerState::setBestVariable(const Variable& variable)
  {bestVariable = variable;}

double OptimizerState::getBestScore() const
  {return bestScore;}

void OptimizerState::setBestScore(double score)
  {bestScore = score;}

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

OptimizerContext::OptimizerContext() {}

void OptimizerContext::setPostEvaluationCallback(const FunctionCallbackPtr& callback)
  {objectiveFunction->addPostCallback(callback);}

void OptimizerContext::removePostEvaluationCallback(const FunctionCallbackPtr& callback)
  {objectiveFunction->removePostCallback(callback);}
