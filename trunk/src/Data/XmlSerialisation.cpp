/*-----------------------------------------.---------------------------------.
| Filename: XmlSerialisation.cpp           | Xml Import/Export               |
| Author  : Francis Maes                   |                                 |
| Started : 26/08/2010 17:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Data/XmlSerialisation.h>
#include <lbcpp/Data/DynamicObject.h>
using namespace lbcpp;

/*
** XmlExporter
*/
XmlExporter::XmlExporter(const String& rootTag, int version)
{
  root = new XmlElement(rootTag);
  if (version)
    root->setAttribute(T("version"), version);
  currentStack.push_back(root);
}

XmlExporter::~XmlExporter()
  {delete root;}

bool XmlExporter::saveToFile(const File& file, MessageCallback& callback)
{
  flushSave();
  if (file.exists())
  {
    if (file.existsAsFile())
    {
      if (!file.deleteFile())
      {
        callback.errorMessage(T("XmlExporter::saveToFile"), T("Could not delete file ") + file.getFullPathName());
        return false;
      }
    }
    else
    {
      callback.errorMessage(T("XmlExporter::saveToFile"), file.getFullPathName() + T(" is a directory"));
      return false;
    }
  }
  
  if (!root)
  {
    callback.errorMessage(T("XmlExporter::saveToFile"), T("No root xml element in file ") + file.getFullPathName());
    return false;
  }
  bool ok = root->writeToFile(file, String::empty);
  if (!ok)
    callback.errorMessage(T("XmlExporter::saveToFile"), T("Could not write file ") + file.getFullPathName());
  return ok;
}

XmlElement* XmlExporter::getCurrentElement()
  {return currentStack.back();}

void XmlExporter::enter(const String& tagName, const String& name)
{
  XmlElement* elt = new XmlElement(tagName);
  currentStack.push_back(elt);
  writeName(name);
}

void XmlExporter::saveVariable(const String& name, const Variable& variable, TypePtr expectedType)
{
  enter(T("variable"), name);
  writeVariable(variable, expectedType);
  leave();
}

void XmlExporter::saveElement(size_t index, const Variable& variable, TypePtr expectedType)
{
  enter(T("element"));
  setAttribute(T("index"), String((int)index));
  writeVariable(variable, expectedType);
  leave();
}

void XmlExporter::leave()
{
  jassert(currentStack.size() > 1);
  XmlElement* elt = getCurrentElement();
  currentStack.pop_back();
  getCurrentElement()->addChildElement(elt);
}

void XmlExporter::writeName(const String& name)
{
  XmlElement* elt = getCurrentElement();
  if (name.isNotEmpty())
    elt->setAttribute(T("name"), name);
}

void XmlExporter::writeType(TypePtr type)
{
  jassert(type);
  if (type->isUnnamedType())
  {
    enter(T("type"));
    writeVariable(type, TypePtr());
    leave();
  }
  else
    setAttribute(T("type"), type->getName().replaceCharacters(T("<>"), T("[]")));
}

void XmlExporter::writeVariable(const Variable& variable, TypePtr expectedType)
{
  XmlElement* elt = getCurrentElement();
  
  if (variable.isMissingValue())
  {
    writeType(variable.getType());
    elt->setAttribute(T("missing"), T("true"));
  }
  else if (variable.isObject())
  {
    Object* object = variable.getObject().get();
    TypePtr typeValue = dynamic_cast<Type* >(object);
    if (typeValue && !typeValue->isUnnamedType())
      addTextElement(typeValue->getName()); // named type
    else
      writeObject(variable.getObject(), expectedType); // traditional object
  }
  else
  {
    if (variable.getType() != expectedType)
      writeType(variable.getType());
    variable.saveToXml(*this);
  }
}

void XmlExporter::writeObject(const ObjectPtr& object, TypePtr expectedType)
{
  jassert(object->getReferenceCount());
  XmlElement* currentElement = getCurrentElement();

  size_t index;
  SavedObjectsMap::const_iterator it = savedObjectsMap.find(object);
  if (it == savedObjectsMap.end())
  {
    // create index
    index = savedObjects.size();
    savedObjectsMap[object] = index;

    // create SavedObject
    SavedObject savedObject;
    savedObject.object = object;
    savedObject.elt = new XmlElement(T("NOTAGYET"));
    savedObjects.push_back(savedObject);

    // save object
    currentlySavedObjects.insert(index);
    currentStack.push_back(savedObject.elt);
    if (object->getClass() != expectedType)
      writeType(object->getClass());
    object->saveToXml(*this);
    currentStack.pop_back();
    currentlySavedObjects.erase(index);

    savedObjects[index].references.push_back(currentElement);
  }
  else
  {
    index = it->second;
    std::vector<XmlElement* >& references = savedObjects[index].references;
    references.push_back(currentElement);
    if (references.size() == 2)
      sharedObjectsIndices.insert(it->second);
  }

  for (std::set<size_t>::const_iterator it = currentlySavedObjects.begin(); it != currentlySavedObjects.end(); ++it)
    savedObjects[*it].dependencies.insert(index);
}

