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

class WorkUnitVector : public ObjectVector
{
public:
  WorkUnitVector(const String& name, size_t initialSize)
    : ObjectVector(workUnitClass, initialSize), name(name) {}
  WorkUnitVector() {}

  virtual TypePtr getElementsType() const
    {return workUnitClass;}

  virtual String getName() const
    {return name;}

  virtual void setName(const String& name)
    {this->name = name;}

  size_t getNumWorkUnits() const
    {return getNumElements();}

  const WorkUnitPtr& getWorkUnit(size_t index) const
    {return getAndCast<WorkUnit>(index);}

  void setWorkUnit(size_t index, const WorkUnitPtr& workUnit)
    {set(index, workUnit);}

protected:
  friend class WorkUnitVectorClass;

  String name;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_WORK_UNIT_H_
