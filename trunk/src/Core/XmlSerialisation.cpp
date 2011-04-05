/*-----------------------------------------.---------------------------------.
| Filename: XmlSerialisation.cpp           | Xml Import/Export               |
| Author  : Francis Maes                   |                                 |
| Started : 26/08/2010 17:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Core/DynamicObject.h>
#include <lbcpp/Execution/ExecutionContext.h>
using namespace lbcpp;

/*
** XmlElement
*/
XmlElementPtr XmlElement::createFromXml(juce::XmlElement* element, bool deleteElementOnceConverted)
{
  XmlElementPtr res(new XmlElement(element->getTagName()));
  res->attributes.resize(element->getNumAttributes());
  for (size_t i = 0; i < res->attributes.size(); ++i)
    res->attributes[i] = std::make_pair(element->getAttributeName(i), element->getAttributeValue(i));
  if (element->isTextElement())
    res->text = element->getText();
  res->childElements.reserve(element->getNumChildElements());
  forEachXmlChildElement(*element, child)
    res->childElements.push_back(XmlElement::createFromXml(child, false));
  if (deleteElementOnceConverted)
    delete element;
  return res;
}

juce::XmlElement* XmlElement::createJuceXmlElement() const
{
  if (tagName.isEmpty())
    return juce::XmlElement::createTextElement(text);
  else
  {
    juce::XmlElement* res = new juce::XmlElement(tagName);
    for (size_t i = 0; i < attributes.size(); ++i)
      res->setAttribute(attributes[i].first, attributes[i].second);
    for (size_t i = 0; i < childElements.size(); ++i)
      res->addChildElement(childElements[i]->createJuceXmlElement());
    return res;
  }
}

void XmlElement::saveObject(ExecutionContext& context, ObjectPtr value)
{
  XmlExporter exporter(context, refCountedPointerFromThis(this));
  exporter.writeVariable(value, objectClass);
}

ObjectPtr XmlElement::createObject(ExecutionContext& context) const
{
  juce::XmlDocument newDocument(createJuceXmlElement()->createDocument(String::empty));
  XmlImporter importer(context, newDocument);
  Variable v = importer.isOpened() ? importer.load() : Variable();
  jassert(v.isObject());
  return v.getObject();
}

XmlElementPtr XmlElement::getChildByName(const String& name) const
{
  for (size_t i = 0; i < childElements.size(); ++i)
    if (childElements[i]->getTagName() == name)
      return childElements[i];
  return XmlElementPtr();
}

void XmlElement::setAttribute(const String& name, const String& value)
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
    {
      attributes[i].second = value;
      return;
    }
  attributes.push_back(std::make_pair(name, value));
}

String XmlElement::getStringAttribute(const String& name, const String& defaultValue) const
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
      return attributes[i].second;
  return defaultValue;
}

int XmlElement::getIntAttribute(const String& name, int defaultValue) const
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
      return attributes[i].second.getIntValue();
  return defaultValue;
}

bool XmlElement::hasAttribute(const String& name) const
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
      return true;
  return false;
}

void XmlElement::removeAttribute(const String& name)
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
    {
      attributes.erase(attributes.begin() + i);
      return;
    }
}

String XmlElement::getAllSubText() const
{
  String res;
  for (size_t i = 0; i < childElements.size(); ++i)
    if (childElements[i]->isTextElement())
      res += childElements[i]->getText();
  return res;
}

bool XmlElement::loadFromJuceXmlElement(juce::XmlElement* element)
{
  tagName = element->getTagName();
  
  attributes.clear();
  attributes.resize(element->getNumAttributes());
  for (size_t i = 0; i < attributes.size(); ++i)
    attributes[i] = std::make_pair(element->getAttributeName(i), element->getAttributeValue(i));
  if (element->isTextElement())
    text = element->getText();
  
  childElements.clear();
  childElements.reserve(element->getNumChildElements());
  forEachXmlChildElement(*element, child)
    childElements.push_back(XmlElement::createFromXml(child, false));
  return true;
}

bool XmlElement::loadFromXml(XmlImporter& importer)
{
  juce::XmlElement* element = importer.getCurrentElement()->getFirstChildElement();
  return loadFromJuceXmlElement(element);
}

