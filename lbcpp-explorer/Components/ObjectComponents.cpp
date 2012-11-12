/*-----------------------------------------.---------------------------------.
| Filename: ObjectComponents.cpp           | Object Components               |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ContainerSelectorComponent.h"
#include "StringComponent.h"
#include "ObjectBrowser.h"
#include "HexadecimalFileComponent.h"
#include "LuaCodeEditorComponent.h"
#include <lbcpp/library.h>
#include <lbcpp/Core/Library.h>
#include <lbcpp/Core/Loader.h>
using namespace lbcpp;

extern void flushErrorAndWarningMessages(const String& title);

Component* createComponentForObjectImpl(ExecutionContext& context, const ObjectPtr& object, const String& explicitName)
{
  String name = explicitName.isEmpty() ? object->toShortString() : explicitName;

  // no components for null objects
  if (!object)
    return NULL;
  
  if (object.isInstanceOf<NewFile>())
  {
    File file = NewFile::get(object);

    // if it is a directory, display another tree view
    if (file.isDirectory())
      return userInterfaceManager().createObjectTreeView(context, object, name, false, false);

    // if it is a file, try to open it
    ObjectPtr object = Object::createFromFile(context, file);
    flushErrorAndWarningMessages("Load file " + file.getFullPathName());
    if (object)
      return createComponentForObjectImpl(context, object, file.getFileName());
  }

  // try to create a custom UI component
  Component* res = lbcpp::getTopLevelLibrary()->createUIComponentIfExists(context, object, explicitName);
  if (res)
    return res;

  if (object.isInstanceOf<Container>())
  {
    // generic component for containers
    return new ContainerSelectorComponent(object.staticCast<Container>());
  }
  else
  {
    // generic component for objects
    return userInterfaceManager().createObjectTreeView(context, object, name, true, true, false); 
  }
}

Component* lbcpp::createComponentForObject(ExecutionContext& context, const ObjectPtr& object, const String& explicitName, bool topLevelComponent)
{
  if (!object)
    return NULL;
  Component* res = createComponentForObjectImpl(context, object, explicitName);
  if (topLevelComponent && dynamic_cast<ObjectSelector* >(res))
    res = new ObjectBrowser(object, res);
  String title = T("Create Component");
  if (explicitName.isNotEmpty())
    title += T(" for ") + explicitName;
  flushErrorAndWarningMessages(title);
  return res;
}

ObjectPtr lbcpp::createSelectionObject(const std::vector<ObjectPtr>& objects)
{
  if (objects.empty())
    return ObjectPtr();
  if (objects.size() == 1)
    return objects[0];
  ClassPtr baseType = objects[0]->getClass();
  for (size_t i = 0; i < objects.size(); ++i)
    baseType = Class::findCommonBaseClass(baseType, objects[i]->getClass());
  VectorPtr res = vector(baseType);
  res->reserve(objects.size());
  for (size_t i = 0; i < objects.size(); ++i)
    res->append(objects[i]);
  return res;
}
