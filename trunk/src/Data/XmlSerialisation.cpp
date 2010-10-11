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

void XmlExporter::flushSave()
{
  /*std::cout << unresolvedLinks.size() << " unresolved links" << std::endl;
  std::cout << savedObjects.size() << " saved objects:" << std::endl;
  for (SavedObjectMap::const_iterator it = savedObjects.begin(); it != savedObjects.end(); ++it)
  {
    std::cout << "Num references: " <<  it->second.references.size() << std::endl;
    XmlElement* elt = it->second.elt;
    if (elt)
      std::cout << "\t" << elt->createDocument(T("toto"), true, false) << std::endl << std::endl;
  }*/

  if (unresolvedLinks.empty())
    {jassert(savedObjects.size() == 0); return;}

  std::map<ObjectPtr, int> referencedObjects;
  std::set<String> identifiers;
  resolveChildLinks(root, referencedObjects, identifiers);

  jassert(referencedObjects.empty());
  jassert(identifiers.empty());
  savedObjects.clear();
  solvedLinks.clear();
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

void XmlExporter::resolveLink(XmlElement* xml, std::map<ObjectPtr, int>& referencedObjects, std::set<String>& identifiers)
{
  if (solvedLinks.find(xml) != solvedLinks.end())
    return; // already solved
  solvedLinks.insert(xml);

  std::map<XmlElement* , SavedObject* >::iterator it = unresolvedLinks.find(xml);
  if (it != unresolvedLinks.end())
  {
    SavedObject& savedObject = *(it->second);
    unresolvedLinks.erase(it);

    //std::cout << "Solving Link: " << xml->getTagName() << " " << xml->getStringAttribute(T("name")) << " numref = " << savedObject.references.size() << std::endl;
    
    if (savedObject.references.size() == 1)
    {
      // single reference: classical save
      jassert(savedObject.elt);
      xml->moveChildrenFrom(*savedObject.elt); 
      for (int i = 0; i < savedObject.elt->getNumAttributes(); ++i)
        xml->setAttribute(savedObject.elt->getAttributeName(i), savedObject.elt->getAttributeValue(i));
      deleteAndZero(savedObject.elt);
      resolveChildLinks(xml, referencedObjects, identifiers);
    }
    else
    {
      jassert(savedObject.references.size() > 1);

      // shared reference
      if (savedObject.identifier.isEmpty())
        savedObject.identifier = makeUniqueIdentifier(savedObject.object, identifiers);
      
      currentStack.push_back(xml);
      writeType(savedObject.object->getClass());
      setAttribute(T("reference"), savedObject.identifier);
      currentStack.pop_back();

      jassert(!xml->getNumChildElements());
      referencedObjects[savedObject.object]++;
    }
  }
  else
    resolveChildLinks(xml, referencedObjects, identifiers);
}

void XmlExporter::resolveChildLinks(XmlElement* xml, std::map<ObjectPtr, int>& referencedObjects, std::set<String>& identifiers)
{
  //std::cout << "Start: " << xml->getTagName() << " " << xml->getStringAttribute(T("name"), T("unnamed")) << " " << xml->getNumChildElements() << std::endl;

  std::map<ObjectPtr, int> prevReferencedObjects = referencedObjects;

  forEachXmlChildElement(*xml, elt)
    resolveLink(elt, referencedObjects, identifiers);

  int count = referencedObjects.size();
  while (true)
  {
    std::map<ObjectPtr, int>::iterator nxt;
    for (std::map<ObjectPtr, int>::iterator it = referencedObjects.begin(); it != referencedObjects.end(); it = nxt)
    {
      nxt = it; ++nxt;
      SavedObject& savedObject = savedObjects[it->first];
      if (!savedObject.elt)
        continue;
      if ((prevReferencedObjects.find(it->first) == prevReferencedObjects.end() && (size_t)it->second == savedObject.references.size()))
      {
        savedObject.elt->setAttribute(T("type"), savedObject.object->getClassName().replaceCharacters(T("<>"), T("[]")));
        savedObject.elt->setAttribute(T("identifier"), savedObject.identifier);
        identifiers.erase(savedObject.identifier);
        referencedObjects.erase(it);
        xml->insertChildElement(savedObject.elt, 0);
        resolveChildLinks(savedObject.elt, referencedObjects, identifiers);
      }
    }
    if (referencedObjects.size() == (size_t)count)
      break;
    count = referencedObjects.size();
  }
  //std::cout << "End: " << xml->getTagName() << " " << xml->getStringAttribute(T("name"), T("unnamed")) << std::endl;
}

void XmlExporter::writeName(const String& name)
{
  XmlElement* elt = getCurrentElement();
  if (name.isNotEmpty())
    elt->setAttribute(T("name"), name);
}

void XmlExporter::writeType(TypePtr type)
{
  if (type.dynamicCast<DynamicClass>())
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
  else if (variable.isObject() && variable.getType() != typeClass && !variable.getType()->inheritsFrom(enumerationClass))
    writeObject(variable.getObject(), expectedType);
  else
  {
    if (variable.getType() != expectedType)
      writeType(variable.getType());
    variable.saveToXml(*this);
  }
}

void XmlExporter::writeObject(ObjectPtr object, TypePtr expectedType)
{
  XmlElement* currentElement = getCurrentElement();

  SavedObject& savedObject = savedObjects[object];
  if (!savedObject.object)
  {
    savedObject.object = object;
    savedObject.elt = new XmlElement(T("shared"));
    currentStack.push_back(savedObject.elt);
    if (object->getClass() != expectedType)
      writeType(object->getClass());
    object->saveToXml(*this);
    currentStack.pop_back();
  }
  unresolvedLinks[currentElement] = &savedObject;
  savedObject.references.insert(currentElement);
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
    stack.push_back(root);

  if (lastParseError.isNotEmpty())
    callback.warningMessage(T("Variable::createFromFile"), lastParseError);
}

Variable XmlImporter::load()
{
  jassert(sharedObjectsStack.empty());
  if (root->getTagName() == T("lbcpp"))
    return loadVariable(root->getFirstChildElement(), TypePtr());
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
      Variable typeVariable = loadVariable(child, TypePtr());
      return typeVariable.getObjectAndCast<Type>();
    }
    else
    {
      if (!expectedType)
        errorMessage(T("XmlImporter::loadType"), T("Could not find type"));
      return expectedType;
    }
  }
}

Variable XmlImporter::loadVariable(TypePtr expectedType)
{
  const SharedObjectMap& sharedObjects = sharedObjectsStack.back();
  TypePtr type = loadType(expectedType);
  if (!type)
    return Variable();
  
  if (getStringAttribute(T("missing")) == T("true"))
    return Variable::missingValue(type);

  if (getStringAttribute(T("reference")).isNotEmpty())
  {
    String ref = getStringAttribute(T("reference"));
    SharedObjectMap::const_iterator it = sharedObjects.find(ref);
    if (it == sharedObjects.end())
    {
      errorMessage(T("XmlImporter::loadVariable"), T("Could not find shared object reference ") + ref.quoted());
      return Variable();
    }
    jassert(it->second);
    return Variable(it->second);
  }
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

