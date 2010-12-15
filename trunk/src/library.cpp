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
  struct ApplicationContext
  {
    TypeManager typeManager;
    UserInterfaceManager userInterfaceManager;
    ExecutionContextPtr defaultExecutionContext;
    std::vector<void*> dynamicLibraryHandles;
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
    typeManager().shutdown();
    userInterfaceManager().shutdown();
    std::vector<void* >& handles = applicationContext->dynamicLibraryHandles;
    for (size_t i = 0; i < handles.size(); ++i)
      juce::PlatformUtilities::freeDynamicLibrary(handles[i]);
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

  if (importLibrary(context, res))
  {
    applicationContext->dynamicLibraryHandles.push_back(handle);
    return res;
  }
  else
  {
    juce::PlatformUtilities::freeDynamicLibrary(handle);
    return LibraryPtr();
  }
}

bool lbcpp::importLibrary(ExecutionContext& context, LibraryPtr library)
{
  if (!library->initialize(context))
    return false;
  library->cacheTypes(context);
  typeManager().finishDeclarations(context);
  return true;
}

// called from dynamic library
void lbcpp::initializeDynamicLibrary(lbcpp::ApplicationContext& applicationContext)
{
  lbcpp::applicationContext = &applicationContext;
  ExecutionContext& context = defaultExecutionContext();
  coreLibrary->cacheTypes(context);
  lbCppLibrary->cacheTypes(context);
}

void lbcpp::deinitializeDynamicLibrary()
{
  jassert(lbcpp::applicationContext);
  lbcpp::applicationContext = NULL;
}
