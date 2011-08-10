/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
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
| Filename: XmlSerialisation.h             | Xml Importer/Exporter           |
| Author  : Francis Maes                   |                                 |
| Started : 26/08/2010 17:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_XML_SERIALISATION_H_
# define LBCPP_CORE_XML_SERIALISATION_H_

# include "Variable.h"

namespace lbcpp
{

/*
** XmlElement
*/
class XmlElement;
typedef ReferenceCountedObjectPtr<XmlElement> XmlElementPtr;

class XmlElement : public Object
{
public:
  XmlElement(const String& tagName)
    : tagName(tagName) {}
  XmlElement() {}
  virtual ~XmlElement() {}

  static XmlElementPtr createFromXml(juce::XmlElement* element, bool deleteElementOnceConverted = false);
  bool loadFromJuceXmlElement(juce::XmlElement* element);
  juce::XmlElement* createJuceXmlElement() const;
  
  void saveObject(ExecutionContext& context, const ObjectPtr& object, const String& tagName = T("object"));
  void saveVariable(ExecutionContext& context, const Variable& variable, const String& tagName = T("variable"));

  ObjectPtr createObject(ExecutionContext& context) const;
  Variable createVariable(ExecutionContext& context) const;

  template<class O>
  ReferenceCountedObjectPtr<O> createObjectAndCast(ExecutionContext& context) const
  {
    ObjectPtr res = createObject(context);
    return res.staticCast<O>();
  }

  /*
  ** Tag
  */
  String getTagName() const
    {return tagName;}

  void setTagName(const String& tagName)
    {this->tagName = tagName;}

  /*
  ** Attributes
  */
  size_t getNumAttributes() const
    {return attributes.size();}
  
  const String& getAttributeName(size_t index) const
    {return attributes[index].first;}

  void setAttributeName(size_t index, const String& name)
    {attributes[index].first = name;}

  const String& getAttributeValue(size_t index) const
    {return attributes[index].second;}

  void setAttribute(const String& name, const String& value);
  void setAttribute(const String& name, int value)
    {setAttribute(name, String(value));}
  void setAttribute(const String& name, double value)
    {setAttribute(name, String(value));}

  bool hasAttribute(const String& name) const;
  void removeAttribute(const String& name);

  String getStringAttribute(const String& name, const String& defaultValue = String::empty) const;
  int getIntAttribute(const String& name, int defaultValue = 0) const;

  /*
  ** Text
  */
  bool isTextElement() const
    {return tagName.isEmpty();}

  String getText() const
    {return text;}
  String getAllSubText() const;

  /*
  ** Child Elements
  */
  size_t getNumChildElements() const
    {return childElements.size();}

  XmlElementPtr getChildElement(size_t index) const
    {return childElements[index];}

  XmlElementPtr getChildByName(const String& name) const;

  void addChildElement(XmlElementPtr element)
    {childElements.push_back(element);}

  void addTextElement(const String& text)
  {
    XmlElementPtr element(new XmlElement(String::empty));
    element->text = text;
    addChildElement(element);
  }

  void removeChildElements()
    {childElements.clear();}

  void setChildElements(const std::vector<XmlElementPtr>& childs)
    {childElements = childs;}

  /* Object */
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

  virtual String toString() const;
  virtual String toShortString() const;

  lbcpp_UseDebuggingNewOperator

private:
  friend class XmlElementClass;
  
  String tagName;
  std::vector<XmlElementPtr> childElements;
  std::vector< std::pair<String, String> > attributes;
  String text;
};

extern ClassPtr xmlElementClass;

/*
** XmlExporter
*/
class XmlExporter
{
public:
  XmlExporter(ExecutionContext& context, const String& rootTag = T("lbcpp"), int version = 200);
  XmlExporter(ExecutionContext& context, XmlElementPtr target);

  bool saveToFile(const File& file);
  String toString();

  XmlElementPtr getCurrentElement();

