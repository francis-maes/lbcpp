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

// todo: ranger
class FiniteDomain : public Domain
{
public:
  size_t getNumElements() const
    {return elements.size();}

  const ObjectPtr& getElement(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

private:
  std::vector<ObjectPtr> elements;
};

typedef ReferenceCountedObjectPtr<FiniteDomain> FiniteDomainPtr;

///////////

class SearchState : public Object
{
public:
  virtual DomainPtr getActionsDomain() const = 0;

  virtual void performTransition(ExecutionContext& context, const ObjectPtr& action, Variable* stateBackup = NULL) = 0;
  virtual void undoTransition(ExecutionContext& context, const Variable& stateBackup)
    {jassertfalse;}

  virtual bool isFinalState() const = 0;

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<SearchState> SearchStatePtr;

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