void XmlExporter::makeSharedObjectsSaveOrder(const std::set<size_t>& sharedObjectsIndices, std::vector<size_t>& res)
{
  std::set<size_t> todo = sharedObjectsIndices;

  res.reserve(todo.size());
  size_t n = todo.size();
  for (size_t iteration = 0; todo.size() && iteration < n; ++iteration)
  {
    std::set<size_t>::const_iterator nxt;
    for (std::set<size_t>::const_iterator it = todo.begin(); it != todo.end(); it = nxt)
    {
      nxt = it; ++nxt;

      bool areAllDependenciesDone = true;
      const std::set<size_t>& dependencies = savedObjects[*it].dependencies;
      for (std::set<size_t>::const_iterator it2 = dependencies.begin(); it2 != dependencies.end(); ++it2)
        if (sharedObjectsIndices.find(*it2) != sharedObjectsIndices.end() && !savedObjects[*it2].ordered)
        {
          areAllDependenciesDone = false;
          break;
        }
      if (areAllDependenciesDone)
      {
        res.push_back(*it);
        savedObjects[*it].ordered = true;
        todo.erase(it);
      }
    }
  }
  
  if (todo.empty())
    return;
  MessageCallback::error(T("XmlExporter::makeSharedObjectsSaveOrder"), T("Cyclic dependancy between shared objects"));
}

void XmlExporter::resolveSingleObjectReference(SavedObject& savedObject)
{
  XmlElement* reference = savedObject.references[0];
  reference->moveChildrenFrom(*savedObject.elt); 
  for (int i = 0; i < savedObject.elt->getNumAttributes(); ++i)
    reference->setAttribute(savedObject.elt->getAttributeName(i), savedObject.elt->getAttributeValue(i));
  deleteAndZero(savedObject.elt);
}

void XmlExporter::resolveSharedObjectReferences(SavedObject& savedObject)
{
  jassert(savedObject.references.size() > 1);
  for (size_t i = 0; i < savedObject.references.size(); ++i)
    savedObject.references[i]->setAttribute(T("reference"), savedObject.identifier);
}

void XmlExporter::flushSave()
{
  // make shared object order
  std::vector<size_t> sharedObjectsOrder;
  makeSharedObjectsSaveOrder(sharedObjectsIndices, sharedObjectsOrder);
  jassert(sharedObjectsOrder.size() == sharedObjectsIndices.size());

  // save all shared objects and solve their references
  std::set<String> sharedObjectIdentifiers;
  for (int i = sharedObjectsOrder.size() - 1; i >= 0; --i)
  {
    SavedObject& savedObject = savedObjects[sharedObjectsOrder[i]];
    savedObject.elt->setTagName(T("shared"));
    savedObject.identifier = makeUniqueIdentifier(savedObject.object, sharedObjectIdentifiers);
    savedObject.elt->setAttribute(T("identifier"), savedObject.identifier);
    root->insertChildElement(savedObject.elt, 0);
    resolveSharedObjectReferences(savedObject);
    savedObject.elt = NULL;
  }
  
  // resolve all remaining single-references
  for (size_t i = 0; i < savedObjects.size(); ++i)
  {
    SavedObject& savedObject = savedObjects[i];
    if (savedObject.references.size() == 1)
      resolveSingleObjectReference(savedObject);
  }
}

String XmlExporter::makeUniqueIdentifier(ObjectPtr object, std::set<String>& identifiers)
{
  String cn = object->getClassName();
  for (int i = 1; true; ++i)
  {
    String res = cn + String(i);
    if (identifiers.find(res) == identifiers.end())
    {
      identifiers.insert(res);
      return res;
    }
  }
  return String::empty;
}
/*
** XmlImporter
*/
XmlImporter::XmlImporter(const File& file, MessageCallback& callback)
  : callback(callback), root(NULL)
{
  if (file.isDirectory())
  {
    callback.errorMessage(T("Variable::createFromFile"), file.getFullPathName() + T(" is a directory"));
    return;
  }
  
  if (!file.existsAsFile())
  {
    callback.errorMessage(T("Variable::createFromFile"), file.getFullPathName() + T(" does not exists"));
    return;
  }

  juce::XmlDocument document(file);
  
  root = document.getDocumentElement();
  String lastParseError = document.getLastParseError();
  if (!root)
  {
    callback.errorMessage(T("Variable::createFromFile"),
      lastParseError.isEmpty() ? T("Could not parse file ") + file.getFullPathName() : lastParseError);
    return;
  }
  else
    enter(root);

  if (lastParseError.isNotEmpty())
    callback.warningMessage(T("Variable::createFromFile"), lastParseError);
}

