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
# include "../Core/Vector.h"
# include "../Core/Pair.h"

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

class WorkUnitExecutionTraceItem : public ExecutionTraceItem
{
public:
  WorkUnitExecutionTraceItem(const WorkUnitPtr& workUnit, double startTime);
  WorkUnitExecutionTraceItem() {}

  virtual String toString() const;
  virtual String getPreferedIcon() const;

  const WorkUnitPtr& getWorkUnit() const
    {return workUnit;}

  bool isProgressionAvailable() const
    {return hasProgression;}

  double getProgression() const
    {return progression;}

  String getProgressionString() const
    {return progressionString;}

  void setProgression(double progression, double progressionTotal, const String& unit);

  void setEndTime(double endTime)
    {this->endTime = endTime;}

  double getEndTime() const
    {return endTime;}

  double getTimeLength() const
    {return endTime - time;}

  void setResult(const String& name, const Variable& value);

  const std::vector< std::pair<String, Variable> >& getResults() const
    {return results;}

protected:
  friend class WorkUnitExecutionTraceItemClass;

  WorkUnitPtr workUnit;
  std::vector< std::pair<String, Variable> > results;
  double endTime;

  bool hasProgression;
  double progression;
  String progressionString;
};

class ExecutionTrace : public Object
{
public:
  ExecutionTrace(ExecutionContextPtr context)
    : context(context) {}
  ExecutionTrace() {}

  virtual juce::Component* createComponent() const;

  ExecutionContext& getContext() const
    {jassert(context); return *context;}

  virtual String toString() const
    {return context->getClassName() + T(" Execution Trace");}

protected:
  ExecutionContextPtr context;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_H_
