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
  WorkUnit() : NameableObject(String::empty) {}

  virtual String getName() const
    {return name.isEmpty() ? getClassName() : name;}

  virtual String toShortString() const
    {return getName();}

  virtual String toString() const
    {return getName();}
 
  static int main(ExecutionContext& context, WorkUnitPtr workUnit, int argc, char* argv[]);

  String getUsageString() const;

  bool parseArguments(ExecutionContext& context, const String& arguments);
  bool parseArguments(ExecutionContext& context, const std::vector<String>& arguments);

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExecutionContext;
  friend class DecoratorWorkUnit;

  virtual bool run(ExecutionContext& context) = 0;
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
    : WorkUnit(name), workUnits(new ObjectVector(workUnitClass, initialSize)), progressionUnit(T("Work Units")) {}
  CompositeWorkUnit() {}

  size_t getNumWorkUnits() const
    {return workUnits->getNumElements();}

  const WorkUnitPtr& getWorkUnit(size_t index) const
    {return workUnits->getAndCast<WorkUnit>(index);}

  void setWorkUnit(size_t index, const WorkUnitPtr& workUnit)
    {workUnits->set(index, workUnit);}

  virtual String getProgressionUnit() const
    {return progressionUnit;}

  void setProgressionUnit(const String& progressionUnit)
    {this->progressionUnit = progressionUnit;}

protected:
  friend class CompositeWorkUnitClass;

  ObjectVectorPtr workUnits;
  String progressionUnit;

  virtual bool run(ExecutionContext& context);
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_WORK_UNIT_H_
