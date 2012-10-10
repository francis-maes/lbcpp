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

  void pop()
  {
    if (states.size() == actions.size())
      states.pop_back();
    actions.pop_back();
  }

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
    
  virtual int compare(const ObjectPtr& otherObject) const
    {return finalState->compare(otherObject.staticCast<SearchTrajectory>()->finalState);}

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
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {this->domain = domain.staticCast<SearchDomain>();}
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const ReferenceCountedObjectPtr<SearchSampler>& target = t.staticCast<SearchSampler>();
    target->domain = domain;
  }

protected:
  SearchDomainPtr domain;
};

typedef ReferenceCountedObjectPtr<SearchSampler> SearchSamplerPtr;

extern SearchSamplerPtr randomSearchSampler();
extern SearchSamplerPtr logLinearActionCodeSearchSampler(double regularizer = 0.1, double learningRate = 1.0);

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SEARCH_H_
