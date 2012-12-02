/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: library.cpp                    | LBcpp library functions         |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/library.h>
#include <lbcpp/Core/ClassManager.h>
#include <lbcpp/Core/Library.h>
#include <lbcpp/Core/RandomGenerator.h>
#include <lbcpp/Data/DoubleVector.h>
#include <lbcpp/Core/DefaultClass.h>

#ifdef LBCPP_USER_INTERFACE
# include <lbcpp/UserInterface/UserInterfaceManager.h>
#endif

using namespace lbcpp;

namespace juce
{
  extern void juce_setCurrentExecutableFileName(const string& filename) throw();
};

namespace lbcpp
{

extern void coreLibraryCacheTypes(ExecutionContext& context);
extern void coreLibraryUnCacheTypes();
extern void lbCppLibraryCacheTypes(ExecutionContext& context);
extern void lbCppLibraryUnCacheTypes();

class TopLevelLibrary : public Library
{
public:
  TopLevelLibrary() : Library("TopLevel") {}
  ~TopLevelLibrary()
    {shutdown();}

  void preShutdown()
  {
    for (size_t i = 0; i < subLibraries.size(); ++i)
    {
      subLibraries[i]->uncacheTypes();
      subLibraries[i] = LibraryPtr();
      void* handle = handles[i];
      if (handle)
      {
        typedef void (*DeinitializeLibraryFunction)();

        DeinitializeLibraryFunction deinitializeFunction = (DeinitializeLibraryFunction)juce::PlatformUtilities::getProcedureEntryPoint(handle, T("lbcppDeinitializeLibrary"));
        if (deinitializeFunction)
          deinitializeFunction();
      }
    }
  }

  void shutdown()
  {
    /*for (size_t i = 0; i < libraries.size(); ++i)
      if (libraries[i].second)
        juce::PlatformUtilities::freeDynamicLibrary(libraries[i].second);*/
    subLibraries.clear();
  }

  bool addLibrary(ExecutionContext& context, const LibraryPtr& library, void* handle)
  {
    if (library->getName().isEmpty())
    {
      context.errorCallback(T("Cannot add an unnamed library"));
      return false;
    }

    for (size_t i = 0; i < subLibraries.size(); ++i)
      if (subLibraries[i]->getName() == library->getName())
      {
        context.errorCallback(T("The library called ") + library->getName() + T(" already exists"));
        return false;
      }

    subLibraries.push_back(library);
    handles.push_back(handle);
    return true;
  }

protected:
  virtual bool initialize(ExecutionContext& context) {return true;}
  virtual void cacheTypes(ExecutionContext& context) {}
  virtual void uncacheTypes() {}

  std::vector<void* > handles;
};

typedef ReferenceCountedObjectPtr<TopLevelLibrary> TopLevelLibraryPtr;

#ifdef LBCPP_DEBUG_OBJECT_ALLOCATION

class MemoryLeakDetector
{
public:
  ~MemoryLeakDetector()
  {
    clear();
  }

  void newObject(Object* object)
  {
    ScopedLock _(objectsMapLock);
    if (object->thisClass)
      object->classNameUnderWhichThisIsKnown = object->thisClass->getName();
    objectsMap[object->classNameUnderWhichThisIsKnown].insert(object);
  }

  void deleteObject(Object* object)
  {
    ScopedLock _(objectsMapLock);
    objectsMap[object->classNameUnderWhichThisIsKnown].erase(object);
  }

  void tryToRenameObject(Object* object)
  {
    ScopedLock _(objectsMapLock);
    jassert(object->classNameUnderWhichThisIsKnown.isEmpty());
    string name = object->getClassName();
    if (name != T("Object"))
    {
      objectsMap[object->classNameUnderWhichThisIsKnown].erase(object);
      object->classNameUnderWhichThisIsKnown = name;
      objectsMap[object->classNameUnderWhichThisIsKnown].insert(object);
    }
  }

  void tryToRenameAllUnnamedObjets()
  {
    ScopedLock _(objectsMapLock);
    std::set<Object* > unnamedObjects = objectsMap[string::empty];
    for (std::set<Object* >::iterator it = unnamedObjects.begin(); it != unnamedObjects.end(); ++it)
      tryToRenameObject(*it);
  }

