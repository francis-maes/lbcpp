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

# include "DefaultClass.h"

namespace lbcpp
{

class EnumerationElement : public Object
{
public:
  EnumerationElement(const String& name = String::empty, const String& oneLetterCode = String::empty, const String& shortName = String::empty, const String& description = String::empty);

  const String& getName() const
    {return name;}

  const String& getOneLetterCode() const
    {return oneLetterCode;}

  const String& getShortName() const
    {return shortName;}

  const String& getDescription() const
    {return description;}

  virtual String toShortString() const;

  void setName(const String& name)
    {this->name = name;}

private:
  friend class EnumerationElementClass;

  String name;
  String oneLetterCode;
  String shortName;
  String description;
};

typedef ReferenceCountedObjectPtr<EnumerationElement> EnumerationElementPtr;

class Enumeration : public Class
{
public:
  Enumeration(const String& name, const String& baseTypeName = T("NewEnumValue"));
  Enumeration(TemplateClassPtr templateType, const std::vector<ClassPtr>& templateArguments, ClassPtr baseClass)
    : Class(templateType, templateArguments, baseClass) {}

  virtual ClassPtr getClass() const;

  virtual ObjectPtr createObject(ExecutionContext& context) const;

  // elements
  virtual size_t getNumElements() const = 0;
  virtual EnumerationElementPtr getElement(size_t index) const = 0;

  String getElementName(size_t index) const;

  virtual int findElementByName(const String& name) const;
  int findElementByOneLetterCode(const juce::tchar c) const;

  bool hasOneLetterCodes() const;
  
  virtual int compare(const ObjectPtr& otherObject) const;

  lbcpp_UseDebuggingNewOperator
};

extern ClassPtr enumerationClass;

extern EnumerationPtr addMissingToEnumerationEnumeration(ClassPtr type);
extern EnumerationPtr addEntropyToEnumerationEnumeration(ClassPtr type);
extern EnumerationPtr singletonEnumeration;
extern EnumerationPtr existOrMissingEnumeration;
extern EnumerationPtr falseOrTrueEnumeration;
extern EnumerationPtr falseTrueOrMissingEnumeration;
extern EnumerationPtr variablesEnumerationEnumeration(ClassPtr type);
extern EnumerationPtr positiveIntegerEnumerationEnumeration;

extern EnumerationPtr cartesianProductEnumerationEnumeration(ClassPtr firstType, ClassPtr secondType);

class DefaultEnumeration : public Enumeration
{
public:
  //DefaultEnumeration(const String& name, const juce::tchar** elements, const String& oneLetterCodes = String::empty);
  //DefaultEnumeration(const String& name, const String& oneLetterCodes);
  DefaultEnumeration(const String& name, const String& baseTypeName = T("NewEnumValue"));
  DefaultEnumeration();

  virtual size_t getNumElements() const
    {return elements.size();}

  virtual EnumerationElementPtr getElement(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  void addElement(ExecutionContext& context, const String& elementName, const String& oneLetterCode = String::empty, const String& shortName = String::empty, const String& description = String::empty);
  virtual int findElementByName(const String& name) const;

  size_t findOrAddElement(ExecutionContext& context, const String& name);
  
  void addElementsWithPrefix(ExecutionContext& context, const EnumerationPtr& enumeration, const String& namePrefix, const String& shortNamePrefix);

  void reserveElements(size_t size)
    {elements.reserve(size);}

private:
  friend class DefaultEnumerationClass;
  
  std::vector<EnumerationElementPtr> elements;
  std::map<String, size_t> elementsMap;
};

class ConcatenateEnumeration : public Enumeration
{
public:
  ConcatenateEnumeration(const String& name = T("UnnamedEnumeration"));

  virtual size_t getNumElements() const;
  virtual EnumerationElementPtr getElement(size_t index) const;

  size_t getNumSubEnumerations() const
    {return subEnumerations.size();}

  const String& getSubEnumerationPrefix(size_t index) const
    {jassert(index < subEnumerations.size()); return subEnumerations[index].first;}

  const EnumerationPtr& getSubEnumeration(size_t index) const
    {jassert(index < subEnumerations.size()); return subEnumerations[index].second;}

  void reserveSubEnumerations(size_t size)
    {subEnumerations.reserve(size);}

  void addSubEnumeration(const String& prefix, const EnumerationPtr& enumeration);

private:
  friend class ConcatenateEnumerationClass;

  std::vector< std::pair<String, EnumerationPtr> > subEnumerations;
  std::map<size_t, size_t> indexToBaseIndex; // element index -> base enumeration index
};

typedef ReferenceCountedObjectPtr<ConcatenateEnumeration> ConcatenateEnumerationPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_ENUMERATION_H_
