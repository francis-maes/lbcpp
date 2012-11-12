/*-----------------------------------------.---------------------------------.
| Filename: UserInterfaceManager.cpp       | User Interface Manager          |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Library.h>
#include <lbcpp/Execution/ExecutionContext.h>
#include <lbcpp/UserInterface/UserInterfaceManager.h>
#include <lbcpp/library.h>
using namespace lbcpp;

using juce::Desktop;
using juce::Image;
using juce::ImageCache;

namespace UserInterfaceData {extern const char* get(const string& fileName, int& size);};

bool UserInterfaceManager::hasImage(const string& fileName) const
  {int size; return UserInterfaceData::get(fileName, size) != NULL;}

Image* UserInterfaceManager::getImage(const string& fileName) const
{
  int size;
  const char* data = UserInterfaceData::get(fileName, size);
  return data ? ImageCache::getFromMemory(data, size) : NULL;
}

Image* UserInterfaceManager::getImage(const string& fileName, int width, int height) const
{
  juce::int64 hashCode = (fileName.hashCode64() * 101 + (juce::int64)width) * 101 + (juce::int64)height;
  Image* res = ImageCache::getFromHashCode(hashCode);
  if (!res)
  {
    res = getImage(fileName);
    if (!res)
      return NULL;
    res = res->createCopy(width, height);
    jassert(res);
    ImageCache::addImageToCache(res, hashCode);
  }
  return res;
}

#include "Component/ObjectTreeView.h"
#include "Component/ExecutionTraceTreeView.h"

juce::TreeView* UserInterfaceManager::createObjectTreeView(ExecutionContext& context, const ObjectPtr& object, const string& name,
                                                              bool showTypes, bool showShortSummaries, bool showMissingVariables, bool makeRootNodeVisible) const
{
  return new ObjectTreeView(object, name, ObjectTreeOptions(showTypes, showShortSummaries, showMissingVariables, makeRootNodeVisible));
}

juce::TreeView* UserInterfaceManager::createExecutionTraceInteractiveTreeView(ExecutionContext& context, ExecutionTracePtr trace, ExecutionContextPtr traceContext) const
{
  return new ExecutionTraceTreeView(trace, "Trace", traceContext);
}