  typedef std::multimap<int, string> ObjectCountsMap;

  void getSortedCounts(ObjectCountsMap& res, size_t& totalSize)
  {
    ScopedLock _(objectsMapLock);
    for (ObjectsMap::const_iterator it = objectsMap.begin(); it != objectsMap.end(); ++it)
    {
      size_t size = getSizeInBytes(it->second);
      if (size)
      {
        totalSize += size;
        res.insert(std::make_pair(size, it->first));
      }
    }
  }

  void getSortedDeltaCounts(ObjectCountsMap& res)
  {
    jassert(previousCounts.size());
    ScopedLock _(objectsMapLock);
    for (ObjectsMap::const_iterator it = objectsMap.begin(); it != objectsMap.end(); ++it)
    {
      size_t size = getSizeInBytes(it->second);
      int delta = (int)size - (int)previousCounts[it->first];
      if (delta > 0)
        res.insert(std::make_pair(delta, it->first));
    }
  }

  string describe(const string& name, size_t kiloBytes) const
    {return string(kiloBytes / 1024.0, 2) + T(" Kb ") + name + T(" [") + string((int)objectsMap.find(name)->second.size()) + T("]\n");}

  string updateMemoryInformation()
  {
    enum {n = 5};

    tryToRenameAllUnnamedObjets();

    // display most allocated objects
    ObjectCountsMap sortedCounts;
    size_t totalSize = 0;
    getSortedCounts(sortedCounts, totalSize);
    string res = T("Total size: ") + ObjectPtr(new MemorySize(totalSize))->toShortString() + T("\n");
    res += T("Most allocated objects:\n");
    size_t i = 0;
    
    {
      ScopedLock _(objectsMapLock);
      for (ObjectCountsMap::reverse_iterator it = sortedCounts.rbegin(); it != sortedCounts.rend() && i < 20; ++it, ++i)
        res += describe(it->second, it->first);
    }

    // display biggest deltas
    if (previousCounts.size())
    {
      ObjectCountsMap sortedDeltaCounts;
      getSortedDeltaCounts(sortedDeltaCounts);
      if (sortedDeltaCounts.size() && sortedDeltaCounts.rbegin()->first > 0)
      {
        res += T("Biggest allocation increases:\n");
        i = 0;
        ScopedLock _(objectsMapLock);
        for (ObjectCountsMap::reverse_iterator it = sortedDeltaCounts.rbegin(); it != sortedDeltaCounts.rend() && i < 20; ++it, ++i)
          res += describe(it->second, it->first);
      }
      else
        res += T("No allocation increase\n");
    }

    // update previous counts
    previousCounts.clear();
    for (ObjectsMap::const_iterator it = objectsMap.begin(); it != objectsMap.end(); ++it)
      previousCounts[it->first] = getSizeInBytes(it->second);

    return res;
  }

  void clear()
  {
    objectsMap.clear();
    previousCounts.clear();
  }

private:
  typedef std::map<string, std::set<Object* > > ObjectsMap;
  CriticalSection objectsMapLock;
  ObjectsMap objectsMap;
  std::map<string, size_t> previousCounts;

  size_t getSizeInBytes(const std::set<Object*>& objects)
  {
    size_t res = 0;
    for (std::set<Object* >::const_iterator it = objects.begin(); it != objects.end(); ++it)
      res += (*it)->getSizeInBytes(false);
    return res;
    //return objects.size();
  }
};
#endif // !LBCPP_DEBUG_OBJECT_ALLOCATION

struct ApplicationContext
{
  ApplicationContext()
  {
    lbcpp::applicationContext = this;
#ifdef LBCPP_DEBUG_OBJECT_ALLOCATION
    memoryLeakDetector = new MemoryLeakDetector();
#endif
#ifdef LBCPP_USER_INTERFACE
    userInterfaceManager = new UserInterfaceManager();
#endif
  }
  ~ApplicationContext()
  {
#ifdef LBCPP_USER_INTERFACE
    delete userInterfaceManager;
#endif
  }

#ifdef LBCPP_DEBUG_OBJECT_ALLOCATION
  MemoryLeakDetector* memoryLeakDetector;
#endif
  TopLevelLibraryPtr topLevelLibrary;
  ClassManager typeManager;
  ExecutionContextPtr defaultExecutionContext;
#ifdef LBCPP_USER_INTERFACE
  UserInterfaceManager* userInterfaceManager;
#endif
};

ApplicationContext* applicationContext = NULL;

extern lbcpp::LibraryPtr coreLibrary();
extern lbcpp::LibraryPtr lbCppLibrary();

}; /* namespace lbcpp */ 

