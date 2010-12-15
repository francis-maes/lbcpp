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
| Filename: Library.h                      | Library and Dynamic Library     |
| Author  : Francis Maes                   |                                 |
| Started : 15/12/2010 16:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_LIBRARY_H_
# define LBCPP_CORE_LIBRARY_H_

# include "Type.h"

namespace lbcpp
{

class Library : public NameableObject
{
public:
  const std::vector<TypePtr>& getTypes() const
    {return types;}

  const std::vector<TemplateTypePtr>& getTemplateTypes() const
    {return templateTypes;}

  lbcpp_UseDebuggingNewOperator

private:
  friend class LibraryClass;

  std::vector<TypePtr> types;
  std::vector<TemplateTypePtr> templateTypes;
};

typedef ReferenceCountedObjectPtr<Library> LibraryPtr;

class DynamicLibrary : public Library
{
public:
  DynamicLibrary() : handle(NULL) {}
  virtual ~DynamicLibrary()
    {freeDynamicLibrary();}

  bool loadDynamicLibrary(ExecutionContext& context, const File& file);
  void freeDynamicLibrary();

  bool isLoaded() const
    {return handle != NULL;}

  lbcpp_UseDebuggingNewOperator

private:
  friend class DynamicLibraryClass;
  File file;

  typedef void (*InitializeFunction)(lbcpp::ApplicationContext& applicationContext, lbcpp::ExecutionContext& executionContext);
  typedef void (*DeinitializeFunction)();

  void* handle;
  InitializeFunction initializeFunction;
  DeinitializeFunction deinitializeFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_LIBRARY_H_