String XmlElement::toString() const
{
  juce::XmlElement* xml = createJuceXmlElement();
  String res = xml->createDocument(String::empty);
  delete xml;
  return res;
}

String XmlElement::toShortString() const
  {return T("<") + tagName + T(">");}

void XmlElement::saveToXml(XmlExporter& exporter) const
  {exporter.addChildElement(refCountedPointerFromThis(this));}


/*
** XmlExporter
*/
XmlExporter::XmlExporter(ExecutionContext& context, const String& rootTag, int version)
  : context(context)
{
  root = new XmlElement(rootTag);
  if (version)
    root->setAttribute(T("version"), version);
  currentStack.push_back(root);
}

XmlExporter::XmlExporter(ExecutionContext& context, XmlElementPtr target)
  : context(context), root(target)
{
  currentStack.push_back(root);
}

bool XmlExporter::saveToFile(const File& file)
{
  if (file.exists())
  {
    if (file.existsAsFile())
    {
      if (!file.deleteFile())
      {
        context.errorCallback(T("XmlExporter::saveToFile"), T("Could not delete file ") + file.getFullPathName());
        return false;
      }
    }
    else
    {
      context.errorCallback(T("XmlExporter::saveToFile"), file.getFullPathName() + T(" is a directory"));
      return false;
    }
  }
  
  juce::XmlElement* xml = root->createJuceXmlElement();
  if (!root)
  {
    context.errorCallback(T("XmlExporter::saveToFile"), T("No root xml element in file ") + file.getFullPathName());
    return false;
  }
  bool ok = xml->writeToFile(file, String::empty);
  if (!ok)
    context.errorCallback(T("XmlExporter::saveToFile"), T("Could not write file ") + file.getFullPathName());
  delete xml;
  return ok;
}

String XmlExporter::toString()
  {return root ? root->toString() : String::empty;}

XmlElementPtr XmlExporter::getCurrentElement()
  {return currentStack.back();}

void XmlExporter::enter(const String& tagName, const String& name)
{
  XmlElementPtr elt = new XmlElement(tagName);
  currentStack.push_back(elt);
  writeName(name);
}

void XmlExporter::saveVariable(const String& name, const Variable& variable, TypePtr expectedType)
{
  enter(T("variable"), name);
  writeVariable(variable, expectedType);
  leave();
}

void XmlExporter::saveGeneratedVariable(const String& name, const ObjectPtr& object, TypePtr expectedType)
{
  enter(T("variable"), name);
  writeType(expectedType);
  linkObjectToCurrentElement(object);
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
  XmlElementPtr elt = getCurrentElement();
  currentStack.pop_back();
  getCurrentElement()->addChildElement(elt);
}

void XmlExporter::addTextElement(const String& text)
{
  enum {lineLength = 60};
  int n = text.length();
  int b = 0;
  while (b < n)
  {
    int e = -1;
    if (b + lineLength < n)
      e = text.indexOfAnyOf(T(" \t\n\r"), b + lineLength);
    if (e < 0)
      e = n;
    getCurrentElement()->addTextElement(text.substring(b, e));
    b = e;
    if (b < n)
      getCurrentElement()->addTextElement(T("\n"));
  }
}

void XmlExporter::writeName(const String& name)
{
  XmlElementPtr elt = getCurrentElement();
  if (name.isNotEmpty())
    elt->setAttribute(T("name"), name);
}

void XmlExporter::writeType(TypePtr type)
{
  jassert(type);
  
  if (type->isNamedType())
  {
    jassert(!getCurrentElement()->hasAttribute(T("type")));
    setAttribute(T("type"), type->getName().replaceCharacters(T("<>"), T("[]")));
  }
  else
  {
    enter(T("type"));
    writeVariable(type, TypePtr());
    leave();
  }
}

