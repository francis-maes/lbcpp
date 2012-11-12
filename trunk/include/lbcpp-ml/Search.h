/*-----------------------------------------.---------------------------------.
| Filename: Search.h                       | Search base classes             |
| Author  : Francis Maes                   |                                 |
| Started : 03/10/2012 19:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SEARCH_H_
# define LBCPP_ML_SEARCH_H_

# include "Sampler.h"
# include "Domain.h"
# include "Solver.h"

namespace lbcpp
{

/*
** State
*/
class SearchState : public Object
{
public:
  virtual DomainPtr getActionDomain() const = 0;

  virtual void performTransition(ExecutionContext& context, const ObjectPtr& action, ObjectPtr* stateBackup = NULL) = 0;
  virtual void undoTransition(ExecutionContext& context, const ObjectPtr& stateBackup)
    {jassertfalse;}

  virtual bool isFinalState() const = 0;

  virtual ObjectPtr getConstructedObject() const
    {jassertfalse; return ObjectPtr();}

  lbcpp_UseDebuggingNewOperator
};

/*
** Trajectory
*/
class SearchTrajectory : public Object
{
public:
  void append(const ObjectPtr& action)
    {actions.push_back(action);}

  void append(const SearchStatePtr& state, const ObjectPtr& action)
    {states.push_back(state); actions.push_back(action);}

  void pop();

  bool areStatesComputed() const;
  void ensureStatesAreComputed(ExecutionContext& context, SearchStatePtr initialState);

  size_t getLength() const
    {return actions.size();}

  SearchStatePtr getState(size_t index) const
    {jassert(index < actions.size()); return states[index];}

  ObjectPtr getAction(size_t index) const
    {jassert(index < actions.size()); return actions[index];}

  void setFinalState(const SearchStatePtr& finalState)
    {this->finalState = finalState;}

  SearchStatePtr getFinalState() const
    {return finalState;}
    
  virtual string toShortString() const;
  virtual int compare(const ObjectPtr& otherObject) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;
  
protected:
  friend class SearchTrajectoryClass;

  std::vector<SearchStatePtr> states;
  std::vector<ObjectPtr> actions;
  SearchStatePtr finalState;
};

/*
** Domain
*/
class SearchDomain : public Domain
{
public:
  SearchDomain(SearchStatePtr initialState = SearchStatePtr())
    : initialState(initialState) {}

  SearchStatePtr getInitialState() const
    {return initialState;}

  SearchStatePtr createInitialState() const
    {return initialState->cloneAndCast<SearchState>();}

protected:
  friend class SearchDomainClass;

  SearchStatePtr initialState;
};

/*
** Objective
*/
class MaximizeSearchTrajectoryLengthObjective : public Objective
{
public:
  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = DBL_MAX;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
    {return (double)object.staticCast<SearchTrajectory>()->getLength();}
};

/*
** Sampler
*/
class SearchSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain);
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;
  virtual ObjectPtr sample(ExecutionContext& context) const;

  virtual ObjectPtr sampleAction(ExecutionContext& context, SearchStatePtr state) const = 0;

protected:
  SearchDomainPtr domain;
};

class SearchActionCodeGenerator : public Object
{
public:
  virtual size_t getCode(const SearchStatePtr& state, const ObjectPtr& action) = 0;
  virtual size_t getNumCodes(const SearchStatePtr& initialState) const
    {return 0;}
};

typedef ReferenceCountedObjectPtr<SearchActionCodeGenerator> SearchActionCodeGeneratorPtr;

extern SearchSamplerPtr randomSearchSampler();
extern SearchSamplerPtr logLinearActionCodeSearchSampler(SearchActionCodeGeneratorPtr codeGenerator, double regularizer = 0.1, double learningRate = 1.0);

/*
** Solver
*/
class SearchAlgorithm : public Solver
{
public:
  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution);
  virtual void stopSolver(ExecutionContext& context);

protected:
  SearchDomainPtr domain;
  SearchTrajectoryPtr trajectory;
};

extern SearchAlgorithmPtr rolloutSearchAlgorithm(SearchSamplerPtr sampler);

class DecoratorSearchAlgorithm : public SearchAlgorithm
{
public:
  DecoratorSearchAlgorithm(SolverPtr algorithm = SolverPtr())
    : algorithm(algorithm) {}
   
protected:
  friend class DecoratorSearchAlgorithmClass;

  SolverPtr algorithm;

  void subSearch(ExecutionContext& context);
};

extern DecoratorSearchAlgorithmPtr stepSearchAlgorithm(SolverPtr algorithm);
extern DecoratorSearchAlgorithmPtr lookAheadSearchAlgorithm(SolverPtr algorithm, double numActions = 1.0);

/*
** SearchNode
*/
class SearchNode : public Object
{
public:
  SearchNode(SearchNode* parent, const SearchStatePtr& state);
  SearchNode();

  SearchNodePtr getSuccessor(ExecutionContext& context, const ObjectPtr& action);

  const SearchStatePtr& getState() const
    {return state;}

  bool isFinalState() const
    {return state->isFinalState();}

  DiscreteDomainPtr getPrunedActionDomain() const;

  lbcpp_UseDebuggingNewOperator

private:
  friend class SearchNodeClass;

  SearchNode* parent;
  SearchStatePtr state;
  std::vector<SearchNodePtr> successors;
  DiscreteDomainPtr actions;
  bool fullyVisited;

  void updateIsFullyVisited();
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SEARCH_H_
