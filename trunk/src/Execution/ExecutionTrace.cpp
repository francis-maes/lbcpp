/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTrace.cpp             | Stores an Execution Trace       |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 18:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Execution/ExecutionTrace.h>
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
  case statusMessageType:      return T("Information-32.png");
  }
  jassert(false);
  return String::empty;
} 

/*
** WorkUnitExecutionTraceItem
*/
WorkUnitExecutionTraceItem::WorkUnitExecutionTraceItem(const WorkUnitPtr& workUnit, double startTime)
  : ExecutionTraceItem(startTime), workUnit(workUnit), endTime(startTime),
    hasProgression(false), progression(0.0) {}

String WorkUnitExecutionTraceItem::toString() const
  {return workUnit ? workUnit->getName() : T("root");}

String WorkUnitExecutionTraceItem::getPreferedIcon() const
  {return T("WorkUnit-32.png");}

void WorkUnitExecutionTraceItem::setProgression(double progression, double progressionTotal, const String& unit)
{
  progressionString = String(progression);
  if (progressionTotal)
    progressionString += T(" / ") + String(progressionTotal);
  if (unit.isNotEmpty())
    progressionString += T(" ") + unit;
  this->progression = progressionTotal ? progression / progressionTotal : -progression;
  hasProgression = true;
}

void WorkUnitExecutionTraceItem::setResult(const String& name, const Variable& value)
{
  for (size_t i = 0; i < results.size(); ++i)
    if (results[i].first == name)
    {
      results[i].second = value;
      return;
    }
  results.push_back(std::make_pair(name, value));
}
