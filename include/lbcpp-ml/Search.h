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

class SearchState : public Object
{
public:
  virtual DomainPtr getActionDomain() const = 0;

  virtual void performTransition(ExecutionContext& context, const ObjectPtr& action, Variable* stateBackup = NULL) = 0;
  virtual void undoTransition(ExecutionContext& context, const Variable& stateBackup)
    {jassertfalse;}

  virtual bool isFinalState() const = 0;

  virtual size_t getActionCode(const ObjectPtr& action) const
    {jassertfalse; return 0;}

  virtual ObjectPtr getConstructedObject() const
    {jassertfalse; return ObjectPtr();}

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<SearchState> SearchStatePtr;

class SearchTrajectory;
typedef ReferenceCountedObjectPtr<SearchTrajectory> SearchTrajectoryPtr;

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
    
  virtual int compare(const ObjectPtr& otherObject) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;
  
protected:
  friend class SearchTrajectoryClass;

  std::vector<SearchStatePtr> states;
  std::vector<ObjectPtr> actions;
  SearchStatePtr finalState;
};

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

typedef ReferenceCountedObjectPtr<SearchDomain> SearchDomainPtr;

class SearchSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain);
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

protected:
  SearchDomainPtr domain;
};

typedef ReferenceCountedObjectPtr<SearchSampler> SearchSamplerPtr;

extern SearchSamplerPtr randomSearchSampler();
extern SearchSamplerPtr logLinearActionCodeSearchSampler(double regularizer = 0.1, double learningRate = 1.0);

class SearchAlgorithm : public Solver
{
public:
  virtual void configure(ExecutionContext& context, ProblemPtr problem, SolutionContainerPtr solutions, ObjectPtr initialSolution, Verbosity verbosity);
  virtual void clear(ExecutionContext& context);

protected:
  SearchDomainPtr domain;
  SearchTrajectoryPtr trajectory;
};

typedef ReferenceCountedObjectPtr<SearchAlgorithm> SearchAlgorithmPtr;

extern SearchAlgorithmPtr rolloutSearchAlgorithm();

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

typedef ReferenceCountedObjectPtr<DecoratorSearchAlgorithm> DecoratorSearchAlgorithmPtr;

extern DecoratorSearchAlgorithmPtr stepSearchAlgorithm(SolverPtr algorithm);
extern DecoratorSearchAlgorithmPtr lookAheadSearchAlgorithm(SolverPtr algorithm, double numActions = 1.0);

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SEARCH_H_