void XmlExporter::writeVariable(const Variable& variable, TypePtr expectedType)
{
  XmlElementPtr elt = getCurrentElement();
  
  if (variable.isMissingValue())
  {
    writeType(variable.getType());
    elt->setAttribute(T("missing"), T("true"));
  }
  else if (variable.isObject())
  {
    const ObjectPtr& object = variable.getObject();
    TypePtr typeValue = object.dynamicCast<Type>();
    if (typeValue && typeValue->isNamedType())
      addTextElement(typeValue->getName()); // named type
    else
      writeObject(object, expectedType); // traditional object
  }
  else
  {
    TypePtr type = variable.getType();
    if (type != expectedType)
      writeType(type);
    if (!type->isMissingValue(variable.value))
      type->saveToXml(*this, variable.value);
  }
}

void XmlExporter::writeObject(const ObjectPtr& object, TypePtr expectedType)
{
  jassert(object->getReferenceCount());
  ObjectXmlElementsMap::iterator it = objectXmlElements.find(object);
  if (it == objectXmlElements.end())
  {
    // new object
    object->saveToXml(*this);
    if (expectedType != object->getClass())
      writeType(object->getClass());

    linkObjectToCurrentElement(object);
  }
  else
  {
    // shared object
    String& identifier = it->second.second;
    if (identifier == String::empty)
    {
      // first time this is object is referred to, need to create an identifier
      identifier = makeSharedObjectIdentifier(object);
      const XmlElementPtr& sourceElement = it->second.first;
      sourceElement->setAttribute(T("sharedObjectId"), identifier);
    }
    getCurrentElement()->setAttribute(T("shared"), identifier);
  }
}

void XmlExporter::linkObjectToCurrentElement(const ObjectPtr& object)
{
  jassert(objectXmlElements.find(object) == objectXmlElements.end());
  objectXmlElements[object] = std::make_pair(getCurrentElement(), String::empty);
}

String XmlExporter::makeSharedObjectIdentifier(ObjectPtr object)
{
  String cn = object->getClassName();
  for (int i = 1; true; ++i)
  {
    String res = cn + String(i);
    if (sharedObjectsIdentifiers.find(res) == sharedObjectsIdentifiers.end())
    {
      sharedObjectsIdentifiers.insert(res);
      return res;
    }
  }
  return String::empty;
}

/*
** XmlImporter
*/
XmlImporter::XmlImporter(ExecutionContext& context, const File& file)
  : context(context), root(NULL)
{
  if (file.isDirectory())
  {
    context.errorCallback(T("Variable::createFromFile"), file.getFullPathName() + T(" is a directory"));
    return;
  }
  
  if (!file.existsAsFile())
  {
    context.errorCallback(T("Variable::createFromFile"), file.getFullPathName() + T(" does not exists"));
    return;
  }

  juce::XmlDocument document(file);
  
  root = document.getDocumentElement();
  String lastParseError = document.getLastParseError();
  if (!root)
  {
    context.errorCallback(T("Variable::createFromFile"),
      lastParseError.isEmpty() ? T("Could not parse file ") + file.getFullPathName() : lastParseError);
    return;
  }
  else
    enter(root);

  if (lastParseError.isNotEmpty())
    context.warningCallback(T("Variable::createFromFile"), lastParseError);
}

XmlImporter::XmlImporter(ExecutionContext& context, juce::XmlDocument& document)
  : context(context), root(document.getDocumentElement())
{
  String lastParseError = document.getLastParseError();
  if (!root)
  {
    context.errorCallback(T("NetworkClient::messageReceived"), 
                          lastParseError.isEmpty() ? T("Could not parse message") : lastParseError);
    return;
  }
  else
    enter(root);
  
  if (lastParseError.isNotEmpty())
    context.warningCallback(T("NetworkClient::messageReceived"), lastParseError);
}

void XmlImporter::errorMessage(const String& where, const String& what) const
  {context.errorCallback(where, what);}

void XmlImporter::warningMessage(const String& where, const String& what) const
  {context.warningCallback(where, what);}

Variable XmlImporter::load()
{
  if (root->getTagName() == T("lbcpp"))
  {
    loadSharedObjects();
    return loadVariable(root->getChildByName(T("variable")), TypePtr());
  }
  else
    return loadVariable(root, TypePtr());
}

