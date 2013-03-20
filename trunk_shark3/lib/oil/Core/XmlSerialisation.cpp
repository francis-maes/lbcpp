/*-----------------------------------------.---------------------------------.
| Filename: XmlSerialisation.cpp           | Xml Import/Export               |
| Author  : Francis Maes                   |                                 |
| Started : 26/08/2010 17:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <oil/Core/XmlSerialisation.h>
#include <oil/Core/Object.h>
#include <oil/Core/DefaultClass.h>
#include <oil/Core/ClassManager.h>
#include <oil/Execution/ExecutionContext.h>
using namespace lbcpp;

/*
** XmlElement
*/
XmlElementPtr XmlElement::createFromJuceXml(juce::XmlElement* element, bool deleteElementOnceConverted)
{
  XmlElementPtr res(new XmlElement(element->getTagName()));
  res->attributes.resize(element->getNumAttributes());
  for (size_t i = 0; i < res->attributes.size(); ++i)
    res->attributes[i] = std::make_pair(element->getAttributeName(i), element->getAttributeValue(i));
  if (element->isTextElement())
    res->text = element->getText();
  res->childElements.reserve(element->getNumChildElements());
  forEachXmlChildElement(*element, child)
    res->childElements.push_back(XmlElement::createFromJuceXml(child, false));
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

XmlElementPtr XmlElement::getChildByName(const string& name) const
{
  for (size_t i = 0; i < childElements.size(); ++i)
    if (childElements[i]->getTagName() == name)
      return childElements[i];
  return XmlElementPtr();
}

void XmlElement::setAttribute(const string& name, const string& value)
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
    {
      attributes[i].second = value;
      return;
    }
  attributes.push_back(std::make_pair(name, value));
}

string XmlElement::getStringAttribute(const string& name, const string& defaultValue) const
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
      return attributes[i].second;
  return defaultValue;
}

int XmlElement::getIntAttribute(const string& name, int defaultValue) const
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
      return attributes[i].second.getIntValue();
  return defaultValue;
}

bool XmlElement::hasAttribute(const string& name) const
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
      return true;
  return false;
}

void XmlElement::removeAttribute(const string& name)
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
    {
      attributes.erase(attributes.begin() + i);
      return;
    }
}

string XmlElement::getAllSubText() const
{
  string res;
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
    childElements.push_back(XmlElement::createFromJuceXml(child, false));
  return true;
}

bool XmlElement::loadFromXml(XmlImporter& importer)
{
  juce::XmlElement* element = importer.getCurrentElement()->getFirstChildElement();
  return loadFromJuceXmlElement(element);
}

string XmlElement::toString() const
{
  juce::XmlElement* xml = createJuceXmlElement();
  string res = xml->createDocument(string::empty);
  delete xml;
  return res;
}

string XmlElement::toShortString() const
  {return JUCE_T("<") + tagName + JUCE_T(">");}

void XmlElement::saveToXml(XmlExporter& exporter) const
  {exporter.addChildElement(refCountedPointerFromThis(this));}


/*
** XmlExporter
*/
XmlExporter::XmlExporter(ExecutionContext& context, const string& rootTag, int version)
  : context(context)
{
  root = new XmlElement(rootTag);
  if (version)
    root->setAttribute(JUCE_T("version"), version);
  currentStack.push_back(root);
}

XmlExporter::XmlExporter(ExecutionContext& context, XmlElementPtr target)
  : context(context), root(target)
{
  currentStack.push_back(root);
}

bool XmlExporter::saveToFile(const juce::File& file)
{
  if (file.exists())
  {
    if (file.existsAsFile())
    {
      if (!file.deleteFile())
      {
        context.errorCallback(JUCE_T("XmlExporter::saveToFile"), JUCE_T("Could not delete file ") + file.getFullPathName());
        return false;
      }
    }
    else
    {
      context.errorCallback(JUCE_T("XmlExporter::saveToFile"), file.getFullPathName() + JUCE_T(" is a directory"));
      return false;
    }
  }
  
  juce::XmlElement* xml = root->createJuceXmlElement();
  if (!root)
  {
    context.errorCallback(JUCE_T("XmlExporter::saveToFile"), JUCE_T("No root xml element in file ") + file.getFullPathName());
    return false;
  }
  bool ok = xml->writeToFile(file, string::empty);
  if (!ok)
    context.errorCallback(JUCE_T("XmlExporter::saveToFile"), JUCE_T("Could not write file ") + file.getFullPathName());
  delete xml;
  return ok;
}

