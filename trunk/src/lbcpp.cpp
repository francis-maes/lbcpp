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
| Filename: lbcpp.cpp                      | LBcpp application functions     |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/lbcpp.h>
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
  };

  ApplicationContext globalContext;
  ApplicationContext* applicationContext = NULL;

}; /* namespace lbcpp */ 

void lbcpp::initialize(const char* executableName)
{
  jassert(!applicationContext);
  applicationContext = &globalContext;
  juce::initialiseJuce_NonGUI();
  juce::juce_setCurrentExecutableFileName(String::fromUTF8((const juce::uint8* )executableName));
  typeManager().ensureIsInitialized(*silentExecutionContext);
}

void lbcpp::deinitialize()
{
  if (applicationContext)
  {
    typeManager().shutdown();
    userInterfaceManager().shutdown();
    applicationContext = NULL;
    juce::shutdownJuce_NonGUI();
  }
}

TypeManager& lbcpp::typeManager()
  {jassert(applicationContext); return applicationContext->typeManager;}

UserInterfaceManager& lbcpp::userInterfaceManager()
  {jassert(applicationContext); return applicationContext->userInterfaceManager;}

void lbcpp::initializeDynamicLibrary(lbcpp::ApplicationContext& applicationContext, ExecutionContext& executionContext)
{
  jassert(!lbcpp::context);
  lbcpp::applicationContext = &applicationContext;
}

void lbcpp::deinitializeDynamicLibrary()
{
  jassert(lbcpp::applicationContext);
  lbcpp::applicationContext = NULL;
}
