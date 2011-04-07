/*-----------------------------------------.---------------------------------.
| Filename: DamienDecisionProblem.h        | Wrapper for Damien Ernst' code  |
| Author  : Francis Maes                   |                                 |
| Started : 28/03/2011 13:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_DAMIEN_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_DAMIEN_H_

# include "../Core/DecisionProblem.h"
# include <lbcpp/Data/RandomGenerator.h>
# include "Damien/HIV.hpp"
# include "Damien/invertedPendulum.hpp"

namespace lbcpp
{

extern ClassPtr damienStateClass;

class DamienState;
typedef ReferenceCountedObjectPtr<DamienState> DamienStatePtr;

class DamienState : public DecisionProblemState
{
public:
  DamienState(damien::optimalControlProblemPtr problem, const std::vector<double>& initialState)
    : problem(problem), state(initialState) {}
  DamienState() : problem(NULL) {}

  size_t getNumDimensions() const
    {return state.size();}
  
  double getStateDimension(size_t i) const
    {return state[i];}

  virtual TypePtr getActionType() const
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);}

  virtual void performTransition(const Variable& a, double& reward)
  {
    const DenseDoubleVectorPtr& action = a.getObjectAndCast<DenseDoubleVector>();
    ScopedLock _(problemLock);
    problem->PutState(state);
    problem->PutAction(action->getValues());
    problem->Transition();
    reward = problem->GetReward();
    state = problem->GetState();
  }

  virtual bool isFinalState() const
    {return problem->TerminalStateReached();}
 
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const DamienStatePtr& target = t.staticCast<DamienState>();
    ScopedLock _(problemLock);
    target->problem = problem;
    target->state = state;
  }
 
private:
  friend class DamienStateClass;

  CriticalSection problemLock;
  damien::optimalControlProblemPtr problem;
  std::vector<double> state;
};

class HIVDecisionProblemState : public DamienState
{
public:
  HIVDecisionProblemState(const std::vector<double>& initialState)
    : DamienState(new damien::HIV(), initialState)
  {
    ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
    availableActions = new ObjectVector(actionClass, 4);
    DenseDoubleVectorPtr a;
    
    a = new DenseDoubleVector(actionClass, 2, 0.0);
    availableActions->setElement(0, a);

    a = new DenseDoubleVector(actionClass, 2, 0.0);
    a->setValue(0, 0.7);
    availableActions->setElement(1, a);

    a = new DenseDoubleVector(actionClass, 2, 0.0);
    a->setValue(1, 0.3);
    availableActions->setElement(2, a);

    a = new DenseDoubleVector(actionClass, 2, 0.0);
    a->setValue(0, 0.7);
    a->setValue(1, 0.3);
    availableActions->setElement(3, a);
  }

  HIVDecisionProblemState() {}

  virtual ContainerPtr getAvailableActions() const
    {return availableActions;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    DamienState::clone(context, t);
    t.staticCast<HIVDecisionProblemState>()->availableActions = availableActions;
  }

protected:
  ContainerPtr availableActions;
};

typedef ReferenceCountedObjectPtr<HIVDecisionProblemState> HIVDecisionProblemStatePtr;

class HIVInitialStateSampler : public SimpleUnaryFunction
{
public:
  HIVInitialStateSampler()
    : SimpleUnaryFunction(randomGeneratorClass, damienStateClass) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const RandomGeneratorPtr& random = input.getObjectAndCast<RandomGenerator>();
    
    std::vector<double> initialState(6);

    /*
    ** Uninfected state
    *
    initialState[0] = 1000000.0;
    initialState[1] = 3198.0;
    //initialState[2] = initialState[3] = initialState[4] = 0.0;

    initialState[2]=0.0001;
    initialState[3]=0.0001;
    initialState[4]=1.;

    initialState[5] = 10.0;
    */

    // "non-healty" locally stable equilibrium point
    initialState[0] = 163573;
    initialState[1] = 5;
    initialState[2] = 11945;
    initialState[3] = 46;
    initialState[4] = 63919;
    initialState[5] = 24;

    return new HIVDecisionProblemState(initialState);
  }
};

class HIVDecisionProblem : public DecisionProblem
{
public:
  HIVDecisionProblem(double discount = 0.9)
    : DecisionProblem(new HIVInitialStateSampler(), discount) {}
};

extern DecisionProblemPtr hivDecisionProblem(double discount);

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_DAMIEN_H_