string XmlExporter::toString()
  {return root ? root->toString() : string::empty;}

XmlElementPtr XmlExporter::getCurrentElement()
  {return currentStack.back();}

void XmlExporter::enter(const string& tagName, const string& name)
{
  XmlElementPtr elt = new XmlElement(tagName);
  currentStack.push_back(elt);
  writeName(name);
}

void XmlExporter::saveObject(const string& name, const ObjectPtr& object, ClassPtr expectedType)
{
  enter(JUCE_T("variable"), name);
  writeObject(object, expectedType);
  leave();
}

void XmlExporter::saveGeneratedObject(const string& name, const ObjectPtr& object, ClassPtr expectedType)
{
  enter(JUCE_T("variable"), name);
  writeType(expectedType);
  linkObjectToCurrentElement(object);
  setAttribute(JUCE_T("generated"), JUCE_T("yes"));
  leave();
}

void XmlExporter::saveElement(size_t index, const ObjectPtr& object, ClassPtr expectedType)
{
  enter(JUCE_T("element"));
  setAttribute(JUCE_T("index"), string((int)index));
  writeObject(object, expectedType);
  leave();
}

void XmlExporter::leave()
{
  jassert(currentStack.size() > 1);
  XmlElementPtr elt = getCurrentElement();
  currentStack.pop_back();
  getCurrentElement()->addChildElement(elt);
}

void XmlExporter::addTextElement(const string& text)
  {getCurrentElement()->addTextElement(text);}

void XmlExporter::writeName(const string& name)
{
  XmlElementPtr elt = getCurrentElement();
  if (name.isNotEmpty())
    elt->setAttribute(JUCE_T("name"), name);
}

void XmlExporter::writeType(ClassPtr type)
{
  jassert(type);
  
  if (type->isNamedType())
  {
    jassert(!getCurrentElement()->hasAttribute(JUCE_T("type")));
    setAttribute(JUCE_T("type"), type->getName().replaceCharacters(JUCE_T("<>"), JUCE_T("[]")));
  }
  else
  {
    enter(JUCE_T("type"));
    writeObject(type, ClassPtr());
    leave();
  }
}

void XmlExporter::writeObject(const ObjectPtr& object, ClassPtr expectedType)
{
  XmlElementPtr elt = getCurrentElement();
  
  if (object)
  {
    ClassPtr typeValue = object.dynamicCast<Class>();
    if (typeValue && typeValue->isNamedType())
      addTextElement(typeValue->getName()); // named type
    else
      writeObjectImpl(object, expectedType); // traditional object
  }
  else
  {
    // warning: we do not write the type anymore since Variable quake
    //writeType(object->getType());
    elt->setAttribute(JUCE_T("missing"), JUCE_T("true"));
  }
}

void XmlExporter::writeObjectImpl(const ObjectPtr& object, ClassPtr expectedType)
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
    string& identifier = it->second.second;
    if (identifier == string::empty)
    {
      // first time this is object is referred to, need to create an identifier
      identifier = makeSharedObjectIdentifier(object);
      const XmlElementPtr& sourceElement = it->second.first;
      sourceElement->setAttribute(JUCE_T("id"), identifier);

      // write "id" and add "type" if it is missing
      // (we do not have access to the "expectedType" information anymore since the object shared)
      currentStack.push_back(sourceElement);
      setAttribute(JUCE_T("id"), identifier);
      if (!getCurrentElement()->hasAttribute(JUCE_T("type")) &&
        !getCurrentElement()->getChildByName(JUCE_T("type")))
        writeType(object->getClass());
      currentStack.pop_back();
    }
    getCurrentElement()->setAttribute(JUCE_T("ref"), identifier);
  }
}

