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

namespace UserInterfaceData {extern const char* get(const String& fileName, int& size);};

Image* UserInterfaceManager::getImage(const String& fileName)
{
  int size;
  const char* data = UserInterfaceData::get(fileName, size);
  return data ? ImageCache::getFromMemory(data, size) : NULL;
}

Image* UserInterfaceManager::getImage(const String& fileName, int width, int height)
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

juce::Component* UserInterfaceManager::createComponentIfExists(ExecutionContext& context, const ObjectPtr& object, const String& name) const
{
  size_t n = lbcpp::getNumLibraries();
  for (size_t i = 0; i < n; ++i)
  {
    juce::Component* res = lbcpp::getLibrary(i)->createUIComponentIfExists(context, object, name);
    if (res)
      return res;
  }
  return NULL;
}

#include "Component/VariableTreeView.h"
#include "Component/ExecutionTraceTreeView.h"

juce::TreeView* UserInterfaceManager::createVariableTreeView(ExecutionContext& context, const Variable& variable, const String& name,
                                                              bool showTypes, bool showShortSummaries, bool showMissingVariables, bool makeRootNodeVisible) const
{
  return new VariableTreeView(variable, name, VariableTreeOptions(showTypes, showShortSummaries, showMissingVariables, makeRootNodeVisible));
}

juce::TreeView* UserInterfaceManager::createExecutionTraceInteractiveTreeView(ExecutionContext& context, ExecutionTracePtr trace, ExecutionContextPtr traceContext) const
{
  return new ExecutionTraceTreeView(trace, traceContext);
}
