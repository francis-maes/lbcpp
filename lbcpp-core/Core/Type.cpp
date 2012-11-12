/*-----------------------------------------.---------------------------------.
| Filename: Type.cpp                       | The class interface for         |
| Author  : Francis Maes                   |  introspection                  |
| Started : 24/06/2010 11:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core.h>
#include <map>
using namespace lbcpp;

TypePtr lbcpp::topLevelType;
TypePtr lbcpp::anyType;

int lbcpp::integerMissingValue = 0;
double lbcpp::doubleMissingValue = 0.0;

/*
** Type
*/
Type::Type(const String& className, TypePtr baseType)
  : NameableObject(className), initialized(false), baseType(baseType), namedType(false) {}

Type::Type(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseType)
  : NameableObject(templateType->makeTypeName(templateArguments)), initialized(false),
      baseType(baseType), templateType(templateType), templateArguments(templateArguments), namedType(false)
 {}

Type::~Type() {}

bool Type::initialize(ExecutionContext& context)
  {return (initialized = true);}

void Type::deinitialize()
{
  baseType = TypePtr();
  templateType = TemplateTypePtr();
  templateArguments.clear();
  initialized = false;
}

ClassPtr Type::getClass() const
  {return typeClass;}

/*
** Xml Serialisation
*/
void Type::saveToXml(XmlExporter& exporter) const
{
  jassert(!namedType);
  if (templateType)
  {
    exporter.setAttribute(T("templateType"), templateType->getName());
    for (size_t i = 0; i < templateArguments.size(); ++i)
    {
      exporter.enter(T("templateArgument"));
      exporter.setAttribute(T("index"), (int)i);
      exporter.writeType(templateArguments[i]);
      exporter.leave();
    }
  }
  else
    exporter.setAttribute(T("typeName"), name);
}

TypePtr Type::loadUnnamedTypeFromXml(XmlImporter& importer)
{
  // load unnamed type from xml
  if (importer.hasAttribute(T("templateType")))
  {
    String templateType = importer.getStringAttribute(T("templateType"));
    std::vector<TypePtr> templateArguments;
    forEachXmlChildElementWithTagName(*importer.getCurrentElement(), elt, T("templateArgument"))
    {
      importer.enter(elt);
      int index = importer.getIntAttribute(T("index"));
      if (index < 0)
      {
        importer.errorMessage(T("Type::loadTypeFromXml"), T("Invalid template argument index"));
        return TypePtr();
      }
      templateArguments.resize(index + 1);
      templateArguments[index] = importer.loadType(TypePtr());
      if (!templateArguments[index])
        return TypePtr();
      importer.leave();
    }
    return typeManager().getType(importer.getContext(), templateType, templateArguments);
  }
  else
    return typeManager().getType(importer.getContext(), importer.getStringAttribute(T("typeName")));
}

/*
** Type Operations
*/
TypePtr Type::findCommonBaseType(TypePtr type1, TypePtr type2)
{
  if (type1->inheritsFrom(type2))
    return type2;
  if (type2->inheritsFrom(type1))
    return type1;
  if (type1 == topLevelType || type2 == topLevelType)
    return topLevelType;
  TypePtr baseType1 = type1->getBaseType();
  TypePtr baseType2 = type2->getBaseType();
  jassert(baseType1 && baseType2);
  baseType1 = findCommonBaseType(baseType1, type2);
  baseType2 = findCommonBaseType(type1, baseType2);
  return baseType1->inheritsFrom(baseType2) ? baseType1 : baseType2;
}

bool Type::inheritsFrom(TypePtr baseType) const
{
  jassert(this && baseType.get());

  if (this == baseType.get())
    return true;

  if (!this->baseType)
    return false;

  if (this->templateType && this->templateType == baseType->templateType)
  {
    jassert(this->templateArguments.size() == baseType->templateArguments.size());
    for (size_t i = 0; i < this->templateArguments.size(); ++i)
      if (!this->templateArguments[i]->inheritsFrom(baseType->templateArguments[i]))
        return false;
    return true;
  }

  return this->baseType->inheritsFrom(baseType);
}

TypePtr Type::findBaseTypeFromTemplateName(const String& templateName) const
{
  TypePtr res = refCountedPointerFromThis(this);
  while (res)
  {
    if (res->templateType && res->templateType->getName() == templateName)
      return res;
    res = res->getBaseType();
  }
  return TypePtr();
}