bool XmlImporter::loadSharedObjects()
{
  bool res = true;
  forEachXmlChildElementWithTagName(*getCurrentElement(), child, T("shared"))
  {
    Variable variable = loadVariable(child, TypePtr());
    if (!variable.exists())
    {
      res = false;
      continue;
    }

    if (!variable.isObject())
    {
      errorMessage(T("XmlImporter::loadRecursively"), T("Shared variable is not an object"));
      res = false;
      continue;
    }
    String identifier = child->getStringAttribute(T("identifier"));
    if (identifier.isEmpty())
    {
      errorMessage(T("XmlImporter::loadRecursively"), T("Shared object has no identifier"));
      res = false;
      continue;
    }
    sharedObjectsStack.back()[identifier] = variable.getObject();
  }
  return res;
}

TypePtr XmlImporter::loadType(TypePtr expectedType)
{
  juce::XmlElement* elt = getCurrentElement();
  String typeName = elt->getStringAttribute(T("type"), String::empty).replaceCharacters(T("[]"), T("<>"));
  if (typeName.isNotEmpty())
    return typeManager().getType(context, typeName);
  else
  {
    juce::XmlElement* child = elt->getChildByName(T("type"));
    if (child)
    {
      TypePtr res;
      enter(child);
      if (hasAttribute(T("reference")))
        res = getSharedObject(getStringAttribute(T("reference"))).staticCast<Type>().get();
      else
        res = Type::loadUnnamedTypeFromXml(*this);
      leave();
      return res;
    }
    else
    {
      if (!expectedType)
        context.errorCallback(T("XmlImporter::loadType"), T("Could not find type"));
      return expectedType;
    }
  }
}

Variable XmlImporter::loadVariable(TypePtr expectedType)
{
  TypePtr type = loadType(expectedType);
  if (!type)
    return Variable();
  
  if (getStringAttribute(T("missing")) == T("true"))
    return Variable::missingValue(type);

  jassert(!hasAttribute(T("generatedId")));

  if (hasAttribute(T("reference")))
    return getSharedObject(getStringAttribute(T("reference")));
  else
    return Variable::createFromXml(type, *this);
}

Variable XmlImporter::loadVariable(juce::XmlElement* elt, TypePtr expectedType)
{
  enter(elt);
  Variable res = loadSharedObjects() ? loadVariable(expectedType) : Variable();
  leave();
  return res;
}

void XmlImporter::enter(juce::XmlElement* child)
{
  jassert(child);
  stack.push_back(child);
  sharedObjectsStack.push_back(sharedObjectsStack.size() ? sharedObjectsStack.back() : SharedObjectMap());
}

bool XmlImporter::enter(const String& childTagName)
{
  juce::XmlElement* child = getCurrentElement()->getChildByName(childTagName);
  if (!child)
  {
    context.errorCallback(T("XmlImporter::enter"), T("Could not find child ") + childTagName.quoted());
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

bool XmlImporter::addSharedObject(const String& name, ObjectPtr object)
{
  jassert(sharedObjectsStack.size());
  SharedObjectMap& m = sharedObjectsStack.back();
  if (m.find(name) != m.end())
  {
    context.errorCallback(T("Could not add shared object ") + name.quoted() + T(": this identifier is already used by another object"));
    return false;
  }
  std::cerr << "addSharedObject: " << name << " " << object->toShortString() << std::endl;
  m[name] = object;
  return true;
}

bool XmlImporter::removeSharedObject(const String& name, ObjectPtr object)
{
  jassert(sharedObjectsStack.size());
  SharedObjectMap& m = sharedObjectsStack.back();
  if (m.find(name) == m.end())
  {
    context.errorCallback(T("Could not remove shared object ") + name.quoted() + T(": this object does not exists"));
    return false;
  }
  std::cerr << "removeSharedObject: " << name << " " << object->toShortString() << std::endl;
  m.erase(name);
  return true;
}

ObjectPtr XmlImporter::getSharedObject(const String& name) const
{
  const SharedObjectMap& sharedObjects = sharedObjectsStack.back();
  SharedObjectMap::const_iterator it = sharedObjects.find(name);
  if (it == sharedObjects.end())
  {
    errorMessage(T("XmlImporter::getSharedObject"), T("Could not find shared object reference ") + name.quoted());
    return ObjectPtr();
  }
  std::cerr << "getSharedObject: " << name << " " << it->second->toShortString() << std::endl;
  jassert(it->second);
  return it->second;
}
