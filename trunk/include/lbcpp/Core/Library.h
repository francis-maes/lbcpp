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

extern ClassPtr libraryClass;

class Library : public NameableObject
{
public:
  Library(const String& name);
  Library() {}

  virtual ClassPtr getClass() const;

  const std::vector<TypePtr>& getTypes() const
    {return types;}

  std::vector<TypePtr> getTypesInheritingFrom(TypePtr baseType) const;

  const std::vector<TemplateTypePtr>& getTemplateTypes() const
    {return templateTypes;}

  const std::vector<LibraryPtr>& getSubLibraries() const
    {return subLibraries;}
#ifdef LBCPP_USER_INTERFACE
  bool hasUIComponent(TypePtr type) const;
  juce::Component* createUIComponentIfExists(ExecutionContext& context, const ObjectPtr& object, const String& name = String::empty);
#endif
  
  void luaRegister(LuaState& state) const;

  LoaderPtr findLoaderForFile(ExecutionContext& context, const File& file) const;
  LoaderPtr findLoaderForStream(ExecutionContext& context, juce::InputStream& istr) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LibraryClass;
  friend class TopLevelLibrary;
  friend bool importLibrary(ExecutionContext& context, LibraryPtr library, void* handle);
  friend void initializeDynamicLibrary(lbcpp::ApplicationContext& applicationContext);

  std::vector<TypePtr> types;
  std::vector<TemplateTypePtr> templateTypes;
  std::vector<LibraryPtr> subLibraries;
  std::vector<LoaderPtr> fileLoaders;
  
#ifdef LBCPP_USER_INTERFACE
  typedef juce::Component* (*UIComponentConstructor)(const ObjectPtr& object, const String& name);
  std::vector< std::pair<TypePtr, UIComponentConstructor> > uiComponents;
  bool declareUIComponent(ExecutionContext& context, const String& typeName, UIComponentConstructor constructor);
#endif

  virtual bool initialize(ExecutionContext& context) = 0;
  virtual void cacheTypes(ExecutionContext& context) = 0;
  virtual void uncacheTypes() = 0;
  
  bool declareType(ExecutionContext& context, TypePtr type);
  bool declareTemplateType(ExecutionContext& context, TemplateTypePtr templateType);
  bool declareSubLibrary(ExecutionContext& context, LibraryPtr subLibrary);

  void getTypesInheritingFrom(TypePtr baseType, std::vector<TypePtr>& res) const;
};

typedef ReferenceCountedObjectPtr<Library> LibraryPtr;

#ifdef LBCPP_USER_INTERFACE
template<class ComponentClass>
struct MakeUIComponentConstructor
{
  static juce::Component* ctor(const ObjectPtr& object, const String& name)
    {return new ComponentClass(object, name);}
};
#endif
  
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_LIBRARY_H_