Variable XmlImporter::load()
{
  if (root->getTagName() == T("lbcpp"))
    return loadSharedObjects() ? loadVariable(root->getChildByName(T("variable")), TypePtr()) : Variable();
  else
    return loadVariable(root, TypePtr());
}

bool XmlImporter::loadSharedObjects()
{
  forEachXmlChildElementWithTagName(*getCurrentElement(), child, T("shared"))
  {
    Variable variable = loadVariable(child, TypePtr());
    if (!variable.exists())
      return false;
    if (!variable.isObject())
    {
      errorMessage(T("XmlImporter::loadRecursively"), T("Shared variable is not an object"));
      return false;
    }
    String identifier = child->getStringAttribute(T("identifier"));
    if (identifier.isEmpty())
    {
      errorMessage(T("XmlImporter::loadRecursively"), T("Shared object has no identifier"));
      return false;
    }
    sharedObjectsStack.back()[identifier] = variable.getObject();
  }
  //getCurrentElement()->deleteAllChildElementsWithTagName(T("shared"));
  return true;
}

TypePtr XmlImporter::loadType(TypePtr expectedType)
{
  XmlElement* elt = getCurrentElement();
  String typeName = elt->getStringAttribute(T("type"), String::empty).replaceCharacters(T("[]"), T("<>"));
  if (typeName.isNotEmpty())
    return Type::get(typeName, callback);
  else
  {
    XmlElement* child = elt->getChildByName(T("type"));
    if (child)
    {
      TypePtr res;
      enter(child);
      if (hasAttribute(T("reference")))
        res = getReferencedObject().checkCast<Type>(T("XmlImporter::loadType"), callback).get();
      else
        res = Type::loadUnnamedTypeFromXml(*this);
      leave();
      return res;
    }
    else
    {
      if (!expectedType)
        errorMessage(T("XmlImporter::loadType"), T("Could not find type"));
      return expectedType;
    }
  }
}

ObjectPtr XmlImporter::getReferencedObject() const
{
  const SharedObjectMap& sharedObjects = sharedObjectsStack.back();
  String ref = getStringAttribute(T("reference"));
  SharedObjectMap::const_iterator it = sharedObjects.find(ref);
  if (it == sharedObjects.end())
  {
    errorMessage(T("XmlImporter::getReferencedObject"), T("Could not find shared object reference ") + ref.quoted());
    return ObjectPtr();
  }
  jassert(it->second);
  return it->second;
}

Variable XmlImporter::loadVariable(TypePtr expectedType)
{
  TypePtr type = loadType(expectedType);
  if (!type)
    return Variable();
  
  if (getStringAttribute(T("missing")) == T("true"))
    return Variable::missingValue(type);

  if (hasAttribute(T("reference")))
    return getReferencedObject();
  else
    return Variable::createFromXml(type, *this);
}

Variable XmlImporter::loadVariable(XmlElement* elt, TypePtr expectedType)
{
  enter(elt);
  Variable res = loadSharedObjects() ? loadVariable(expectedType) : Variable();
  leave();
  return res;
}

void XmlImporter::enter(XmlElement* child)
{
  jassert(child);
  stack.push_back(child);
  sharedObjectsStack.push_back(sharedObjectsStack.size() ? sharedObjectsStack.back() : SharedObjectMap());
}

bool XmlImporter::enter(const String& childTagName)
{
  XmlElement* child = getCurrentElement()->getChildByName(childTagName);
  if (!child)
  {
    callback.errorMessage(T("XmlImporter::enter"), T("Could not find child ") + childTagName.quoted());
    return false;
  }
  enter(child);
  return true;
}

void XmlImporter::leave()
{
  jassert(stack.size());
  stack.pop_back();
  jassert(sharedObjectsStack.size());
  sharedObjectsStack.pop_back();
}
/*
bool XmlExporter::CompareObjectsDeterministically::operator()(const ObjectPtr& object1, const ObjectPtr& object2) const
{
  if (!object2)
    return false;
  if (!object1)
    return true;
  if (object1->getClass() != object2->getClass())
  {
    jassert(object1->getClass()->getName() != object2->getClass()->getName());
    return object1->getClass()->getName() < object2->getClass()->getName();
  }
  ClassPtr cl = object1->getClass();
  size_t n = cl->getObjectNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable v1 = object1->getVariable(i);
    Variable v2 = object2->getVariable(i);
    if (v1 == v2)
      continue;
    if (!v2.exists())
      return false;
    if (!v1.exists())
      return true;
    if (v1.isObject())
    {
      bool res1 = (*this)(v1.getObject(), v2.getObject());
      bool res2 = (*this)(v2.getObject(), v1.getObject());
      jassert(!res1 || !res2);
      if (res1 != res2)
        return res1;
      else
      {
        // objects are stricly identic
        return v1.getObject() < v2.getObject();
      }
    }
  }
  return false;
}
*/