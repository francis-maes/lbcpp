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
#include <oil/library.h>
#include <oil/Core/Library.h>
#include <oil/Core/Loader.h>
using namespace lbcpp;

extern void flushErrorAndWarningMessages(const string& title);

Component* createComponentForObjectImpl(ExecutionContext& context, const ObjectPtr& object, const string& explicitName)
{
  string name = explicitName.isEmpty() ? object->toShortString() : explicitName;

  // no components for null objects
  if (!object)
    return NULL;
  
  if (object.isInstanceOf<File>() && !object.isInstanceOf<Directory>())
  {
    // if it is a file, try to open it
    juce::File file = File::get(object);
    ObjectPtr object = Object::createFromFile(context, file);
    flushErrorAndWarningMessages("Load file " + file.getFullPathName());
    return object ? createComponentForObjectImpl(context, object, file.getFileName()) : NULL;
  }

  // try to create a custom UI component
  Component* res = lbcpp::getTopLevelLibrary()->createUIComponentIfExists(context, object, explicitName);
  if (res)
    return res;

  if (object.isInstanceOf<Vector>())
  {
    // generic component for containers
    return new ContainerSelectorComponent(object.staticCast<Vector>());
  }
  else
  {
    // generic component for objects
    return userInterfaceManager().createObjectTreeView(context, object, name, true); 
  }
}

Component* lbcpp::createComponentForObject(ExecutionContext& context, const ObjectPtr& object, const string& explicitName, bool topLevelComponent)
{
  if (!object)
    return NULL;
  Component* res = createComponentForObjectImpl(context, object, explicitName);
  if (topLevelComponent && dynamic_cast<ObjectSelector* >(res))
    res = new ObjectBrowser(object, res);
  string title = T("Create Component");
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
    res->appendElement(objects[i]);
  return res;
}
