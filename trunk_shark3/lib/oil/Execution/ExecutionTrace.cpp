/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTrace.cpp             | Stores an Execution Trace       |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 18:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <oil/Execution/ExecutionTrace.h>
#include <oil/Execution/ExecutionStack.h>
#include <oil/Execution/WorkUnit.h>
#include <oil/Core/XmlSerialisation.h>
#include <oil/Core/String.h>
#include <oil/Core/NativeToObject.h>
#include <oil/Core/Table.h>
using namespace lbcpp;

/*
** ExecutionTraceItem
*/
void ExecutionTraceItem::saveToXml(XmlExporter& exporter) const
{
  exporter.setAttribute(JUCE_T("time"), time);
}

bool ExecutionTraceItem::loadFromXml(XmlImporter& importer)
{
  time = importer.getDoubleAttribute(JUCE_T("time"), -1);
  if (time < 0)
  {
    importer.getContext().errorCallback(JUCE_T("Missing time attribute"));
    return false;
  }
  return true;
}

/*
** MessageExecutionTraceItem
*/
MessageExecutionTraceItem::MessageExecutionTraceItem(double time, ExecutionMessageType messageType, const string& what, const string& where)
  : ExecutionTraceItem(time), messageType(messageType), what(what), where(where) {}

string MessageExecutionTraceItem::toString() const
  {return what + (where.isEmpty() ? string::empty : (JUCE_T(" (in ") + where + JUCE_T(")")));}

string MessageExecutionTraceItem::toShortString() const
  {return toString();}

string MessageExecutionTraceItem::getPreferedXmlTag() const
{
  switch (messageType)
  {
  case informationMessageType: return JUCE_T("info");
  case warningMessageType:     return JUCE_T("warning");
  case errorMessageType:       return JUCE_T("error");
  }
  jassert(false);
  return string::empty;
} 

string MessageExecutionTraceItem::getPreferedIcon() const
{
  switch (messageType)
  {
  case informationMessageType: return JUCE_T("Information-32.png");
  case warningMessageType:     return JUCE_T("Warning-32.png");
  case errorMessageType:       return JUCE_T("Error-32.png");
  }
  jassert(false);
  return string::empty;
} 

void MessageExecutionTraceItem::saveToXml(XmlExporter& exporter) const
{
  ExecutionTraceItem::saveToXml(exporter);
  exporter.setAttribute(JUCE_T("what"), what);
  if (where.isNotEmpty())
    exporter.setAttribute(JUCE_T("where"), where);
}

bool MessageExecutionTraceItem::loadFromXml(XmlImporter& importer)
{
  if (!ExecutionTraceItem::loadFromXml(importer))
    return false;

  string tag = importer.getTagName().toLowerCase();
  if (tag == JUCE_T("info"))
    messageType = informationMessageType;
  else if (tag == JUCE_T("warning"))
    messageType = warningMessageType;
  else if (tag == JUCE_T("error"))
    messageType = errorMessageType;
  else
  {
    importer.getContext().errorCallback(JUCE_T("Unrecognized message type: ") + tag);
    return false;
  }

  what = importer.getStringAttribute(JUCE_T("what"));
  where = importer.getStringAttribute(JUCE_T("where"));
  return true;
}

/*
** ExecutionTraceNode
*/
ExecutionTraceNode::ExecutionTraceNode(const string& description, const WorkUnitPtr& workUnit, double startTime)
  : ExecutionTraceItem(startTime), description(description), timeLength(0.0), workUnit(workUnit)
{
  setThisClass(executionTraceNodeClass);
}

string ExecutionTraceNode::getPreferedIcon() const
  {return JUCE_T("WorkUnit-32.png");}

size_t ExecutionTraceNode::getNumSubItems() const
{
  ScopedLock _(subItemsLock);
  return subItems.size();
}

std::vector<ExecutionTraceItemPtr> ExecutionTraceNode::getSubItems() const
{
  ScopedLock _(subItemsLock);
  return subItems;
}

void ExecutionTraceNode::appendSubItem(const ExecutionTraceItemPtr& item)
{
  ScopedLock _(subItemsLock);
  subItems.push_back(item);
}

ExecutionTraceNodePtr ExecutionTraceNode::findFirstNode() const
{
  ScopedLock _(subItemsLock);
  for (size_t i = 0; i < subItems.size(); ++i)
  {
    ExecutionTraceNodePtr res = subItems[i].dynamicCast<ExecutionTraceNode>();
    if (res)
      return res;
  }
  return ExecutionTraceNodePtr();
}

