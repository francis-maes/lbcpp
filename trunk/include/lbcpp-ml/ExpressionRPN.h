/*-----------------------------------------.---------------------------------.
| Filename: ExpressionRPN.h                | Reverse Polish Notation         |
| Author  : Francis Maes                   |  Type Search space              |
| Started : 20/11/2011 16:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_RPN_H_
# define LBCPP_ML_EXPRESSION_RPN_H_

# include "Expression.h"
# include "ExpressionUniverse.h"
# include "Search.h"

namespace lbcpp
{
 
class ExpressionRPNSequence : public Object
{
public:
  ExpressionRPNSequence(const std::vector<ObjectPtr>& sequence);
  ExpressionRPNSequence() {}

  static ExpressionRPNSequencePtr fromNode(const ExpressionPtr& node);
  ExpressionPtr toNode(const ExpressionUniversePtr& universe) const;

  static void apply(const ExpressionUniversePtr& universe, std::vector<ExpressionPtr>& stack, const ObjectPtr& element);

  void appendNode(const ExpressionPtr& node);
  void append(const ObjectPtr& action)
    {sequence.push_back(action);}

  size_t getLength() const
    {return sequence.size();}

  const ObjectPtr& getElement(size_t index) const
    {jassert(index < sequence.size()); return sequence[index];}

  bool startsWith(const ExpressionRPNSequencePtr& start) const;

  virtual String toShortString() const;

  std::vector<TypePtr> computeTypeState(const std::vector<TypePtr>& initialState = std::vector<TypePtr>()) const;

private:
  friend class ExpressionRPNSequenceClass;

  std::vector<ObjectPtr> sequence;
};

class ExpressionRPNTypeState : public Object
{
public:
  ExpressionRPNTypeState(size_t depth, const std::vector<TypePtr>& stack, bool yieldable);
  ExpressionRPNTypeState();

  virtual String toShortString() const;

  size_t getDepth() const
    {return depth;}

  const std::vector<TypePtr>& getStack() const
    {return stack;}

  size_t getStackSize() const
    {return stack.size();}

  bool hasPushActions() const
    {return push.size() > 0;}

  const std::vector<std::pair<TypePtr, ExpressionRPNTypeStatePtr> >& getPushActions() const
    {return push;}

  bool hasPushAction(const TypePtr& type) const;
  ExpressionRPNTypeStatePtr getPushTransition(const TypePtr& type) const;

  bool hasApplyActions() const
    {return apply.size() > 0;}

  bool hasApplyAction(const FunctionPtr& function) const;

  const std::vector<std::pair<FunctionPtr, ExpressionRPNTypeStatePtr> >& getApplyActions() const
    {return apply;}

  bool hasYieldAction() const
    {return yieldable;}

  bool hasAnyAction() const
    {return hasPushActions() || hasApplyActions() || hasYieldAction();}

  size_t getStateIndex() const
    {return stateIndex;}

private:
  friend class ExpressionRPNTypeStateClass;
  friend class ExpressionRPNTypeSpace;

  size_t depth;
  std::vector<TypePtr> stack;
  std::vector<std::pair<TypePtr, ExpressionRPNTypeStatePtr> > push;
  std::vector<std::pair<FunctionPtr, ExpressionRPNTypeStatePtr> > apply;
  bool yieldable;
  size_t stateIndex;
  
  size_t numNodeTypesWhenBuilt;
  bool canBePruned;
  bool canBePrunedComputed;
  
  void setPushTransition(const TypePtr& type, const ExpressionRPNTypeStatePtr& nextState);
  void setApplyTransition(const FunctionPtr& function, const ExpressionRPNTypeStatePtr& nextState);
};

class ExpressionRPNTypeSpace : public Object
{
public:
  ExpressionRPNTypeSpace(const ExpressionDomainPtr& domain, const std::vector<TypePtr>& initialState, size_t maxDepth);
  ExpressionRPNTypeSpace() {}

  void pruneStates(ExecutionContext& context, bool verbose);
  void assignStateIndices(ExecutionContext& context);

  ExpressionRPNTypeStatePtr getInitialState() const
    {return initialState;}

  size_t getNumStates() const
    {return states.size();}

  ExpressionRPNTypeStatePtr getState(size_t depth, const std::vector<TypePtr>& stack) const;

  typedef std::pair<size_t, std::vector<TypePtr> > StateKey;
  typedef std::map<StateKey, ExpressionRPNTypeStatePtr> StateMap;

  const StateMap& getStates() const
    {return states;}

private:
  ExpressionRPNTypeStatePtr initialState;
  StateMap states;

  ExpressionRPNTypeStatePtr getOrCreateState(const ExpressionDomainPtr& problem, size_t depth, const std::vector<TypePtr>& stack);
  static void insertType(std::vector<TypePtr>& types, const TypePtr& type);

  void buildSuccessors(const ExpressionDomainPtr& problem, const ExpressionRPNTypeStatePtr& state, std::vector<TypePtr>& nodeTypes, size_t maxDepth);
  void enumerateFunctionVariables(const ExpressionUniversePtr& universe, const FunctionPtr& function, const std::vector<TypePtr>& inputTypes, std::vector<Variable>& variables, size_t variableIndex, std::vector<FunctionPtr>& res);
  void applyFunctionAndBuildSuccessor(const ExpressionDomainPtr& problem, const ExpressionRPNTypeStatePtr& state, const FunctionPtr& function, std::vector<TypePtr>& nodeTypes, size_t maxDepth);
  bool acceptInputTypes(const FunctionPtr& function, const std::vector<TypePtr>& stack) const;
  bool prune(ExpressionRPNTypeStatePtr state); // return true if state is prunable
};

class ExpressionRPNSearchDomain : public SearchDomain
{
public:
  ExpressionRPNSearchDomain(const ExpressionDomainPtr& domain, size_t expressionSize);

  virtual SearchStatePtr createInitialState() const;
  virtual size_t getActionCode(const SearchStatePtr& state, const ObjectPtr& action) const;
  virtual DoubleVectorPtr getActionFeatures(const SearchStatePtr& state, const ObjectPtr& action) const;

protected:
  ExpressionDomainPtr domain;
  ExpressionRPNTypeSpacePtr typeSearchSpace;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_RPN_H_
