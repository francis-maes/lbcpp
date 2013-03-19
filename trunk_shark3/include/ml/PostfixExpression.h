/*-----------------------------------------.---------------------------------.
| Filename: PostfixExpression.h            | Expressions in postfix notation |
| Author  : Francis Maes                   |                                 |
| Started : 20/11/2011 16:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_EXPRESSION_POSTFIX_H_
# define ML_EXPRESSION_POSTFIX_H_

# include "Expression.h"
# include "Search.h"

namespace lbcpp
{
 
class PostfixExpressionSequence : public Object
{
public:
  PostfixExpressionSequence(const std::vector<ObjectPtr>& sequence);
  PostfixExpressionSequence() {}
  
  static PostfixExpressionSequencePtr fromNode(const ExpressionPtr& node);
  ExpressionPtr toNode() const;

  static void apply(std::vector<ExpressionPtr>& stack, const ObjectPtr& element);

  void appendNode(const ExpressionPtr& node);
  void append(const ObjectPtr& action)
    {sequence.push_back(action);}

  size_t getLength() const
    {return sequence.size();}

  const ObjectPtr& getElement(size_t index) const
    {jassert(index < sequence.size()); return sequence[index];}

  bool startsWith(const PostfixExpressionSequencePtr& start) const;

  virtual string toShortString() const;

  std::vector<ClassPtr> computeTypeState(const std::vector<ClassPtr>& initialState = std::vector<ClassPtr>()) const;

private:
  friend class PostfixExpressionSequenceClass;

  std::vector<ObjectPtr> sequence;
};

class PostfixExpressionTypeState : public Object
{
public:
  PostfixExpressionTypeState(size_t depth, const std::vector<ClassPtr>& stack, bool yieldable);
  PostfixExpressionTypeState();

  virtual string toShortString() const;

  size_t getDepth() const
    {return depth;}

  const std::vector<ClassPtr>& getStack() const
    {return stack;}

  size_t getStackSize() const
    {return stack.size();}

  bool hasPushActions() const
    {return push.size() > 0;}

  const std::vector<std::pair<ClassPtr, PostfixExpressionTypeStatePtr> >& getPushActions() const
    {return push;}

  bool hasPushAction(const ClassPtr& type) const;
  PostfixExpressionTypeStatePtr getPushTransition(const ClassPtr& type) const;

  bool hasApplyActions() const
    {return apply.size() > 0;}

  bool hasApplyAction(const FunctionPtr& function) const;

  const std::vector<std::pair<FunctionPtr, PostfixExpressionTypeStatePtr> >& getApplyActions() const
    {return apply;}

  bool hasYieldAction() const
    {return yieldable;}

  bool hasAnyAction() const
    {return hasPushActions() || hasApplyActions() || hasYieldAction();}

  size_t getStateIndex() const
    {return stateIndex;}

private:
  friend class PostfixExpressionTypeStateClass;
  friend class PostfixExpressionTypeSpace;

  size_t depth;
  std::vector<ClassPtr> stack;
  std::vector<std::pair<ClassPtr, PostfixExpressionTypeStatePtr> > push;
  std::vector<std::pair<FunctionPtr, PostfixExpressionTypeStatePtr> > apply;
  bool yieldable;
  size_t stateIndex;
  
  size_t numNodeTypesWhenBuilt;
  bool canBePruned;
  bool canBePrunedComputed;
  
  void setPushTransition(const ClassPtr& type, const PostfixExpressionTypeStatePtr& nextState);
  void setApplyTransition(const FunctionPtr& function, const PostfixExpressionTypeStatePtr& nextState);
};

class PostfixExpressionTypeSpace : public Object
{
public:
  PostfixExpressionTypeSpace(const ExpressionDomainPtr& domain, const std::vector<ClassPtr>& initialState, size_t maxDepth);
  PostfixExpressionTypeSpace() {}

  void pruneStates(ExecutionContext& context, bool verbose);
  void assignStateIndices(ExecutionContext& context);

  PostfixExpressionTypeStatePtr getInitialState() const
    {return initialState;}

  size_t getNumStates() const
    {return states.size();}

  PostfixExpressionTypeStatePtr getState(size_t depth, const std::vector<ClassPtr>& stack) const;

  typedef std::pair<size_t, std::vector<ClassPtr> > StateKey;
  typedef std::map<StateKey, PostfixExpressionTypeStatePtr> StateMap;

  const StateMap& getStates() const
    {return states;}

private:
  PostfixExpressionTypeStatePtr initialState;
  StateMap states;

  PostfixExpressionTypeStatePtr getOrCreateState(const ExpressionDomainPtr& problem, size_t depth, const std::vector<ClassPtr>& stack);
  static void insertType(std::vector<ClassPtr>& types, const ClassPtr& type);

  void buildSuccessors(const ExpressionDomainPtr& problem, const PostfixExpressionTypeStatePtr& state, std::vector<ClassPtr>& nodeTypes, size_t maxDepth);
  void enumerateFunctionVariables(const FunctionPtr& function, const std::vector<ClassPtr>& inputTypes, std::vector<ObjectPtr>& variables, size_t variableIndex, std::vector<FunctionPtr>& res);
  void applyFunctionAndBuildSuccessor(const ExpressionDomainPtr& problem, const PostfixExpressionTypeStatePtr& state, const FunctionPtr& function, std::vector<ClassPtr>& nodeTypes, size_t maxDepth);
  bool acceptInputTypes(const FunctionPtr& function, const std::vector<ClassPtr>& stack) const;
  bool prune(PostfixExpressionTypeStatePtr state); // return true if state is prunable
};

}; /* namespace lbcpp */

#endif // !ML_EXPRESSION_POSTFIX_H_