void XmlExporter::linkObjectToCurrentElement(const ObjectPtr& object)
{
  jassert(objectXmlElements.find(object) == objectXmlElements.end());
  objectXmlElements[object] = std::make_pair(getCurrentElement(), string::empty);
}

string XmlExporter::makeSharedObjectIdentifier(ObjectPtr object)
{
  string cn = object->getClassName();
  for (int i = 1; true; ++i)
  {
    string res = cn + string(i);
    if (sharedObjectsIdentifiers.find(res) == sharedObjectsIdentifiers.end())
    {
      sharedObjectsIdentifiers.insert(res);
      return res;
    }
  }
  return string::empty;
}

/*
** XmlImporter
*/
XmlImporter::XmlImporter(ExecutionContext& context, const juce::File& file)
  : context(context), root(NULL)
{
  if (file.isDirectory())
  {
    context.errorCallback(JUCE_T("Object::createFromFile"), file.getFullPathName() + JUCE_T(" is a directory"));
    return;
  }
  
  if (!file.existsAsFile())
  {
    context.errorCallback(JUCE_T("Object::createFromFile"), file.getFullPathName() + JUCE_T(" does not exists"));
    return;
  }

  juce::XmlDocument document(file);
  
  root = document.getDocumentElement();
  string lastParseError = document.getLastParseError();
  if (!root)
  {
    context.errorCallback(JUCE_T("Object::createFromFile"),
      lastParseError.isEmpty() ? JUCE_T("Could not parse file ") + file.getFullPathName() : lastParseError);
    return;
  }
  else
    enter(root);

  if (lastParseError.isNotEmpty())
    context.warningCallback(JUCE_T("Object::createFromFile"), lastParseError);
}

XmlImporter::XmlImporter(ExecutionContext& context, juce::XmlDocument& document)
  : context(context), root(document.getDocumentElement())
{
  string lastParseError = document.getLastParseError();
  if (!root)
  {
    context.errorCallback(JUCE_T("NetworkClient::messageReceived"), 
                          lastParseError.isEmpty() ? JUCE_T("Could not parse message") : lastParseError);
    return;
  }
  else
    enter(root);
  
  if (lastParseError.isNotEmpty())
    context.warningCallback(JUCE_T("NetworkClient::messageReceived"), lastParseError);
}

XmlImporter::XmlImporter(ExecutionContext& context, juce::XmlElement* newRoot)
  : context(context), root(newRoot)
{
  enter(root);
}

void XmlImporter::errorMessage(const string& where, const string& what) const
  {context.errorCallback(where, what);}

void XmlImporter::warningMessage(const string& where, const string& what) const
  {context.warningCallback(where, what);}

void XmlImporter::unknownVariableWarning(ClassPtr type, const string& variableName) const
{
  std::pair<ClassPtr, string> key = std::make_pair(type, variableName);
  if (unknownVariables.find(key) == unknownVariables.end())
  {
    warningMessage(JUCE_T("Load from xml"), JUCE_T("Unknown variable ") + type->getName() + JUCE_T("::") + variableName);
    const_cast<XmlImporter* >(this)->unknownVariables.insert(key);
  }
}

ObjectPtr XmlImporter::load()
{
  if (root->getTagName() == JUCE_T("lbcpp"))
    return loadObject(root->getChildByName(JUCE_T("variable")), ClassPtr());
  else
    return loadObject(root, ClassPtr());
}

ClassPtr XmlImporter::loadType(ClassPtr expectedType)
{
  juce::XmlElement* elt = getCurrentElement();
  string typeName = elt->getStringAttribute(JUCE_T("type"), string::empty).replaceCharacters(JUCE_T("[]"), JUCE_T("<>"));
  if (typeName.isNotEmpty())
    return typeManager().getType(context, typeName);
  else
  {
    juce::XmlElement* child = elt->getChildByName(JUCE_T("type"));
    if (child)
    {
      ClassPtr res;
      enter(child);
      if (hasAttribute(JUCE_T("ref")))
        res = getSharedObject(getStringAttribute(JUCE_T("ref"))).staticCast<Class>().get();
      else
        res = loadUnnamedType();
      linkCurrentElementToObject(res);
      leave();
      return res;
    }
    else
    {
      if (!expectedType)
        context.errorCallback(JUCE_T("XmlImporter::loadType"), JUCE_T("Could not find type"));
      return expectedType;
    }
  }
}

