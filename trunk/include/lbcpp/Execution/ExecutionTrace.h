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

  virtual String getPreferedXmlTag() const = 0;
  virtual String getPreferedIcon() const = 0;

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
  MessageExecutionTraceItem(double time, ExecutionMessageType messageType, const String& what, const String& where = String::empty);
  MessageExecutionTraceItem() {}

  virtual String toString() const;
  virtual String getPreferedXmlTag() const;
  virtual String getPreferedIcon() const;

  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

protected:
  friend class MessageExecutionTraceItemClass;

  ExecutionMessageType messageType;
  String what;
  String where;
};

typedef ReferenceCountedObjectPtr<MessageExecutionTraceItem> MessageExecutionTraceItemPtr;

class ExecutionTraceNode : public ExecutionTraceItem
{
public:
  ExecutionTraceNode(const String& description, const WorkUnitPtr& workUnit, double startTime);
  ExecutionTraceNode() {}

  virtual String toString() const;
  virtual String getPreferedXmlTag() const
    {return T("node");}
  virtual String getPreferedIcon() const;

  /*
  ** Sub Items
  */
  void appendSubItem(const ExecutionTraceItemPtr& item);
  ExecutionTraceNodePtr findSubNode(const String& description, const WorkUnitPtr& workUnit) const;
  std::vector<ExecutionTraceItemPtr> getSubItems() const;
  size_t getNumSubItems() const;

  /*
  ** Results
  */
  Variable getReturnValue() const
    {return returnValue;}

  void setReturnValue(const Variable& value)
    {returnValue = value;}

  void setResult(const String& name, const Variable& value);
  std::vector< std::pair<String, Variable> > getResults() const;

  ObjectPtr getResultsObject(ExecutionContext& context) const;
  VectorPtr getChildrenResultsTable(ExecutionContext& context) const;

  /*
  ** Progression
  */
  void setProgression(const ProgressionStatePtr& progression)
    {this->progression = progression;}

  ProgressionStatePtr getProgression() const
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

  String description;

  CriticalSection subItemsLock;
  std::vector<ExecutionTraceItemPtr> subItems;

  Variable returnValue;
  CriticalSection resultsLock;
  std::vector< std::pair<String, Variable> > results;

  ProgressionStatePtr progression;
  double timeLength;

  WorkUnitPtr workUnit;
};

typedef ReferenceCountedObjectPtr<ExecutionTraceNode> ExecutionTraceNodePtr;

class ExecutionTrace : public Object
{
public:
  ExecutionTrace(ExecutionContextPtr context);
  ExecutionTrace() {}

#ifdef LBCPP_UI
  virtual juce::Component* createComponent() const;
#endif
  
  ExecutionContextPtr getContextPointer() const
    {return contextPointer;}

  virtual String toString() const
    {return context + T(" Execution Trace");}

  ExecutionTraceNodePtr getRootNode() const
    {return root;}
  ExecutionTraceNodePtr findNode(const ExecutionStackPtr& stack) const;

  Time getStartTime() const
    {return startTime;}

  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

protected:
  friend class ExecutionTraceClass;

  ExecutionContextPtr contextPointer;

  String operatingSystem;
  bool is64BitOs;
  size_t numCpus;
  int cpuSpeedInMegaherz;
  int memoryInMegabytes;
  String context;

  ExecutionTraceNodePtr root;

  Time startTime;
  Time saveTime;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_H_
