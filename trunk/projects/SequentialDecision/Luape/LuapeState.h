/*-----------------------------------------.---------------------------------.
| Filename: LuapeState.h                   | Lua Program Evolution State     |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 18:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_STATE_H_
# define LBCPP_LUAPE_STATE_H_

# include "../Core/DecisionProblem.h"
# include "../../lua/lua.h" // tmp

namespace lbcpp
{

class LuaWrapperValue : public Object
{
public:
  LuaWrapperValue(const LuaState& state, int reference)
    : state(state), reference(reference) {}
  LuaWrapperValue() : reference(-1) {}

  virtual ~LuaWrapperValue()
    {state.freeReference(reference);}

  int getReference() const
    {return reference;}

protected:
  LuaState state;
  int reference;
};

typedef ReferenceCountedObjectPtr<LuaWrapperValue> LuaWrapperValuePtr;

class LuaWrapperVector : public Container
{
public:
  LuaWrapperVector(const LuaState& state, int index)
    : state(state), index(index) {}
  LuaWrapperVector() : index(-1) {}

  virtual size_t getNumElements() const
    {return state.length(index);}

  virtual Variable getElement(size_t index) const
  {
    LuaState& state = const_cast<LuaWrapperVector* >(this)->state;
    state.pushInteger(index + 1);
    lua_gettable(state, this->index);
    int reference = state.toReference(-1);
    return new LuaWrapperValue(state, reference);
  }

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);} // not implemented yet

protected:
  LuaState state;
  int index;
};


class LuapeAction : public Object
{
public:

};

extern ClassPtr luapeActionClass;

class LuapeState;
typedef ReferenceCountedObjectPtr<LuapeState> LuapeStatePtr;


/* thread status; 0 is OK */
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5

class LuapeState : public DecisionProblemState
{
public:
  LuapeState(LuaState& coroutine, const String& name = "Unnamed")
    : DecisionProblemState(name), coroutine(coroutine), rewards(0.0)
  {
  }
  LuapeState() {}

  virtual TypePtr getActionType() const
    {return luapeActionClass;}

  virtual bool isFinalState() const
    {return !coroutine.exists();}

  virtual ContainerPtr getAvailableActions() const
    {return actions;}

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    LuaWrapperValuePtr value = action.getObjectAndCast<LuaWrapperValue>();
    coroutine.setTop(0);
    coroutine.pushReference(value->getReference());

    actions = ContainerPtr();
    this->rewards = 0.0;
    while (resume(context, false))
      if (actions)
        break;
    reward = this->rewards;
    jassert(actions || isFinalState());
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const LuapeStatePtr& target = t.staticCast<LuapeState>();
    lua_State* L = const_cast<LuapeState* >(this)->coroutine;
    target->coroutine = LuaState(lua_clonethread(L, L));
    if (target->coroutine.isTable(2))
      target->actions = new LuaWrapperVector(target->coroutine, 2);
    target->returnValues = returnValues;
    target->rewards = rewards;
  }
  
  static int create(LuaState& state)
  {
    LuaState coroutine = state.newThread();

    int n = state.getTop();
    for (int i = 1; i <= n; ++i)
      coroutine.pushValueFrom(state, i);

    LuapeStatePtr res(new LuapeState(coroutine));
    res->resume(state.getContext(), true);
    state.pushObject(res);
    return 1;
  }

protected:
  LuaState coroutine;
  ContainerPtr actions;
  VariableVector returnValues;
  double rewards;

  void enterFinalState()
  {
    int n = coroutine.getTop();
    for (int i = 1; i <= n; ++i)
      returnValues.append(coroutine.checkVariable(i));
    coroutine = LuaState(); // FIXME: is this enough to clear the object ??
  }

  void processYield()
  {
    // position 1: the yielded results
    jassert(coroutine.getTop() == 1);

    // position 2: actions
    lua_pushstring(coroutine, "actions");
    lua_gettable(coroutine, 1); // get "actions" container

    // position 3: rewards
    lua_pushstring(coroutine, "reward");
    lua_gettable(coroutine, 1);

    if (coroutine.isTable(2))
      actions = new LuaWrapperVector(coroutine, 2);
    if (coroutine.isNumber(3))
      rewards += coroutine.toNumber(3);
  }

  bool resume(ExecutionContext& context, bool isInitialization)
  {
    int n = coroutine.getTop();
    int status = coroutine.resume(isInitialization ? n - 1 : n);
    switch (status)
    {
    case LUA_YIELD:
      processYield();
      return true;

    case 0:
      enterFinalState();
      coroutine = LuaState();
      return false;

    default:
      coroutine.processExecuteError(status);
      coroutine = LuaState();
      return false;
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_STATE_H_
