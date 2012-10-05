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

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<SearchState> SearchStatePtr;

class SearchTrajectory;
typedef ReferenceCountedObjectPtr<SearchTrajectory> SearchTrajectoryPtr;

class SearchTrajectory : public Object
{
public:
  void append(const SearchStatePtr& state, const ObjectPtr& action)
    {states.push_back(state); actions.push_back(action);}

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
  virtual SearchStatePtr createInitialState() const = 0;

  virtual size_t getActionCode(const SearchStatePtr& state, const ObjectPtr& action) const
    {jassertfalse; return 0;}

  virtual DoubleVectorPtr getActionFeatures(const SearchStatePtr& state, const ObjectPtr& action) const
    {jassertfalse; return DoubleVectorPtr();}
};

typedef ReferenceCountedObjectPtr<SearchDomain> SearchDomainPtr;

class SearchSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {this->domain = domain.staticCast<SearchDomain>();}

protected:
  SearchDomainPtr domain;
};

typedef ReferenceCountedObjectPtr<SearchSampler> SearchSamplerPtr;

extern SearchSamplerPtr logLinearActionCodeSearchSampler(double learningRate = 1.0);

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SEARCH_H_