bool Type::canBeCastedTo(TypePtr targetType) const
  {return inheritsFrom(targetType);}

/*
** Instance basic operations
*/
VariableValue Type::getMissingValue() const
{
  jassert(sizeof (VariableValue) == 8);
  static const juce::int64 missing = 0x0FEEFEEEFEEEFEEELL;
  return VariableValue(missing);
}

bool Type::isMissingValue(const VariableValue& value) const
{
  VariableValue missing = getMissingValue();
  return value.getInteger() == missing.getInteger();
}

ObjectPtr Type::create(ExecutionContext& context) const
  {jassert(baseType); return baseType->create(context);}

ObjectPtr Type::createFromString(ExecutionContext& context, const String& value) const
  {jassert(baseType); return baseType->createFromString(context, value);}

ObjectPtr Type::createFromXml(XmlImporter& importer) const
  {jassert(baseType); return baseType->createFromXml(importer);}

String Type::toString(const VariableValue& value) const
  {jassert(baseType); return baseType->toString(value);}

bool Type::isConvertibleToBoolean() const
  {jassert(baseType); return baseType->isConvertibleToBoolean();}

bool Type::isConvertibleToDouble() const
  {jassert(baseType); return baseType->isConvertibleToDouble();}

void Type::destroy(VariableValue& value) const
  {jassert(baseType); baseType->destroy(value);}

void Type::copy(VariableValue& dest, const VariableValue& source) const
  {jassert(baseType); baseType->copy(dest, source);}

int Type::compare(const VariableValue& value1, const VariableValue& value2) const
  {jassert(baseType); return baseType->compare(value1, value2);}

void Type::saveToXml(XmlExporter& exporter, const VariableValue& value) const
  {jassert(baseType); return baseType->saveToXml(exporter, value);}

/*
** Member Variables
*/
size_t Type::getNumMemberVariables() const
  {jassert(baseType); return baseType->getNumMemberVariables();}

VariableSignaturePtr Type::getMemberVariable(size_t index) const
  {jassert(baseType); return baseType->getMemberVariable(index);}

int Type::findMemberVariable(const String& name) const
  {jassert(baseType); return baseType->findMemberVariable(name);}
  
ObjectPtr Type::getMemberVariableValue(const Object* pthis, size_t index) const
  {jassert(baseType); return baseType->getMemberVariableValue(pthis, index);}

void Type::setMemberVariableValue(Object* pthis, size_t index, const ObjectPtr& subValue) const
  {if (baseType) baseType->setMemberVariableValue(pthis, index, subValue);}

String Type::makeUniqueMemberVariableName(const String& name) const
{
  if (findMemberVariable(name) < 0)
    return name;
  for (int i = 2; true; ++i)
  {
    String res = name + String(i);
    if (findMemberVariable(res) < 0)
      return res;
  }
  return String::empty;
}

VariableSignaturePtr Type::getLastMemberVariable() const
{
  size_t n = getNumMemberVariables();
  jassert(n);
  return getMemberVariable(n - 1);
}

TypePtr Type::getMemberVariableType(size_t index) const
{
  VariableSignaturePtr signature = getMemberVariable(index);
  return signature ? signature->getType() : TypePtr();
}

String Type::getMemberVariableName(size_t index) const
{
  VariableSignaturePtr signature = getMemberVariable(index);
  return signature ? signature->getName() : String::empty;
}

String Type::getMemberVariableShortName(size_t index) const
{
  VariableSignaturePtr signature = getMemberVariable(index);
  return signature ? signature->getShortName() : String::empty;
}

String Type::getMemberVariableDescription(size_t index) const
{
  VariableSignaturePtr signature = getMemberVariable(index);
  return signature ? signature->getDescription() : String::empty;
}

/*
** Member Functions
*/
size_t Type::getNumMemberFunctions() const
  {jassert(baseType); return baseType->getNumMemberFunctions();}

FunctionSignaturePtr Type::getMemberFunction(size_t index) const
  {jassert(baseType); return baseType->getMemberFunction(index);}

int Type::findMemberFunction(const String& name) const
  {jassert(baseType); return baseType->findMemberFunction(name);}
