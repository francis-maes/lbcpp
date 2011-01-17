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
using namespace lbcpp;

/*
** MessageExecutionTraceItem
*/
MessageExecutionTraceItem::MessageExecutionTraceItem(double time, ExecutionMessageType messageType, const String& what, const String& where)
  : ExecutionTraceItem(time), messageType(messageType), what(what), where(where) {}

String MessageExecutionTraceItem::toString() const
  {return what + (where.isEmpty() ? String::empty : (T(" (in ") + where + T(")")));}

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

/*
** ExecutionTraceNode
*/
ExecutionTraceNode::ExecutionTraceNode(const String& description, const WorkUnitPtr& workUnit, double startTime)
  : ExecutionTraceItem(startTime), description(description), workUnit(workUnit), endTime(startTime),
    hasProgression(false), progression(0.0) {}

String ExecutionTraceNode::toString() const
  {return description;}

String ExecutionTraceNode::getPreferedIcon() const
  {return T("WorkUnit-32.png");}

ExecutionTraceNodePtr ExecutionTraceNode::findSubNode(const String& description, const WorkUnitPtr& workUnit) const
{
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

void ExecutionTraceNode::setProgression(double progression, double progressionTotal, const String& unit)
{
  progressionString = String(progression);
  if (progressionTotal)
    progressionString += T(" / ") + String(progressionTotal);
  if (unit.isNotEmpty())
    progressionString += T(" ") + unit;
  this->progression = progressionTotal ? progression / progressionTotal : -progression;
  hasProgression = true;
}

void ExecutionTraceNode::setResult(const String& name, const Variable& value)
{
  for (size_t i = 0; i < results.size(); ++i)
    if (results[i].first == name)
    {
      results[i].second = value;
      return;
    }
  results.push_back(std::make_pair(name, value));
}

ObjectPtr ExecutionTraceNode::getResultsObject(ExecutionContext& context)
{
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
  : context(context), root(new ExecutionTraceNode(T("root"), WorkUnitPtr(), 0.0))
{
  startTime = Time::getCurrentTime();
  startTimeMs = Time::getApproximateMillisecondCounter();
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
