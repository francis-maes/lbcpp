/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTrace.h               | Stores an Execution Trace       |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 18:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_TRACE_H_
# define LBCPP_EXECUTION_TRACE_H_

# include "ExecutionContext.h"
# include "WorkUnit.h"
# include "../Core/Vector.h"
# include "../Core/Pair.h"
# include "../Core/DynamicObject.h"

namespace lbcpp
{

class ExecutionTraceItem : public Object
{
public:
  ExecutionTraceItem(double time)
    : time(time) {}
  ExecutionTraceItem() {}

  virtual String getPreferedIcon() const = 0;

  double getTime() const
    {return time;}

protected:
  friend class ExecutionTraceItemClass;

  double time;
};

class MessageExecutionTraceItem : public ExecutionTraceItem
{
public:
  MessageExecutionTraceItem(double time, ExecutionMessageType messageType, const String& what, const String& where = String::empty);
  MessageExecutionTraceItem() {}

  virtual String toString() const;
  virtual String getPreferedIcon() const;

protected:
  friend class MessageExecutionTraceItemClass;

  ExecutionMessageType messageType;
  String what;
  String where;
};

class ExecutionTraceNode : public ExecutionTraceItem
{
public:
  ExecutionTraceNode(const String& description, const WorkUnitPtr& workUnit, double startTime);
  ExecutionTraceNode() {}

  virtual String toString() const;
  virtual String getPreferedIcon() const;

  /*
  ** Sub Items
  */
  void appendSubItem(const ExecutionTraceItemPtr& item)
    {subItems.push_back(item);}
  ExecutionTraceNodePtr findSubNode(const String& description, const WorkUnitPtr& workUnit) const;

  /*
  ** Results
  */
  void setResult(const String& name, const Variable& value);

  const std::vector< std::pair<String, Variable> >& getResults() const
    {return results;}

  ObjectPtr getResultsObject(ExecutionContext& context);

  /*
  ** WorkUnit
  */
  // this is only valid during workUnit execution
  // after execution, the pointer is reset in order to save memory
  const WorkUnitPtr& getWorkUnit() const
    {return workUnit;}

  void removeWorkUnit()
    {workUnit = WorkUnitPtr();}

  /*
  ** Progression
  */
  bool isProgressionAvailable() const
    {return hasProgression;}

  double getProgression() const
    {return progression;}

  String getProgressionString() const
    {return progressionString;}

  void setProgression(double progression, double progressionTotal, const String& unit);

  /*
  ** Time
  */
  void setEndTime(double endTime)
    {this->endTime = endTime;}

  double getEndTime() const
    {return endTime;}

  double getTimeLength() const
    {return endTime - time;}

protected:
  friend class ExecutionTraceNodeClass;

  String description;
  WorkUnitPtr workUnit;

  std::vector<ExecutionTraceItemPtr> subItems;

  UnnamedDynamicClassPtr resultsClass;
  std::vector< std::pair<String, Variable> > results;

  double endTime;

  bool hasProgression;
  double progression;
  String progressionString;
};

typedef ReferenceCountedObjectPtr<ExecutionTraceNode> ExecutionTraceNodePtr;

class ExecutionTrace : public Object
{
public:
  ExecutionTrace(ExecutionContextPtr context);
  ExecutionTrace() {}

  virtual juce::Component* createComponent() const;

  ExecutionContext& getContext() const
    {jassert(context); return *context;}

  virtual String toString() const
    {return context->getClassName() + T(" Execution Trace");}

  ExecutionTraceNodePtr findNode(const ExecutionStackPtr& stack) const;

  double getInitialTime() const
    {return startTimeMs / 1000.0;}

protected:
  friend class ExecutionTraceClass;

  ExecutionContextPtr context;
  ExecutionTraceNodePtr root;
  
  Time startTime;
  juce::uint32 startTimeMs;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_H_
