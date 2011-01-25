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

class WorkUnit : public Object
{
public:
  //WorkUnit(const String& name) : NameableObject(name) {}
  //WorkUnit() : NameableObject(String::empty) {}

  static int main(ExecutionContext& context, WorkUnitPtr workUnit, int argc, char* argv[]);

  String getUsageString() const;

  bool parseArguments(ExecutionContext& context, const String& arguments, std::vector< std::pair<size_t, Variable> >& res);
  bool parseArguments(ExecutionContext& context, const std::vector<String>& arguments, std::vector< std::pair<size_t, Variable> >& res);
  void setArguments(ExecutionContext& context, const std::vector< std::pair<size_t, Variable> >& arguments);
  
  bool parseArguments(ExecutionContext& context, const String& arguments);
  bool parseArguments(ExecutionContext& context, const std::vector<String>& arguments);

  virtual bool run(ExecutionContext& context) = 0;
  
  virtual size_t getNumRequiredCpus() const
    {return 0;}
  
  virtual size_t getRequiredMemory() const // in Megabytes
    {return 0;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExecutionContext;
  friend class DecoratorWorkUnit;
};

extern ClassPtr workUnitClass;

typedef bool (Object::*ObjectMethod)(ExecutionContext& context); 

class ObjectMethodWorkUnit : public WorkUnit
{
public:
  ObjectMethodWorkUnit(const String& description, ObjectPtr object, ObjectMethod method)
    : description(description), object(object), method(method) {}
 
  virtual String toString() const
    {return description;}

  virtual bool run(ExecutionContext& context)
  {
    Object& obj = *object;
    return (obj.*method)(context);
  }

protected:
  ObjectPtr object;
  ObjectMethod method;
  String description;
};

class DecoratorWorkUnit : public WorkUnit
{
public:
  DecoratorWorkUnit(WorkUnitPtr decorated)
    : decorated(decorated) {}
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
  CompositeWorkUnit(const String& description, size_t initialSize = 0)
    : description(description), workUnits(new ObjectVector(workUnitClass, initialSize)), progressionUnit(T("Work Units")), pushChildrenIntoStack(false) {}
  CompositeWorkUnit() {}

  virtual String toString() const
    {return description;}

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
  
  // push into stack
  void setPushChildrenIntoStackFlag(bool value)
    {pushChildrenIntoStack = value;}

  bool hasPushChildrenIntoStackFlag() const
    {return pushChildrenIntoStack;}

protected:
  friend class CompositeWorkUnitClass;

  ObjectVectorPtr workUnits;
  String progressionUnit;
  bool pushChildrenIntoStack;
  String description;

  virtual bool run(ExecutionContext& context);
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_WORK_UNIT_H_
