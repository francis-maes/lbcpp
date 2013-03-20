/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTrace.h               | Stores an Execution Trace       |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 18:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef OIL_EXECUTION_TRACE_H_
# define OIL_EXECUTION_TRACE_H_

# include "ExecutionContext.h"
# include "WorkUnit.h"
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

  virtual string getPreferedXmlTag() const = 0;
  virtual string getPreferedIcon() const = 0;

  double getTime() const
    {return time;}

  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

protected:
  friend class ExecutionTraceItemClass;

  double time;
};

class MessageExecutionTraceItem : public ExecutionTraceItem
{
public:
  MessageExecutionTraceItem(double time, ExecutionMessageType messageType, const string& what, const string& where = string::empty);
  MessageExecutionTraceItem() {}

  virtual string toString() const;
  virtual string toShortString() const;
  virtual string getPreferedXmlTag() const;
  virtual string getPreferedIcon() const;

  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

protected:
  friend class MessageExecutionTraceItemClass;

  ExecutionMessageType messageType;
  string what;
  string where;
};

typedef ReferenceCountedObjectPtr<MessageExecutionTraceItem> MessageExecutionTraceItemPtr;

class ExecutionTraceNode : public ExecutionTraceItem
{
public:
  ExecutionTraceNode(const string& description, const WorkUnitPtr& workUnit, double startTime);
  ExecutionTraceNode() {}

  virtual string toString() const
    {return description;}

  virtual string toShortString() const
    {return description;}

  virtual string getPreferedXmlTag() const
    {return JUCE_T("node");}

  virtual string getPreferedIcon() const;

  /*
  ** Sub Items
  */
  void appendSubItem(const ExecutionTraceItemPtr& item);
  ExecutionTraceNodePtr findSubNode(const string& description, const WorkUnitPtr& workUnit = WorkUnitPtr()) const;
  ExecutionTraceNodePtr findFirstNode() const;
  std::vector<ExecutionTraceItemPtr> getSubItems() const;
  size_t getNumSubItems() const;

  /*
  ** Results
  */
  const ObjectPtr& getReturnValue() const
    {return returnValue;}

  void setReturnValue(const ObjectPtr& value)
    {returnValue = value;}

  void setResult(const string& name, const ObjectPtr& value);
  std::vector< std::pair<string, ObjectPtr> > getResults() const;

  VectorPtr getResultsVector(ExecutionContext& context) const;
  TablePtr getChildrenResultsTable(ExecutionContext& context) const;

  /*
  ** Progression
  */
  void setProgression(const ProgressionStatePtr& progression)
    {this->progression = progression;}

  const ProgressionStatePtr& getProgression() const
    {return progression;}

  /*
  ** Time
  */
  void setEndTime(double endTime)
    {timeLength = endTime - time; jassert(timeLength >= 0.0);}

  double getEndTime() const
    {return time + timeLength;}

  double getTimeLength() const
    {return timeLength;}

  /*
  ** WorkUnit
  ** The work unit is only stored during execution
  ** After execution, the pointer is reset in order to save memory
  */
  const WorkUnitPtr& getWorkUnit() const
    {return workUnit;}

  void removeWorkUnit()
    {workUnit = WorkUnitPtr();}

  virtual void saveToXml(XmlExporter& exporter) const;
  void saveSubItemsToXml(XmlExporter& exporter) const;

  virtual bool loadFromXml(XmlImporter& importer);
  bool loadSubItemsFromXml(XmlImporter& importer);

protected:
  friend class ExecutionTraceNodeClass;

  string description;

  CriticalSection subItemsLock;
  std::vector<ExecutionTraceItemPtr> subItems;

  ObjectPtr returnValue;
  CriticalSection resultsLock;
  std::vector< std::pair<string, ObjectPtr> > results;

  ProgressionStatePtr progression;
  double timeLength;

  WorkUnitPtr workUnit;
};

extern ClassPtr executionTraceNodeClass;
typedef ReferenceCountedObjectPtr<ExecutionTraceNode> ExecutionTraceNodePtr;

class ExecutionTrace : public Object
{
public:
  ExecutionTrace(const string& contextDescription);
  ExecutionTrace() {}

  virtual string toString() const
    {ScopedLock _(lock); return context + JUCE_T(" Execution Trace");}

  ExecutionTraceNodePtr getRootNode() const
    {ScopedLock _(lock); return root;}
  ExecutionTraceNodePtr findNode(const ExecutionStackPtr& stack) const;

  juce::Time getStartTime() const
    {ScopedLock _(lock); return startTime;}

  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

protected:
  friend class ExecutionTraceClass;

  CriticalSection lock;

  string operatingSystem;
  bool is64BitOs;
  size_t numCpus;
  int cpuSpeedInMegaherz;
  int memoryInMegabytes;
  string context;

  ExecutionTraceNodePtr root;

  juce::Time startTime;
  juce::Time saveTime;
};
extern ClassPtr executionTraceClass;  

}; /* namespace lbcpp */

#endif //!OIL_EXECUTION_CONTEXT_H_
