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

#ifndef OIL_CORE_XML_SERIALISATION_H_
# define OIL_CORE_XML_SERIALISATION_H_

# include "Object.h"

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
  XmlElement(const string& tagName)
    : tagName(tagName) {}
  XmlElement() {}
  virtual ~XmlElement() {}

  static XmlElementPtr createFromJuceXml(juce::XmlElement* element, bool deleteElementOnceConverted = false);
  bool loadFromJuceXmlElement(juce::XmlElement* element);
  juce::XmlElement* createJuceXmlElement() const;

  /*
  ** Tag
  */
  string getTagName() const
    {return tagName;}

  void setTagName(const string& tagName)
    {this->tagName = tagName;}

  /*
  ** Attributes
  */
  size_t getNumAttributes() const
    {return attributes.size();}
  
  const string& getAttributeName(size_t index) const
    {return attributes[index].first;}

  void setAttributeName(size_t index, const string& name)
    {attributes[index].first = name;}

  const string& getAttributeValue(size_t index) const
    {return attributes[index].second;}

  void setAttribute(const string& name, const string& value);
  void setAttribute(const string& name, int value)
    {setAttribute(name, string(value));}
  void setAttribute(const string& name, double value)
    {setAttribute(name, string(value));}

  bool hasAttribute(const string& name) const;
  void removeAttribute(const string& name);

  string getStringAttribute(const string& name, const string& defaultValue = string::empty) const;
  int getIntAttribute(const string& name, int defaultValue = 0) const;

  /*
  ** Text
  */
  bool isTextElement() const
    {return tagName.isEmpty();}

  string getText() const
    {return text;}
  string getAllSubText() const;

  /*
  ** Child Elements
  */
  size_t getNumChildElements() const
    {return childElements.size();}

  XmlElementPtr getChildElement(size_t index) const
    {return childElements[index];}

  XmlElementPtr getChildByName(const string& name) const;

  void addChildElement(XmlElementPtr element)
    {childElements.push_back(element);}

  void addTextElement(const string& text)
  {
    XmlElementPtr element(new XmlElement(string::empty));
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

  virtual string toString() const;
  virtual string toShortString() const;

  lbcpp_UseDebuggingNewOperator

private:
  friend class XmlElementClass;
  
  string tagName;
  std::vector<XmlElementPtr> childElements;
  std::vector< std::pair<string, string> > attributes;
  string text;
};

extern ClassPtr xmlElementClass;

/*
** XmlExporter
*/
class XmlExporter
{
public:
  XmlExporter(ExecutionContext& context, const string& rootTag = T("lbcpp"), int version = 200);
  XmlExporter(ExecutionContext& context, XmlElementPtr target);

  bool saveToFile(const juce::File& file);
  string toString();

  XmlElementPtr getCurrentElement();

  void saveObject(const string& name, const ObjectPtr& object, ClassPtr expectedType);
  void saveGeneratedObject(const string& name, const ObjectPtr& object, ClassPtr expectedType);

  void saveElement(size_t index, const ObjectPtr& object, ClassPtr expectedType);
  
  void enter(const string& tagName, const string& name = string::empty);
  void writeType(ClassPtr type);
  void writeName(const string& name);
  void writeObject(const ObjectPtr& object, ClassPtr expectedType);
  void leave();

  void addTextElement(const string& text);

  template<class TT>
  void setAttribute(const string& name, const TT& value)
    {getCurrentElement()->setAttribute(name, value);}

  void setAttribute(const string& name, size_t value)
    {getCurrentElement()->setAttribute(name, (int)value);}

  void addChildElement(XmlElementPtr elt)
    {getCurrentElement()->addChildElement(elt);}

  ExecutionContext& getContext()
    {return context;}

private:
  ExecutionContext& context;
  XmlElementPtr root;
  std::vector<XmlElementPtr> currentStack;

  std::set<string> sharedObjectsIdentifiers;

  typedef std::map<ObjectPtr, std::pair<XmlElementPtr, string> > ObjectXmlElementsMap; // Object -> (XmlElement, Identifier)
  ObjectXmlElementsMap objectXmlElements;

  void linkObjectToCurrentElement(const ObjectPtr& object);
  string makeSharedObjectIdentifier(ObjectPtr object);

  void writeObjectImpl(const ObjectPtr& object, ClassPtr expectedType);
};

/*
** XmlImporter
*/
class XmlImporter
{
public:
  XmlImporter(ExecutionContext& context, const juce::File& file);
  XmlImporter(ExecutionContext& context, juce::XmlDocument& document);
  XmlImporter(ExecutionContext& context, juce::XmlElement* newRoot);
  ~XmlImporter()
    {if (root) delete root;}

  bool isOpened() const
    {return root != NULL;}

  void errorMessage(const string& where, const string& what) const;
  void warningMessage(const string& where, const string& what) const;
  void unknownVariableWarning(ClassPtr type, const string& variableName) const;

  typedef std::map<string, ObjectPtr> SharedObjectMap;

  ObjectPtr load();

  juce::XmlElement* getCurrentElement() const
    {return stack.back();}

  string getTagName() const
    {return getCurrentElement()->getTagName();}

  string getAllSubText() const
    {return getCurrentElement()->getAllSubText();}

  bool hasAttribute(const string& attributeName) const
    {return getCurrentElement()->hasAttribute(attributeName);}

  bool getBoolAttribute(const string& attributeName, bool defaultResult = 0) const
    {return getCurrentElement()->getBoolAttribute(attributeName, defaultResult);}

  int getIntAttribute(const string& attributeName, int defaultResult = 0) const
    {return getCurrentElement()->getIntAttribute(attributeName, defaultResult);}

  double getDoubleAttribute(const string& attributeName, double defaultResult = 0.0) const
    {return getCurrentElement()->getDoubleAttribute(attributeName, defaultResult);}

  string getStringAttribute(const string& attributeName, const string& defaultResult = string::empty) const
    {return getCurrentElement()->getStringAttribute(attributeName, defaultResult);}

  ObjectPtr loadObject(juce::XmlElement* child, ClassPtr expectedType);

  void enter(juce::XmlElement* child);
  bool enter(const string& childTagName);
  ClassPtr loadType(ClassPtr expectedType);
  ClassPtr loadUnnamedType();
  ObjectPtr loadObject(ClassPtr expectedType);
  void leave();

  void linkCurrentElementToObject(ObjectPtr object);

  ExecutionContext& getContext()
    {return context;}

private:
  ExecutionContext& context;
  juce::XmlElement* root;

  std::vector<juce::XmlElement* > stack;

  typedef std::map<string, ObjectPtr> SharedObjectsMap;
  SharedObjectsMap sharedObjects;

  bool addSharedObject(const string& name, ObjectPtr object);
  ObjectPtr getSharedObject(const string& name) const;

  std::set<std::pair<ClassPtr, string> > unknownVariables; // store unknown variables to produce warnings only once
};

}; /* namespace lbcpp */

#endif // !OIL_CORE_XML_SERIALISATION_H_
