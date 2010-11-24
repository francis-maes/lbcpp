/*-----------------------------------------.---------------------------------.
| Filename: WorkUnit.h                     | Base class for Work Units       |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 18:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_WORK_UNIT_H_
# define LBCPP_EXECUTION_WORK_UNIT_H_

# include "../Data/Object.h"
# include "predeclarations.h"

namespace lbcpp
{

class WorkUnit : public NameableObject
{
public:
  WorkUnit(const String& name) : NameableObject(name) {}
  WorkUnit() {}

protected:
  friend class ExecutionContext;

  virtual bool run(ExecutionContext& context) = 0;
};

typedef ReferenceCountedObjectPtr<WorkUnit> WorkUnitPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_WORK_UNIT_H_
