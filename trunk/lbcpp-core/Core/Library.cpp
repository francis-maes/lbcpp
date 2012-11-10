/*-----------------------------------------.---------------------------------.
| Filename: Library.cpp                    | Library                         |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 13:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/TypeManager.h>
#include <lbcpp/Core/Library.h>
#include <lbcpp/Core/Loader.h>
#include <lbcpp/Core/Class.h>
#include <lbcpp/library.h>
using namespace lbcpp;

Library::Library(const String& name) : NameableObject(name)
{
}

bool Library::declareType(ExecutionContext& context, TypePtr type)
{
  if (!typeManager().declare(context, type))
    return false;
  types.push_back(type);
  ClassPtr loaderClass = typeManager().findType("Loader");
  if (loaderClass && type->inheritsFrom(loaderClass) && !type.staticCast<Class>()->isAbstract())
    fileLoaders.push_back(Object::create((ClassPtr)type).staticCast<Loader>());
  return true;
}

bool Library::declareTemplateType(ExecutionContext& context, TemplateTypePtr templateType)
{
  if (!typeManager().declare(context, templateType))
    return false;
  templateTypes.push_back(templateType);
  return true;
}

bool Library::declareSubLibrary(ExecutionContext& context, LibraryPtr subLibrary)
{
  if (!subLibrary)
    return true; // this may happen with the "ifdef" directive in the <import> command
  if (!subLibrary->initialize(context))
    return false;
  subLibraries.push_back(subLibrary);
  return true;
}

#ifdef LBCPP_USER_INTERFACE
bool Library::declareUIComponent(ExecutionContext& context, const String& typeName, UIComponentConstructor constructor)
{
  TypePtr type = typeManager().getType(context, typeName);
  if (!type)
    return false;
  uiComponents.push_back(std::make_pair(type, constructor));
  return true;
}

bool Library::hasUIComponent(TypePtr type) const
{
  for (size_t i = 0; i < uiComponents.size(); ++i)
    if (type->inheritsFrom(uiComponents[i].first))
      return true;
  for (size_t i = 0; i < subLibraries.size(); ++i)
    if (subLibraries[i]->hasUIComponent(type))
      return true;
  return false;
}

juce::Component* Library::createUIComponentIfExists(ExecutionContext& context, const ObjectPtr& object, const String& name)
{
  ClassPtr type = object->getClass();
  for (size_t i = 0; i < uiComponents.size(); ++i)
    if (type->inheritsFrom(uiComponents[i].first))
      return uiComponents[i].second(object, name);
  for (size_t i = 0; i < subLibraries.size(); ++i)
  {
    juce::Component* res = subLibraries[i]->createUIComponentIfExists(context, object, name);
    if (res)
      return res;
  }
  return NULL;
}
#endif // LBCPP_USER_INTERFACE

void Library::getTypesInheritingFrom(TypePtr baseType, std::vector<TypePtr>& res) const
{
  for (size_t i = 0; i < types.size(); ++i)
    if (types[i]->inheritsFrom(baseType))
      res.push_back(types[i]);

  for (size_t i = 0; i < subLibraries.size(); ++i)
    subLibraries[i]->getTypesInheritingFrom(baseType, res);
}

std::vector<TypePtr> Library::getTypesInheritingFrom(TypePtr baseType) const
{
  std::vector<TypePtr> res;
  getTypesInheritingFrom(baseType, res);
  return res;
}

ClassPtr Library::getClass() const
  {return libraryClass;}

void Library::luaRegister(LuaState& state) const
{
  for (size_t i = 0; i < types.size(); ++i)
    types[i]->luaRegister(state);
  for (size_t i = 0; i < subLibraries.size(); ++i)
    subLibraries[i]->luaRegister(state);
}

LoaderPtr Library::findLoaderForFile(ExecutionContext& context, const File& file) const
{
  String ext = file.getFileExtension();
  if (ext.startsWithChar('.'))
    ext = ext.substring(1);

  for (size_t i = 0; i < fileLoaders.size(); ++i)
  {
    LoaderPtr loader = fileLoaders[i];
    if (loader->getFileExtensions().indexOf(ext) >= 0 && loader->canUnderstand(context, file))
      return loader;
  }
  for (size_t i = 0; i < subLibraries.size(); ++i)
  {
    LoaderPtr loader = subLibraries[i]->findLoaderForFile(context, file);
    if (loader)
      return loader;
  }
  return LoaderPtr();
}

LoaderPtr Library::findLoaderForStream(ExecutionContext& context, juce::InputStream& istr) const
{
  for (size_t i = 0; i < fileLoaders.size(); ++i)
  {
    LoaderPtr loader = fileLoaders[i];
    if (loader->canUnderstand(context, istr))
      return loader;
  }
  for (size_t i = 0; i < subLibraries.size(); ++i)
  {
    LoaderPtr loader = subLibraries[i]->findLoaderForStream(context, istr);
    if (loader)
      return loader;
  }
  return LoaderPtr();
}
