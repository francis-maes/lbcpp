/*-----------------------------------------.---------------------------------.
| Filename: WorkUnit.h                     | Base class for Work Units       |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 18:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_WORK_UNIT_H_
# define LBCPP_EXECUTION_WORK_UNIT_H_

# include "predeclarations.h"
# include "../Core/Variable.h"
# include "../Core/Vector.h"

namespace lbcpp
{

class WorkUnit : public NameableObject
{
public:
  WorkUnit(const String& name) : NameableObject(name) {}
  WorkUnit() {}

  virtual String toShortString() const
    {return getName();}

  virtual String toString() const
    {return getName();}
 
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

class CompositeWorkUnit : public WorkUnit
{
public:
  CompositeWorkUnit(const String& name, size_t initialSize)
    : WorkUnit(name), workUnits(new ObjectVector(workUnitClass, initialSize)) {}
  CompositeWorkUnit() {}

  size_t getNumWorkUnits() const
    {return workUnits->getNumElements();}

  const WorkUnitPtr& getWorkUnit(size_t index) const
    {return workUnits->getAndCast<WorkUnit>(index);}

  void setWorkUnit(size_t index, const WorkUnitPtr& workUnit)
    {workUnits->set(index, workUnit);}

protected:
  friend class CompositeWorkUnitClass;

  ObjectVectorPtr workUnits;

  virtual bool run(ExecutionContext& context);
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_WORK_UNIT_H_
