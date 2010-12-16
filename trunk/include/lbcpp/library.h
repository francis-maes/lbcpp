/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2010 by Francis Maes, francis.maes@lip6.fr.
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
| Filename: library.h                      | library init/deinit functions   |
| Author  : Francis Maes                   |                                 |
| Started : 15/12/2010 16:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LIBRARY_H_
# define LBCPP_LIBRARY_H_

# include "Core/predeclarations.h"
# include "Execution/ExecutionContext.h"

namespace lbcpp
{
  struct ApplicationContext;
  extern ApplicationContext* applicationContext;

  // called from the main program
  extern void initialize(const char* executableName);
  extern void deinitialize();

  extern bool importLibrariesFromDirectory(ExecutionContext& executionContext, const File& directory);
  extern LibraryPtr importLibraryFromFile(ExecutionContext& executionContext, const File& file);
  extern bool importLibrary(ExecutionContext& executionContext, LibraryPtr library, void* dynamicLibraryHandle = NULL);

  inline bool importLibrariesFromDirectory(const File& directory)
    {return importLibrariesFromDirectory(defaultExecutionContext(), directory);}

  inline LibraryPtr importLibraryFromFile(const File& file)
    {return importLibraryFromFile(defaultExecutionContext(), file);}

  inline bool importLibrary(LibraryPtr library, void* dynamicLibraryHandle = NULL)
    {return importLibrary(defaultExecutionContext(), library, dynamicLibraryHandle);}

 // called from dynamic libraries
  extern void initializeDynamicLibrary(lbcpp::ApplicationContext& applicationContext);
  extern void deinitializeDynamicLibrary();

}; /* namespace lbcpp */

#endif // !LBCPP_LIBRARY_H_
