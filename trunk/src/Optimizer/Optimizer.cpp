/*
 *  Optimizer.cpp
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 11/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
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
  optimizerContext->setPostEvaluationCallback((FunctionCallbackPtr) optimizerState.get());
  context.enterScope(T("Optimizing ..."));
  Variable output = optimize(context, optimizerContext, optimizerState);
  context.resultCallback(T("bestParameter"), optimizerState->getBestVariable());
  context.leaveScope(optimizerState->getBestScore());
  optimizerContext->removePostEvaluationCallback((FunctionCallbackPtr) optimizerState.get());
  
  context.enterScope(T("Best Score Evolution"));
  {
    ScopedLock _(optimizerState->getLock());  // should be useless
    std::vector< std::pair<double, Variable> >::const_iterator it;
    size_t i = 0;
    for (it = optimizerState->getBestRequests().begin(); it < optimizerState->getBestRequests().end(); it++) {
      context.enterScope(T("Best Score ") + String((int) i));
      context.resultCallback(T("scoreNumber"), i);
      context.resultCallback(T("bestParameter"), it->second);
      context.leaveScope(it->first);
      i++;
    }
  }
  context.leaveScope();
  return output;
}

/*
 ** OptimizerState
 */
OptimizerState::OptimizerState() 
  : totalNumberOfRequests(0), totalNumberOfEvaluations(0) 
  {bestRequests.push_back(std::make_pair(DBL_MAX, Variable()));}  // this element will be erased as soon as an another bestResult is available

// Distribution
const DistributionPtr& OptimizerState::getDistribution() const
  {return distribution;}

void OptimizerState::setDistribution(ExecutionContext& context, const DistributionPtr& newDistribution)
{
  distribution = newDistribution;
  context.informationCallback(T("Distribution updated : ") + distribution->toString());
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
  return bestRequests[bestRequests.size()-1].second;
}

double OptimizerState::getBestScore() const
{
  ScopedLock _(lock);
  return bestRequests[bestRequests.size()-1].first;
}

void OptimizerState::setBestRequest(double score, const Variable& parameter) 
{
  ScopedLock _(lock);
  jassert(getBestScore() > score);
  if (getBestScore() == DBL_MAX) {
    bestRequests.clear();
  }
  bestRequests.push_back(std::make_pair(score, parameter));
}

const std::vector< std::pair<double, Variable> >& OptimizerState::getBestRequests()
{
  ScopedLock _(lock);
  return bestRequests;
}


// Critical Section
const CriticalSection& OptimizerState::getLock() const
  {return lock;}

// FunctionCallback
void OptimizerState::functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output) 
{
  ScopedLock _(lock);
  processedRequests.push_back(std::make_pair(output.toDouble(), inputs[0]));
  context.enterScope(T("Request ") + String((int) totalNumberOfEvaluations));
  context.resultCallback(T("requestNumber"), totalNumberOfEvaluations);
  context.resultCallback(T("parameter"), inputs[0]);      
  context.leaveScope(output.toDouble());
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
