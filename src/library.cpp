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
| Filename: lirbary.cpp                    | LBcpp library functions         |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/library.h>
#include <lbcpp/Core/TypeManager.h>
#include <lbcpp/Core/Library.h>
#include <lbcpp/UserInterface/UserInterfaceManager.h>
using namespace lbcpp;

namespace juce
{
  extern void juce_setCurrentExecutableFileName(const String& filename) throw();
};

namespace lbcpp
{

class LibraryManager
{
public:
  ~LibraryManager()
    {shutdown();}

  void shutdown()
  {
    for (size_t i = 0; i < libraries.size(); ++i)
      if (libraries[i].second)
        juce::PlatformUtilities::freeDynamicLibrary(libraries[i].second);
    libraries.clear();
  }

  bool addLibrary(ExecutionContext& context, const LibraryPtr& library, void* handle)
  {
    if (library->getName().isEmpty())
    {
      context.errorCallback(T("Cannot add an unnamed library"));
      return false;
    }

    for (size_t i = 0; i < libraries.size(); ++i)
      if (libraries[i].first->getName() == library->getName())
      {
        context.errorCallback(T("The library called ") + library->getName() + T(" already exists"));
        return false;
      }

    libraries.push_back(std::make_pair(library, handle));
    return true;
  }

  const LibraryPtr& getLibrary(ExecutionContext& context, const String& name) const
  {
    for (size_t i = 0; i < libraries.size(); ++i)
      if (libraries[i].first->getName() == name)
        return libraries[i].first;

    static LibraryPtr empty;
    context.errorCallback(T("Could not find library ") + name.quoted());
    return empty;
  }

private:
  std::vector< std::pair<LibraryPtr, void* > > libraries;
};

struct ApplicationContext
{
  LibraryManager libraryManager;
  TypeManager typeManager;
  UserInterfaceManager userInterfaceManager;
  ExecutionContextPtr defaultExecutionContext;
};

ApplicationContext* applicationContext = NULL;

extern lbcpp::LibraryPtr coreLibrary;
extern lbcpp::LibraryPtr lbCppLibrary;

}; /* namespace lbcpp */ 

void lbcpp::initialize(const char* executableName)
{
  // juce
  juce::initialiseJuce_NonGUI();
  juce::juce_setCurrentExecutableFileName(String::fromUTF8((const juce::uint8* )executableName));

  // application context
  jassert(!applicationContext);
  applicationContext = new ApplicationContext();
  applicationContext->defaultExecutionContext = defaultConsoleExecutionContext();
  
  // types
  importLibrary(coreLibrary);
  importLibrary(lbCppLibrary);
  topLevelType = anyType = variableType;
}

void lbcpp::deinitialize()
{
  if (applicationContext)
  {
    applicationContext->libraryManager.shutdown();
    applicationContext->typeManager.shutdown();
    applicationContext->userInterfaceManager.shutdown();
    deleteAndZero(applicationContext);
    juce::shutdownJuce_NonGUI();
  }
}

TypeManager& lbcpp::typeManager()
  {jassert(applicationContext); return applicationContext->typeManager;}

UserInterfaceManager& lbcpp::userInterfaceManager()
  {jassert(applicationContext); return applicationContext->userInterfaceManager;}

ExecutionContext& lbcpp::defaultExecutionContext()
  {jassert(applicationContext); return *applicationContext->defaultExecutionContext;}

void lbcpp::setDefaultExecutionContext(ExecutionContextPtr defaultContext)
  {jassert(applicationContext); applicationContext->defaultExecutionContext = defaultContext;}

bool lbcpp::importLibrariesFromDirectory(ExecutionContext& executionContext, const File& directory)
{
  juce::OwnedArray<File> files;
  directory.findChildFiles(files, File::findFiles | File::ignoreHiddenFiles, false, T("*.dll"));
  directory.findChildFiles(files, File::findFiles | File::ignoreHiddenFiles, false, T("*.so"));
  directory.findChildFiles(files, File::findFiles | File::ignoreHiddenFiles, false, T("*.dylib"));
  bool ok = true;
  for (int i = 0; i < files.size(); ++i)
  {
    File file = *files[i];
    executionContext.informationCallback(T("Loading dynamic library ") + file.getFullPathName());
    executionContext.progressCallback((double)i, (double)files.size(), T("Dynamic Libraries"));
    ok |= importLibraryFromFile(executionContext, file);
  }
  return ok;
}

LibraryPtr lbcpp::importLibraryFromFile(ExecutionContext& context, const File& file)
{
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
    context.errorCallback(T("Load ") + file.getFileName(), T("Could not find initialize function"));
    juce::PlatformUtilities::freeDynamicLibrary(handle);
    return LibraryPtr();
  }

  LibraryPtr res = (*initializeFunction)(*applicationContext);
  if (!res)
  {
    context.errorCallback(T("Load ") + file.getFileName(), T("Could not find create library"));
    return LibraryPtr();
  }

  if (!importLibrary(context, res))
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
  if (!applicationContext->libraryManager.addLibrary(context, library, dynamicLibraryHandle))
    return false;
  library->cacheTypes(context);
  typeManager().finishDeclarations(context);
  return true;
}

namespace lbcpp
{
  extern void coreLibraryCacheTypes(ExecutionContext& context);
  extern void lbCppLibraryCacheTypes(ExecutionContext& context);
};

// called from dynamic library
void lbcpp::initializeDynamicLibrary(lbcpp::ApplicationContext& applicationContext)
{
  lbcpp::applicationContext = &applicationContext;
  ExecutionContext& context = defaultExecutionContext();

  coreLibraryCacheTypes(context);
  lbCppLibraryCacheTypes(context);
  /*
  LibraryPtr library = coreLibrary;//applicationContext.libraryManager.getLibrary(context, T("Core"));
  jassert(library);
  library->cacheTypes(context);

  library = lbCppLibrary;//applicationContext.libraryManager.getLibrary(context, T("LBCpp"));
  jassert(library);
  library->cacheTypes(context);*/
}

void lbcpp::deinitializeDynamicLibrary()
{
  jassert(lbcpp::applicationContext);
  lbcpp::applicationContext = NULL;
}
