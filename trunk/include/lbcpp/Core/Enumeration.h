/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2011 by Francis Maes, francis.maes@lip6.fr.
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
| Filename: Enumeration.h                  | Enumeration introspection       |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 18:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_ENUMERATION_H_
# define LBCPP_CORE_ENUMERATION_H_

# include "Type.h"

namespace lbcpp
{

class EnumerationElement : public Object
{
public:
  EnumerationElement(const String& name = String::empty, const String& oneLetterCode = String::empty, const String& shortName = String::empty, const String& description = String::empty)
    : name(name), oneLetterCode(oneLetterCode), shortName(shortName), description(description) {}

  virtual String getName() const
    {return name;}

  const String& getOneLetterCode() const
    {return oneLetterCode;}

  const String& getShortName() const
    {return shortName;}

  const String& getDescription() const
    {return description;}

private:
  friend class EnumerationElementClass;

  String name;
  String oneLetterCode;
  String shortName;
  String description;
};

typedef ReferenceCountedObjectPtr<EnumerationElement> EnumerationElementPtr;

class Enumeration : public Type
{
public:
  Enumeration(const String& name);
  Enumeration(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
    : Type(templateType, templateArguments, baseClass) {}

  virtual ClassPtr getClass() const;

  virtual VariableValue create(ExecutionContext& context) const;
  virtual VariableValue createFromString(ExecutionContext& context, const String& value) const;
  virtual VariableValue createFromXml(XmlImporter& importer) const;
  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const;

  virtual VariableValue getMissingValue() const;

  virtual String toString(const VariableValue& value) const;

  // elements
  virtual size_t getNumElements() const = 0;
  virtual EnumerationElementPtr getElement(size_t index) const = 0;

  String getElementName(size_t index) const
    {return getElement(index)->getName();}

  int findElementByName(const String& name) const;
  int findElementByOneLetterCode(const juce::tchar c) const;

  bool hasOneLetterCodes() const;

  lbcpp_UseDebuggingNewOperator
};

extern EnumerationPtr addMissingToEnumerationEnumeration(TypePtr type);
extern EnumerationPtr addEntropyToEnumerationEnumeration(TypePtr type);
extern EnumerationPtr missingOrPresentEnumeration;
extern EnumerationPtr variablesEnumerationEnumeration(TypePtr type);
extern EnumerationPtr positiveIntegerEnumerationEnumeration;

class DefaultEnumeration : public Enumeration
{
public:
  DefaultEnumeration(const String& name, const juce::tchar** elements, const String& oneLetterCodes = String::empty);
  DefaultEnumeration(const String& name, const String& oneLetterCodes);
  DefaultEnumeration(const String& name);

  virtual size_t getNumElements() const
    {return elements.size();}

  virtual EnumerationElementPtr getElement(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  void addElement(ExecutionContext& context, const String& elementName, const String& oneLetterCode = String::empty, const String& shortName = String::empty, const String& description = String::empty);
  size_t findOrAddElement(ExecutionContext& context, const String& name);
  
  void addElementsWithPrefix(ExecutionContext& context, const EnumerationPtr& enumeration, const String& namePrefix, const String& shortNamePrefix);

private:
  std::vector<EnumerationElementPtr> elements;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_ENUMERATION_H_
