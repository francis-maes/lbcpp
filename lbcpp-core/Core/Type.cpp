/*-----------------------------------------.---------------------------------.
| Filename: Class.cpp                      | The class interface for         |
| Author  : Francis Maes                   |  introspection                  |
| Started : 24/06/2010 11:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core.h>
#include <map>
using namespace lbcpp;

ClassPtr lbcpp::topLevelType;
ClassPtr lbcpp::anyType;

int lbcpp::integerMissingValue = 0;
double lbcpp::doubleMissingValue = 0.0;

Class::Class(const String& className, ClassPtr baseType)
  : NameableObject(className), initialized(false), baseType(baseType), namedType(false) {}

Class::Class(TemplateTypePtr templateType, const std::vector<ClassPtr>& templateArguments, ClassPtr baseType)
  : NameableObject(templateType->makeTypeName(templateArguments)), initialized(false),
      baseType(baseType), templateType(templateType), templateArguments(templateArguments), namedType(false)
 {}

Class::~Class() {}

bool Class::initialize(ExecutionContext& context)
  {return (initialized = true);}

void Class::deinitialize()
{
  baseType = ClassPtr();
  templateType = TemplateTypePtr();
  templateArguments.clear();
  initialized = false;
}

ClassPtr Class::getClass() const
  {return classClass;}

String Class::toString() const
{
  String res = getName();
  res += T(" = {");
  if (baseType)
  {
    size_t n = getNumMemberVariables();
    for (size_t i = 0; i < n; ++i)
    {
      res += getMemberVariableType(i)->getName() + T(" ") + getMemberVariableName(i);
      if (i < n - 1)
        res += T(", ");
    }
  }
  else
    res += T("!!missing base type!!");
  res += T("}");
  return res;
}

void Class::saveToXml(XmlExporter& exporter) const
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

ClassPtr Class::findCommonBaseClass(ClassPtr type1, ClassPtr type2)
{
  if (type1->inheritsFrom(type2))
    return type2;
  if (type2->inheritsFrom(type1))
    return type1;
  if (type1 == topLevelType || type2 == topLevelType)
    return topLevelType;
  ClassPtr baseType1 = type1->getBaseType();
  ClassPtr baseType2 = type2->getBaseType();
  jassert(baseType1 && baseType2);
  baseType1 = findCommonBaseClass(baseType1, type2);
  baseType2 = findCommonBaseClass(type1, baseType2);
  return baseType1->inheritsFrom(baseType2) ? baseType1 : baseType2;
}

bool Class::inheritsFrom(ClassPtr baseType) const
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

ClassPtr Class::findBaseTypeFromTemplateName(const String& templateName) const
{
  ClassPtr res = refCountedPointerFromThis(this);
  while (res)
  {
    if (res->templateType && res->templateType->getName() == templateName)
      return res;
    res = res->getBaseType();
  }
  return ClassPtr();
}

bool Class::canBeCastedTo(ClassPtr targetType) const
  {return inheritsFrom(targetType);}

ObjectPtr Class::createObject(ExecutionContext& context) const
  {jassert(baseType); return baseType->createObject(context);}

bool Class::isConvertibleToBoolean() const
  {jassert(baseType); return baseType->isConvertibleToBoolean();}

bool Class::isConvertibleToDouble() const
  {jassert(baseType); return baseType->isConvertibleToDouble();}

size_t Class::getNumMemberVariables() const
  {jassert(baseType); return baseType->getNumMemberVariables();}

VariableSignaturePtr Class::getMemberVariable(size_t index) const
  {jassert(baseType); return baseType->getMemberVariable(index);}

int Class::findMemberVariable(const String& name) const
  {jassert(baseType); return baseType->findMemberVariable(name);}
  
ObjectPtr Class::getMemberVariableValue(const Object* pthis, size_t index) const
  {jassert(baseType); return baseType->getMemberVariableValue(pthis, index);}

void Class::setMemberVariableValue(Object* pthis, size_t index, const ObjectPtr& subValue) const
  {if (baseType) baseType->setMemberVariableValue(pthis, index, subValue);}

String Class::makeUniqueMemberVariableName(const String& name) const
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

VariableSignaturePtr Class::getLastMemberVariable() const
{
  size_t n = getNumMemberVariables();
  jassert(n);
  return getMemberVariable(n - 1);
}

ClassPtr Class::getMemberVariableType(size_t index) const
{
  VariableSignaturePtr signature = getMemberVariable(index);
  return signature ? signature->getType() : ClassPtr();
}

String Class::getMemberVariableName(size_t index) const
{
  VariableSignaturePtr signature = getMemberVariable(index);
  return signature ? signature->getName() : String::empty;
}

String Class::getMemberVariableShortName(size_t index) const
{
  VariableSignaturePtr signature = getMemberVariable(index);
  return signature ? signature->getShortName() : String::empty;
}

String Class::getMemberVariableDescription(size_t index) const
{
  VariableSignaturePtr signature = getMemberVariable(index);
  return signature ? signature->getDescription() : String::empty;
}

size_t Class::getNumMemberFunctions() const
  {jassert(baseType); return baseType->getNumMemberFunctions();}

FunctionSignaturePtr Class::getMemberFunction(size_t index) const
  {jassert(baseType); return baseType->getMemberFunction(index);}

int Class::findMemberFunction(const String& name) const
  {jassert(baseType); return baseType->findMemberFunction(name);}
