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
# include "../Core.h"

namespace lbcpp
{

class WorkUnit : public Object
{
public:
  //WorkUnit(const string& name) : NameableObject(name) {}
  //WorkUnit() : NameableObject(string::empty) {}

  static int main(ExecutionContext& context, WorkUnitPtr workUnit, int argc, char* argv[]);

  string getUsageString() const;

  bool parseArguments(ExecutionContext& context, const string& arguments, std::vector< std::pair<size_t, ObjectPtr> >& res);
  bool parseArguments(ExecutionContext& context, const std::vector<string>& arguments, std::vector< std::pair<size_t, ObjectPtr> >& res);
  void setArguments(ExecutionContext& context, const std::vector< std::pair<size_t, ObjectPtr> >& arguments);
  
  bool parseArguments(ExecutionContext& context, const string& arguments);
  bool parseArguments(ExecutionContext& context, const std::vector<string>& arguments);

  virtual ObjectPtr run(ExecutionContext& context) = 0;
  
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

typedef ObjectPtr (Object::*ObjectMethod)(ExecutionContext& context); 

class ObjectMethodWorkUnit : public WorkUnit
{
public:
  ObjectMethodWorkUnit(const string& description, ObjectPtr object, ObjectMethod method)
    : object(object), method(method), description(description) {}
 
  virtual string toString() const
    {return description;}

  virtual ObjectPtr run(ExecutionContext& context)
  {
    Object& obj = *object;
    return (obj.*method)(context);
  }

protected:
  ObjectPtr object;
  ObjectMethod method;
  string description;
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

  virtual ObjectPtr run(ExecutionContext& context)
    {return decorated ? decorated->run(context) : ObjectPtr();}
};

class CompositeWorkUnit : public WorkUnit
{
public:
  CompositeWorkUnit(const string& description, size_t initialSize = 0)
    : workUnits(new OVector(workUnitClass, initialSize)), progressionUnit(T("Work Units")), pushChildrenIntoStack(false), description(description) {}
  CompositeWorkUnit() {}

  virtual string toString() const
    {return description;}

  virtual string toShortString() const
    {return description;}

  size_t getNumWorkUnits() const
    {return workUnits->getNumElements();}

  const WorkUnitPtr& getWorkUnit(size_t index) const
    {return workUnits->getAndCast<WorkUnit>(index);}

  void setWorkUnit(size_t index, const WorkUnitPtr& workUnit)
    {workUnits->set(index, workUnit);}

  void addWorkUnit(const WorkUnitPtr& workUnit)
    {workUnits->append(workUnit);}

  virtual string getProgressionUnit() const
    {return progressionUnit;}

  void setProgressionUnit(const string& progressionUnit)
    {this->progressionUnit = progressionUnit;}

  virtual ProgressionStatePtr getProgression(size_t numWorkUnitsDone) const
    {return new ProgressionState((double)numWorkUnitsDone, (double)getNumWorkUnits(), getProgressionUnit());}

  // push into stack
  void setPushChildrenIntoStackFlag(bool value)
    {pushChildrenIntoStack = value;}

  bool hasPushChildrenIntoStackFlag() const
    {return pushChildrenIntoStack;}

protected:
  friend class CompositeWorkUnitClass;

  OVectorPtr workUnits;
  string progressionUnit;
  bool pushChildrenIntoStack;
  string description;

  virtual ObjectPtr run(ExecutionContext& context);
};
  
}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_WORK_UNIT_H_