static void lbcppInitializeGlobals()
{
  simpleDenseDoubleVectorClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleClass);
  simpleSparseDoubleVectorClass = sparseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleClass);
}

void lbcpp::initialize(const char* executableName)
{
  //_crtBreakAlloc = 13146;

  // juce
  juce::initialiseJuce_NonGUI();
  // FIXME:
  //juce::juce_setCurrentExecutableFileName(string::fromUTF8((const juce::uint8* )executableName));

  // application context
  jassert(!applicationContext);
  applicationContext = new ApplicationContext();
  applicationContext->defaultExecutionContext = defaultConsoleExecutionContext(true);
  applicationContext->topLevelLibrary = new TopLevelLibrary();
  RandomGenerator::initializeRandomGenerator();
  
  // types
  importLibrary(coreLibrary());
  importLibrary(lbCppLibrary());

  // globals
  lbcppInitializeGlobals();
}

void lbcpp::deinitialize()
{
  if (applicationContext)
  {
    applicationContext->defaultExecutionContext = ExecutionContextPtr();

#ifdef LBCPP_DEBUG_OBJECT_ALLOCATION
    deleteAndZero(applicationContext->memoryLeakDetector);
#endif

    // pre shutdown types
    applicationContext->topLevelLibrary->preShutdown();
    coreLibraryUnCacheTypes();
    lbCppLibraryUnCacheTypes();
    simpleDenseDoubleVectorClass = ClassPtr();
    simpleSparseDoubleVectorClass = ClassPtr();

    // shutdown types
    applicationContext->typeManager.shutdown();

    applicationContext->topLevelLibrary->shutdown();
    deleteAndZero(applicationContext);
    juce::shutdownJuce_NonGUI();
  }
}

ClassManager& lbcpp::typeManager()
  {jassert(applicationContext); return applicationContext->typeManager;}

#ifdef LBCPP_USER_INTERFACE
UserInterfaceManager& lbcpp::userInterfaceManager()
  {jassert(applicationContext); return *applicationContext->userInterfaceManager;}
#endif

ExecutionContext& lbcpp::defaultExecutionContext()
  {jassert(applicationContext); return *applicationContext->defaultExecutionContext;}

void lbcpp::setDefaultExecutionContext(ExecutionContextPtr defaultContext)
  {jassert(applicationContext); applicationContext->defaultExecutionContext = defaultContext;}

LibraryPtr lbcpp::getTopLevelLibrary()
  {jassert(applicationContext); return applicationContext->topLevelLibrary;}

size_t lbcpp::getNumLibraries()
  {jassert(applicationContext); return applicationContext->topLevelLibrary->getSubLibraries().size();}
  
LibraryPtr lbcpp::getLibrary(size_t index)
  {jassert(applicationContext); return applicationContext->topLevelLibrary->getSubLibraries()[index];}

bool lbcpp::importLibrariesFromDirectory(ExecutionContext& executionContext, const juce::File& directory)
{
  juce::OwnedArray<juce::File> files;
  directory.findChildFiles(files, juce::File::findFiles | juce::File::ignoreHiddenFiles, false, T("*.dll"));
  directory.findChildFiles(files, juce::File::findFiles | juce::File::ignoreHiddenFiles, false, T("*.so"));
  directory.findChildFiles(files, juce::File::findFiles | juce::File::ignoreHiddenFiles, false, T("*.dylib"));
  ProgressionStatePtr progression(new ProgressionState(0.0, (double)files.size(), T("Dynamic Libraries")));
  for (int i = 0; i < files.size(); ++i)
  {
    juce::File file = *files[i];
    progression->setValue((double)i);
    executionContext.informationCallback(T("Loading dynamic library ") + file.getFullPathName());
    executionContext.progressCallback(progression);
    importLibraryFromFile(executionContext, file);
  }
  return true;
}

