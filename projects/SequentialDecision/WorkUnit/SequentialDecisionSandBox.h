/*-----------------------------------------.---------------------------------.
| Filename: SequentialDecisionSandBox.h    | Sand Box                        |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

# include "../System/LinearPointPhysicSystem.h"
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class SequentialDecisionSandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    SequentialDecisionSystemPtr system = linearPointPhysicSystem();
    


    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
