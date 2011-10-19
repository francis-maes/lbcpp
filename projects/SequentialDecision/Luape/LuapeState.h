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

namespace lbcpp
{

class LuapeAction : public Object
{
public:

};

extern ClassPtr luapeActionClass;

class LuapeState : public DecisionProblemState
{
public:
  LuapeState(const String& name = "Unnamed")
    : DecisionProblemState(name) {}

  virtual TypePtr getActionType() const
    {return luapeActionClass;}

  virtual ContainerPtr getAvailableActions() const
  {
    // FIXME
    return ContainerPtr();
  }

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    // FIXME
  }

};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_STATE_H_
