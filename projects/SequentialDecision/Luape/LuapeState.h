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
  LuapeState(const LuaState& state, const String& name = "Unnamed")
    : DecisionProblemState(name), coroutine(state.newThread()), rewards(0.0)
  {
    int n = state.getTop();
    for (int i = 1; i <= n; ++i)
      coroutine.pushValueFrom(state, i);
  }
  LuapeState() {}

  virtual TypePtr getActionType() const
    {return variableType;}

  virtual bool isFinalState() const
    {return !coroutine.exists();}

  virtual ContainerPtr getAvailableActions() const
    {return actions;}

  virtual String toShortString() const
  {
    LuaState coroutine(this->coroutine);
    lua_Debug ar;
    memset(&ar, 0, sizeof (lua_Debug));
    if (lua_getstack(coroutine, 2, &ar) == 0)
      return "error, could not getstack";
    lua_getinfo(coroutine, "Sl", &ar);
    String res;
    if (ar.currentline > 0)
      res += "Line " + String(ar.currentline) + "\n";
    for (int i = 1; true; ++i)
    {
      const char* name = lua_getlocal(coroutine, &ar, i);
      if (!name)
        break;
      Variable value = coroutine.checkVariable(-1);
      coroutine.pop();
      res += String(name) + " = " + value.toShortString() + "\n";
    }
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    coroutine.setTop(0);
    coroutine.pushVariable(action);

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
    target->coroutine = coroutine.cloneThread();
    if (target->coroutine.isTable(2))
      target->actions = new LuaWrapperVector(target->coroutine, 2);
    target->returnValues = returnValues;
    target->rewards = rewards;
  }
  
  static int create(LuaState& state)
  {
    LuapeStatePtr res(new LuapeState(state));
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
    coroutine.clear();
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
