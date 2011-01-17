/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTrace.cpp             | Stores an Execution Trace       |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 18:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Execution/ExecutionTrace.h>
#include <lbcpp/Execution/ExecutionStack.h>
#include <lbcpp/Execution/WorkUnit.h>
#include <lbcpp/Core/XmlSerialisation.h>
using namespace lbcpp;

/*
** ExecutionTraceItem
*/
void ExecutionTraceItem::saveToXml(XmlExporter& exporter) const
{
  exporter.setAttribute(T("time"), time);
}

/*
** MessageExecutionTraceItem
*/
MessageExecutionTraceItem::MessageExecutionTraceItem(double time, ExecutionMessageType messageType, const String& what, const String& where)
  : ExecutionTraceItem(time), messageType(messageType), what(what), where(where) {}

String MessageExecutionTraceItem::toString() const
  {return what + (where.isEmpty() ? String::empty : (T(" (in ") + where + T(")")));}

String MessageExecutionTraceItem::getPreferedXmlTag() const
{
  switch (messageType)
  {
  case informationMessageType: return T("info");
  case warningMessageType:     return T("warning");
  case errorMessageType:       return T("error");
  }
  jassert(false);
  return String::empty;
} 

String MessageExecutionTraceItem::getPreferedIcon() const
{
  switch (messageType)
  {
  case informationMessageType: return T("Information-32.png");
  case warningMessageType:     return T("Warning-32.png");
  case errorMessageType:       return T("Error-32.png");
  }
  jassert(false);
  return String::empty;
} 

void MessageExecutionTraceItem::saveToXml(XmlExporter& exporter) const
{
  ExecutionTraceItem::saveToXml(exporter);
  exporter.setAttribute(T("what"), what);
  if (where.isNotEmpty())
    exporter.setAttribute(T("where"), where);
}

/*
** ExecutionTraceNode
*/
ExecutionTraceNode::ExecutionTraceNode(const String& description, const WorkUnitPtr& workUnit, double startTime)
  : ExecutionTraceItem(startTime), description(description), workUnit(workUnit), timeLength(0.0)
{
}

String ExecutionTraceNode::toString() const
  {return description;}

String ExecutionTraceNode::getPreferedIcon() const
  {return T("WorkUnit-32.png");}

void ExecutionTraceNode::appendSubItem(const ExecutionTraceItemPtr& item)
{
  ScopedLock _(subItemsLock);
  subItems.push_back(item);
}

ExecutionTraceNodePtr ExecutionTraceNode::findSubNode(const String& description, const WorkUnitPtr& workUnit) const
{
  ScopedLock _(subItemsLock);
  for (size_t i = 0; i < subItems.size(); ++i)
  {
    ExecutionTraceNodePtr res = subItems[i].dynamicCast<ExecutionTraceNode>();
    if (res)
    {
      if (workUnit)
      {
        jassert(res->getWorkUnit());
        if (res->getWorkUnit() == workUnit)
          return res;
      }
      else
      {
        if (res->toString() == description)
          return res;
      }
    }
  }
  return ExecutionTraceNodePtr();
}

void ExecutionTraceNode::saveToXml(XmlExporter& exporter) const
{
  ExecutionTraceItem::saveToXml(exporter);
  exporter.setAttribute(T("description"), description);
  exporter.setAttribute(T("timeLength"), timeLength);

  // progression
  if (progression)
  {
    exporter.enter(T("progression"));
    progression->saveToXml(exporter);
    exporter.leave();
  }

  // results
  {
    ScopedLock _(resultsLock);
    for (size_t i = 0; i < results.size(); ++i)
    {
      exporter.enter(T("result"));
      exporter.setAttribute(T("name"), results[i].first);
      exporter.saveVariable(T("value"), results[i].second, anyType);
      exporter.leave();
    }
  }

  // sub items
  {
    ScopedLock _(subItemsLock);
    for (size_t i = 0; i < subItems.size(); ++i)
    {
      const ExecutionTraceItemPtr& item = subItems[i];
      exporter.enter(item->getPreferedXmlTag());
      item->saveToXml(exporter);
      exporter.leave();
    }
  }
}

void ExecutionTraceNode::setResult(const String& name, const Variable& value)
{
  ScopedLock _(resultsLock);
  for (size_t i = 0; i < results.size(); ++i)
    if (results[i].first == name)
    {
      results[i].second = value;
      return;
    }
  results.push_back(std::make_pair(name, value));
}

std::vector< std::pair<String, Variable> > ExecutionTraceNode::getResults() const
{
  ScopedLock _(resultsLock);
  return results;
}

ObjectPtr ExecutionTraceNode::getResultsObject(ExecutionContext& context)
{
  ScopedLock _(resultsLock);
  if (results.empty())
    return ObjectPtr();
  bool classHasChanged = false;
  if (!resultsClass)
    resultsClass = new UnnamedDynamicClass(workUnit->getName() + T(" results"));
  std::vector<size_t> variableIndices(results.size());
  for (size_t i = 0; i < results.size(); ++i)
  {
    int index = resultsClass->findObjectVariable(results[i].first);
    if (index < 0)
    {
      resultsClass->addVariable(context, results[i].second.getType(), results[i].first);
      index = (int)resultsClass->getObjectNumVariables() - 1;
      classHasChanged = true;
    }
    variableIndices[i] = (size_t)index;
  }
  if (classHasChanged)
    resultsClass->initialize(context);
  ObjectPtr res = resultsClass->createDenseObject();
  for (size_t i = 0; i < results.size(); ++i)
    res->setVariable(context, variableIndices[i], results[i].second);
  return res;
}


/*
** ExecutionTrace
*/
ExecutionTrace::ExecutionTrace(ExecutionContextPtr context)
  : context(context), root(new ExecutionTraceNode(T("root"), WorkUnitPtr(), 0.0)), startTime(Time::getCurrentTime())
{
}

ExecutionTraceNodePtr ExecutionTrace::findNode(const ExecutionStackPtr& stack) const
{
  jassert(root);
  ExecutionTraceNodePtr res = root;
  size_t d = stack->getDepth();
  for (size_t i = 0; i < d; ++i)
  {
    const std::pair<String, WorkUnitPtr>& entry = stack->getEntry(i);
    res = res->findSubNode(entry.first, entry.second);
    if (!res)
      break;
  }
  return res;
}

void ExecutionTrace::saveToXml(XmlExporter& exporter) const
{
  exporter.setAttribute(T("hostname"), T("FIXME"));
  exporter.setAttribute(T("context"), context->toString());
  exporter.setAttribute(T("startTime"), startTime.toString(true, true, true, true));

  exporter.enter(T("trace"));
  root->saveToXml(exporter);
  exporter.leave();
}
