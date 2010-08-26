/*-----------------------------------------.---------------------------------.
| Filename: Variable.cpp                   | Variable                        |
| Author  : Francis Maes                   |                                 |
| Started : 08/08/2010 12:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Data/Variable.h>
#include "Type/TupleType.h"
using namespace lbcpp;

Variable Variable::pair(const Variable& variable1, const Variable& variable2)
{
  TypePtr type = pairType(variable1.getType(), variable2.getType());
  jassert(type->getNumTemplateArguments() == 2);
  return Variable(type, PairType::allocate(variable1, variable2));
}

Variable Variable::copyFrom(TypePtr type, const VariableValue& value)
{
  Variable res;
  res.type = type;
  if (type)
    type->copy(res.value, value);
  return res;
}

Variable Variable::createFromString(TypePtr type, const String& str, MessageCallback& callback)
{
  String failureReason;
  VariableValue value = type->createFromString(str, callback);
  return type->isMissingValue(value) ? Variable() : Variable(type, value);
}

Variable Variable::createFromXml(TypePtr type, XmlImporter& importer)
  {return Variable(type, type->createFromXml(importer));}

Variable Variable::createFromFile(const File& file, MessageCallback& callback)
{
  XmlImporter importer(file, callback);
  return importer.isOpened() ? importer.load() : Variable();
}

void Variable::saveToXml(XmlExporter& exporter) const
{
  if (!type->isMissingValue(value))
    type->saveToXml(exporter, value);
}

bool Variable::saveToFile(const File& file, MessageCallback& callback) const
{
  XmlExporter exporter;
  exporter.saveVariable(String::empty, *this);
  return exporter.saveToFile(file, callback);
}

int Variable::compare(const Variable& otherValue) const
{
  TypePtr type2 = otherValue.getType();
  if (type != type2)
  {
    if (type->inheritsFrom(type2))
      return type2->compare(value, otherValue.value);
    else if (type2->inheritsFrom(type))
      return type->compare(value, otherValue.value);
    else
      return getTypeName().compare(otherValue.getTypeName());
  }
  return type->compare(value, otherValue.value);
}

static void printVariableLine(const Variable& value, std::ostream& ostr, size_t variableNumber, const String& name, int currentDepth)
{
  for (int i = 0; i < currentDepth; ++i)
    ostr << "  ";
  if (variableNumber != (size_t)-1)
    ostr << "[" << variableNumber << "] ";
  ostr << (const char* )value.getTypeName();
  if (name.isNotEmpty())
    ostr << " " << name;
  String v = value.toString();
  if (v.length() > 30)
    v = v.substring(0, 30) + T("...");
  ostr << " = " << v << std::endl;
}

static void printVariablesRecursively(const Variable& variable, std::ostream& ostr, int maxDepth, int currentDepth, bool displayMissingValues)
{
  if (maxDepth >= 0 && currentDepth >= maxDepth)
    return;
  TypePtr type = variable.getType();
  if (type->inheritsFrom(objectClass()))
  {
    ObjectPtr object = variable.getObject();
    if (object)
      for (size_t i = 0; i < type->getObjectNumVariables(); ++i)
      {
        Variable subVariable = object->getVariable(i);
        if (displayMissingValues || subVariable)
        {
          printVariableLine(subVariable, ostr, i, type->getObjectVariableName(i), currentDepth);
          printVariablesRecursively(subVariable, ostr, maxDepth, currentDepth + 1, displayMissingValues);
        }
      }
  }
  for (size_t i = 0; i < variable.size(); ++i)
  {
    Variable subVariable = variable[i];
    if (displayMissingValues || subVariable)
    {
      printVariableLine(subVariable, ostr, (size_t)-1, variable.getName(i), currentDepth);
      printVariablesRecursively(subVariable, ostr, maxDepth, currentDepth + 1, displayMissingValues);
    }
  }
}

void Variable::printRecursively(std::ostream& ostr, int maxDepth, bool displayMissingValues)
{
  printVariableLine(*this, ostr, (size_t)-1, String::empty, 0);
  printVariablesRecursively(*this, ostr, maxDepth, 1, displayMissingValues);
}