LibraryPtr lbcpp::importLibraryFromFile(ExecutionContext& context, const juce::File& file)
{
  if (!file.existsAsFile())
  {
    context.errorCallback(T("File ") + file.getFullPathName() + T(" does not exists"));
    return LibraryPtr();
  }
  void* handle = juce::PlatformUtilities::loadDynamicLibrary(file.getFullPathName());
  if (!handle)
  {
    context.errorCallback(T("Could not open dynamic library ") + file.getFullPathName());
    return LibraryPtr();
  }
 
  typedef Library* (*InitializeLibraryFunction)(lbcpp::ApplicationContext& applicationContext);

  InitializeLibraryFunction initializeFunction = (InitializeLibraryFunction)juce::PlatformUtilities::getProcedureEntryPoint(handle, T("lbcppInitializeLibrary"));
  if (!initializeFunction)
  {
    context.informationCallback(T("Skipping ") + file.getFileName());
    juce::PlatformUtilities::freeDynamicLibrary(handle);
    return LibraryPtr();
  }

  LibraryPtr res = (*initializeFunction)(*applicationContext);
  res->decrementReferenceCounter();
  jassert(res->getReferenceCount() == 1);
  if (!res)
  {
    context.errorCallback(T("Load ") + file.getFileName(), T("Could not find create library"));
    return LibraryPtr();
  }

  if (!importLibrary(context, res, handle))
  {
    juce::PlatformUtilities::freeDynamicLibrary(handle);
    return LibraryPtr();
  }

  return res;
}

bool lbcpp::importLibrary(ExecutionContext& context, LibraryPtr library, void* dynamicLibraryHandle)
{
  if (!library->initialize(context))
    return false;
  if (!applicationContext->topLevelLibrary->addLibrary(context, library, dynamicLibraryHandle))
    return false;
  library->cacheTypes(context);
  typeManager().finishDeclarations(context);
  return true;
}

// called from dynamic library
void lbcpp::initializeDynamicLibrary(lbcpp::ApplicationContext& applicationContext)
{
#ifdef JUCE_WIN32
  lbcpp::applicationContext = &applicationContext;
  ExecutionContext& context = defaultExecutionContext();

  RandomGenerator::initializeRandomGenerator();

  coreLibraryCacheTypes(context);
  lbCppLibraryCacheTypes(context);

  lbcppInitializeGlobals();
#else
  jassert(lbcpp::applicationContext == &applicationContext);
#endif
}

void lbcpp::deinitializeDynamicLibrary()
{
#ifdef JUCE_WIN32
  coreLibraryUnCacheTypes();
  lbCppLibraryUnCacheTypes();
  simpleDenseDoubleVectorClass = ClassPtr();
  simpleSparseDoubleVectorClass = ClassPtr();
  jassert(lbcpp::applicationContext);
  //lbcpp::applicationContext = NULL;
#endif // JUCE_WIN32
}


#ifdef LBCPP_DEBUG_OBJECT_ALLOCATION

Object::Object(ClassPtr thisClass)
  : thisClass(thisClass)
{
  lbcpp::applicationContext->memoryLeakDetector->newObject(this);
}

Object::~Object()
{
  jassert(lbcpp::applicationContext);
  if (lbcpp::applicationContext && lbcpp::applicationContext->memoryLeakDetector)
    lbcpp::applicationContext->memoryLeakDetector->deleteObject(this);
}

void Object::displayObjectAllocationInfo(std::ostream& ostr)
{
  ostr << lbcpp::applicationContext->memoryLeakDetector->updateMemoryInformation() << std::endl;
}

#else

Object::Object(ClassPtr thisClass)
  : thisClass(thisClass) {}

Object::~Object()
{
  jassert(lbcpp::applicationContext);
}

void Object::displayObjectAllocationInfo(std::ostream& ostr)
  {ostr << "No Object Allocation Info, enable LBCPP_DEBUG_OBJECT_ALLOCATION flag in lbcpp/common.h" << std::endl;}

#endif