ExecutionTraceNodePtr ExecutionTraceNode::findSubNode(const string& description, const WorkUnitPtr& workUnit) const
{
  ScopedLock _(subItemsLock);
  for (size_t i = 0; i < subItems.size(); ++i)
  {
    ExecutionTraceNodePtr res = subItems[i].dynamicCast<ExecutionTraceNode>();
    if (res)
    {
      if (workUnit && res->getWorkUnit())
      {
        // workUnit comparison
        if (res->getWorkUnit() == workUnit)
          return res;
      }
      else
      {
        // description comparison
        if (res->toString() == description)
          return res;
      }
    }
  }
  return ExecutionTraceNodePtr();
}

void ExecutionTraceNode::saveSubItemsToXml(XmlExporter& exporter) const
{
  // progression
  if (progression)
  {
    exporter.enter(JUCE_T("progression"));
    progression->saveToXml(exporter);
    exporter.leave();
  }

  // return value
  if (returnValue)
  {
    exporter.enter(JUCE_T("return"));
    exporter.writeObject(returnValue, objectClass);
    exporter.leave();
  }

  // results
  {
    ScopedLock _(resultsLock);
    for (size_t i = 0; i < results.size(); ++i)
      if (results[i].second)
      {
        exporter.enter(JUCE_T("result"));
        exporter.setAttribute(JUCE_T("resultName"), results[i].first);
        exporter.writeObject(results[i].second, objectClass);
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

void ExecutionTraceNode::saveToXml(XmlExporter& exporter) const
{
  ScopedLock _1(resultsLock);
  ScopedLock _2(subItemsLock);

  ExecutionTraceItem::saveToXml(exporter);
  exporter.setAttribute(JUCE_T("description"), description);
  exporter.setAttribute(JUCE_T("timeLength"), timeLength);
  saveSubItemsToXml(exporter);
}

bool ExecutionTraceNode::loadSubItemsFromXml(XmlImporter& importer)
{
  ScopedLock _1(resultsLock);
  ScopedLock _2(subItemsLock);

  bool res = true;
  forEachXmlChildElement(*importer.getCurrentElement(), xml)
  {
    string tagName = xml->getTagName();
    importer.enter(xml);
    
    if (tagName == JUCE_T("info") || tagName == JUCE_T("warning") || tagName == JUCE_T("error") || tagName == JUCE_T("node"))
    {
      ExecutionTraceItemPtr item;
      if (tagName == JUCE_T("node"))
        item = new ExecutionTraceNode();
      else
        item = new MessageExecutionTraceItem();
      if (!item->loadFromXml(importer))
        res = false;
      else
        subItems.push_back(item);
    }
    else if (tagName == JUCE_T("progression"))
    {
      jassert(!progression);
      progression = new ProgressionState();
      res &= progression->loadFromXml(importer);
    }
    else if (tagName == JUCE_T("return"))
      returnValue = importer.loadObject(objectClass);
    else if (tagName == JUCE_T("result"))
    {
      string name = importer.getStringAttribute(JUCE_T("resultName"));
      ObjectPtr value = importer.loadObject(objectClass);
      if (value.exists())
        results.push_back(std::make_pair(name, value));
    }

    importer.leave();
  }
  return res;
}

bool ExecutionTraceNode::loadFromXml(XmlImporter& importer)
{
  if (!ExecutionTraceItem::loadFromXml(importer))
    return false;
  description = importer.getStringAttribute(JUCE_T("description"));
  timeLength = importer.getDoubleAttribute(JUCE_T("timeLength"));
  return loadSubItemsFromXml(importer);
}

void ExecutionTraceNode::setResult(const string& name, const ObjectPtr& value)
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

std::vector< std::pair<string, ObjectPtr> > ExecutionTraceNode::getResults() const
{
  std::vector< std::pair<string, ObjectPtr> > res;
  {
    ScopedLock _(resultsLock);
    res = results;
  }
  if (returnValue.exists())
    res.push_back(std::make_pair(JUCE_T("returnValue"), returnValue));
  return res;
}

VectorPtr ExecutionTraceNode::getResultsVector(ExecutionContext& context) const
{
  std::vector< std::pair<string, ObjectPtr> > results = getResults();
  if (results.empty())
    return VectorPtr();
  ClassPtr elementsType = vectorClass(pairClass(stringClass, objectClass));
  return lbcpp::nativeToObject(results, elementsType);
}

TablePtr ExecutionTraceNode::getChildrenResultsTable(ExecutionContext& context) const
{
  ScopedLock _(subItemsLock);

  // (variable name, variable type) -> index in common class
  typedef std::map<string, std::pair<size_t, ObjectPtr> > ColumnsMap;
  ColumnsMap mapping;

  /*
  ** Create columns
  */
  size_t numChildNodes = 0;
  size_t numRows = subItems.size();
  for (size_t i = 0; i < numRows; ++i)
  {
    ExecutionTraceNodePtr childNode = subItems[i].dynamicCast<ExecutionTraceNode>();
    if (!childNode)
      continue;
    ++numChildNodes;
    std::vector< std::pair<string, ObjectPtr> > childResults = childNode->getResults();
    childResults.insert(childResults.begin(), std::make_pair("name", new String(childNode->toShortString())));
    for (size_t j = 0; j < childResults.size(); ++j)
    {
      string name = childResults[j].first;
      ClassPtr type = childResults[j].second->getClass();

      ColumnsMap::iterator it = mapping.find(name);
      if (it == mapping.end())
      {
        size_t index = mapping.size();
        mapping[name] = std::make_pair(index, type);
      }
      else
        it->second.second = Class::findCommonBaseClass(type, it->second.second);
    }
  }

  /*
  ** Create table
  */
  std::vector< std::pair<string, ClassPtr> > columns(mapping.size());
  for (ColumnsMap::const_iterator it = mapping.begin(); it != mapping.end(); ++it)
    columns[it->second.first] = std::make_pair(it->first, it->second.second);
  TablePtr res = new Table(numChildNodes);
  for (size_t i = 0; i < columns.size(); ++i)
    res->addColumn(columns[i].first, columns[i].second);

  /*
  ** Fill table
  */
  size_t index = 0;
  for (size_t i = 0; i < numRows; ++i)
  {
    ExecutionTraceNodePtr childNode = subItems[i].dynamicCast<ExecutionTraceNode>();
    if (!childNode)
      continue;

    std::vector< std::pair<string, ObjectPtr> > childResults = childNode->getResults();
    childResults.insert(childResults.begin(), std::make_pair("name", new String(childNode->toShortString())));
    for (size_t j = 0; j < childResults.size(); ++j)
    {
      string name = childResults[j].first;
      ColumnsMap::iterator it = mapping.find(name);
      jassert(it != mapping.end());
      res->setElement(index, it->second.first, childResults[j].second);
    }
    ++index;
  }
  return res;
}

/*
** ExecutionTrace
*/
ExecutionTrace::ExecutionTrace(const string& contextDescription)
  : root(new ExecutionTraceNode(JUCE_T("root"), WorkUnitPtr(), 0.0)), startTime(juce::Time::getCurrentTime())
{
  ScopedLock _(lock);
  using juce::SystemStats;

  operatingSystem = SystemStats::getOperatingSystemName();
  is64BitOs = SystemStats::isOperatingSystem64Bit();
  numCpus = (size_t)SystemStats::getNumCpus();
  cpuSpeedInMegaherz = SystemStats::getCpuSpeedInMegaherz();
  memoryInMegabytes = SystemStats::getMemorySizeInMegabytes();
  context = contextDescription;
}

ExecutionTraceNodePtr ExecutionTrace::findNode(const ExecutionStackPtr& stack) const
{
  ScopedLock _(lock);

  jassert(root);
  ExecutionTraceNodePtr res = root;
  size_t d = stack->getDepth();
  for (size_t i = 0; i < d; ++i)
  {
    const std::pair<string, WorkUnitPtr>& entry = stack->getEntry(i);
    res = res->findSubNode(entry.first, entry.second);
    if (!res)
      break;
  }
  return res;
}

void ExecutionTrace::saveToXml(XmlExporter& exporter) const
{
  ScopedLock _(lock);

  const_cast<ExecutionTrace* >(this)->saveTime = juce::Time::getCurrentTime();
  exporter.setAttribute(JUCE_T("os"), operatingSystem);
  exporter.setAttribute(JUCE_T("is64bit"), is64BitOs ? JUCE_T("yes") : JUCE_T("no"));
  exporter.setAttribute(JUCE_T("numcpus"), (int)numCpus);
  exporter.setAttribute(JUCE_T("cpuspeed"), cpuSpeedInMegaherz);
  exporter.setAttribute(JUCE_T("memory"), memoryInMegabytes);
  exporter.setAttribute(JUCE_T("context"), context);
  exporter.setAttribute(JUCE_T("startTime"), startTime.toString(true, true, true, true));
  exporter.setAttribute(JUCE_T("saveTime"), saveTime.toString(true, true, true, true));

  root->saveSubItemsToXml(exporter);
}

bool ExecutionTrace::loadFromXml(XmlImporter& importer)
{
  ScopedLock _(lock);

  operatingSystem = importer.getStringAttribute(JUCE_T("os"));
  is64BitOs = importer.getBoolAttribute(JUCE_T("is64bit"));
  numCpus = (size_t)importer.getIntAttribute(JUCE_T("numcpus"));
  cpuSpeedInMegaherz = importer.getIntAttribute(JUCE_T("cpuspeed"));
  memoryInMegabytes = importer.getIntAttribute(JUCE_T("memory"));
  context = importer.getStringAttribute(JUCE_T("context"));
  // FIXME: startTime and saveTime
  
  root = new ExecutionTraceNode(JUCE_T("root"), WorkUnitPtr(), 0.0);
  return root->loadSubItemsFromXml(importer);
}