ClassPtr XmlImporter::loadUnnamedType()
{
  // load unnamed type from xml
  if (hasAttribute(JUCE_T("templateType")))
  {
    string templateType = getStringAttribute(JUCE_T("templateType"));
    std::vector<ClassPtr> templateArguments;
    forEachXmlChildElementWithTagName(*getCurrentElement(), elt, JUCE_T("templateArgument"))
    {
      enter(elt);
      int index = getIntAttribute(JUCE_T("index"));
      if (index < 0)
      {
        errorMessage(JUCE_T("Type::loadTypeFromXml"), JUCE_T("Invalid template argument index"));
        return ClassPtr();
      }
      templateArguments.resize(index + 1);
      templateArguments[index] = loadType(ClassPtr());
      if (!templateArguments[index])
        return ClassPtr();
      leave();
    }
    return typeManager().getType(getContext(), templateType, templateArguments);
  }
  else
    return typeManager().getType(getContext(), getStringAttribute(JUCE_T("typeName")));
}

ObjectPtr XmlImporter::loadObject(ClassPtr expectedType)
{
  if (hasAttribute(JUCE_T("ref")))
    return getSharedObject(getStringAttribute(JUCE_T("ref")));
  else
  {
    ClassPtr type = loadType(expectedType);
    if (!type || !type->getBaseType())
      return ObjectPtr();
    
    if (getStringAttribute(JUCE_T("missing")) == JUCE_T("true"))
      return ObjectPtr();

    ObjectPtr res = Object::createFromXml(*this, type);
    if (res)
      linkCurrentElementToObject(res);
    return res;
  }
}

void XmlImporter::linkCurrentElementToObject(ObjectPtr object)
{
  if (hasAttribute(JUCE_T("id")))
    addSharedObject(getStringAttribute(JUCE_T("id")), object);
}

ObjectPtr XmlImporter::loadObject(juce::XmlElement* elt, ClassPtr expectedType)
{
  enter(elt);
  ObjectPtr res = loadObject(expectedType);
  leave();
  return res;
}

void XmlImporter::enter(juce::XmlElement* child)
{
  jassert(child);
  stack.push_back(child);
}

bool XmlImporter::enter(const string& childTagName)
{
  juce::XmlElement* child = getCurrentElement()->getChildByName(childTagName);
  if (!child)
  {
    context.errorCallback(JUCE_T("XmlImporter::enter"), JUCE_T("Could not find child ") + childTagName.quoted());
    return false;
  }
  enter(child);
  return true;
}

void XmlImporter::leave()
{
  jassert(stack.size());
  stack.pop_back();
}

bool XmlImporter::addSharedObject(const string& name, ObjectPtr object)
{
  if (sharedObjects.find(name) != sharedObjects.end())
  {
    context.errorCallback(JUCE_T("Could not add shared object ") + name.quoted() + JUCE_T(": this identifier is already used by another object"));
    return false;
  }
  if (!object->getClass() || !object->getClass()->getBaseType())
  {
    context.errorCallback(JUCE_T("Could not add shared object ") + name.quoted() + JUCE_T(": invalid class"));
    return false;
  }
  sharedObjects[name] = object;
  return true;
}

ObjectPtr XmlImporter::getSharedObject(const string& name) const
{
  SharedObjectsMap::const_iterator it = sharedObjects.find(name);
  if (it == sharedObjects.end())
  {
    errorMessage(JUCE_T("XmlImporter::getSharedObject"), JUCE_T("Could not find shared object reference ") + name.quoted());
    return ObjectPtr();
  }
  jassert(it->second);
  return it->second;
}
