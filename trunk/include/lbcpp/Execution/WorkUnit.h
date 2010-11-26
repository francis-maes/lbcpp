/*-----------------------------------------.---------------------------------.
| Filename: WorkUnit.h                     | Base class for Work Units       |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 18:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_WORK_UNIT_H_
# define LBCPP_EXECUTION_WORK_UNIT_H_

# include "../Core/Object.h"
# include "predeclarations.h"

namespace lbcpp
{

class WorkUnit : public NameableObject
{
public:
  WorkUnit(const String& name) : NameableObject(name) {}
  WorkUnit() {}

  virtual String toShortString() const
    {return getClassName();}

  virtual String toString() const
    {return T("No description available !");}
 
  static int main(ExecutionContext& context, WorkUnitPtr workUnit, int argc, char* argv[]);

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExecutionContext;
  friend class DecoratorWorkUnit;

  virtual bool run(ExecutionContext& context) = 0;

protected:
  String getUsageString() const;
  bool parseArguments(ExecutionContext& context, const std::vector<String>& arguments);
};

extern ClassPtr workUnitClass;

class DecoratorWorkUnit : public WorkUnit
{
public:
  DecoratorWorkUnit(const String& name, WorkUnitPtr decorated)
    : WorkUnit(name), decorated(decorated) {}
  DecoratorWorkUnit(WorkUnitPtr decorated)
    : WorkUnit(decorated->getName()), decorated(decorated) {}
  DecoratorWorkUnit() {}

protected:
  friend class DecoratorWorkUnitClass;

  WorkUnitPtr decorated;

  virtual bool run(ExecutionContext& context)
    {return !decorated || decorated->run(context);}
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_WORK_UNIT_H_