  void saveVariable(const String& name, const Variable& variable, TypePtr expectedType);
  void saveGeneratedVariable(const String& name, const ObjectPtr& object, TypePtr expectedType);

  void saveElement(size_t index, const Variable& variable, TypePtr expectedType);
  
  void enter(const String& tagName, const String& name = String::empty);
  void writeType(TypePtr type);
  void writeName(const String& name);
  void writeVariable(const Variable& variable, TypePtr expectedType);
  void writeObject(const ObjectPtr& object, TypePtr expectedType);
  void leave();

  void addTextElement(const String& text);

  template<class TT>
  void setAttribute(const String& name, const TT& value)
    {getCurrentElement()->setAttribute(name, value);}

  void setAttribute(const String& name, size_t value)
    {getCurrentElement()->setAttribute(name, (int)value);}

  void addChildElement(XmlElementPtr elt)
    {getCurrentElement()->addChildElement(elt);}

  ExecutionContext& getContext()
    {return context;}

private:
  ExecutionContext& context;
  XmlElementPtr root;
  std::vector<XmlElementPtr> currentStack;

  std::set<String> sharedObjectsIdentifiers;

  typedef std::map<ObjectPtr, std::pair<XmlElementPtr, String> > ObjectXmlElementsMap; // Object -> (XmlElement, Identifier)
  ObjectXmlElementsMap objectXmlElements;

  void linkObjectToCurrentElement(const ObjectPtr& object);
  String makeSharedObjectIdentifier(ObjectPtr object);
};

/*
** XmlImporter
*/
class XmlImporter
{
public:
  XmlImporter(ExecutionContext& context, const File& file);
  XmlImporter(ExecutionContext& context, juce::XmlDocument& document);
  XmlImporter(ExecutionContext& context, juce::XmlElement* newRoot);
  ~XmlImporter()
    {if (root) delete root;}

  bool isOpened() const
    {return root != NULL;}

  void errorMessage(const String& where, const String& what) const;
  void warningMessage(const String& where, const String& what) const;

  typedef std::map<String, ObjectPtr> SharedObjectMap;

  Variable load();

  juce::XmlElement* getCurrentElement() const
    {return stack.back();}

  String getTagName() const
    {return getCurrentElement()->getTagName();}

  String getAllSubText() const
    {return getCurrentElement()->getAllSubText();}

  bool hasAttribute(const String& attributeName) const
    {return getCurrentElement()->hasAttribute(attributeName);}

  bool getBoolAttribute(const String& attributeName, bool defaultResult = 0) const
    {return getCurrentElement()->getBoolAttribute(attributeName, defaultResult);}

  int getIntAttribute(const String& attributeName, int defaultResult = 0) const
    {return getCurrentElement()->getIntAttribute(attributeName, defaultResult);}

  double getDoubleAttribute(const String& attributeName, double defaultResult = 0.0) const
    {return getCurrentElement()->getDoubleAttribute(attributeName, defaultResult);}

  String getStringAttribute(const String& attributeName, const String& defaultResult = String::empty) const
    {return getCurrentElement()->getStringAttribute(attributeName, defaultResult);}

  Variable loadVariable(juce::XmlElement* child, TypePtr expectedType);

  void enter(juce::XmlElement* child);
  bool enter(const String& childTagName);
  TypePtr loadType(TypePtr expectedType);
  Variable loadVariable(TypePtr expectedType);
  void leave();

  void linkCurrentElementToObject(ObjectPtr object);

  ExecutionContext& getContext()
    {return context;}

private:
  ExecutionContext& context;
  juce::XmlElement* root;

  std::vector<juce::XmlElement* > stack;

  typedef std::map<String, ObjectPtr> SharedObjectsMap;
  SharedObjectsMap sharedObjects;

  bool addSharedObject(const String& name, ObjectPtr object);
  ObjectPtr getSharedObject(const String& name) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_XML_SERIALISATION_H_
