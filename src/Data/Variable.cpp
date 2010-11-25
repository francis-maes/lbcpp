/*-----------------------------------------.---------------------------------.
| Filename: Variable.cpp                   | Variable                        |
| Author  : Francis Maes                   |                                 |
| Started : 08/08/2010 12:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Data/Variable.h>
#include <lbcpp/Data/Pair.h>
#include <lbcpp/Data/Container.h>
#include <lbcpp/Data/XmlSerialisation.h>
using namespace lbcpp;

Variable Variable::pair(const Variable& variable1, const Variable& variable2)
  {return new Pair(variable1, variable2);}

Variable Variable::pair(const Variable& variable1, const Variable& variable2, TypePtr pairType)
  {return new Pair(pairType, variable1, variable2);}

Variable Variable::copyFrom(TypePtr type, const VariableValue& value)
{
  Variable res;
  res.type = type.get();
  if (type)
    type->copy(res.value, value);
  return res;
}

Variable Variable::createFromString(ExecutionContext& context, TypePtr type, const String& str)
{
  VariableValue value = type->createFromString(context, str);
  return type->isMissingValue(value) ? Variable() : Variable(type, value);
}

Variable Variable::createFromXml(TypePtr type, XmlImporter& importer)
  {return Variable(type, type->createFromXml(importer));}

Variable Variable::createFromFile(ExecutionContext& context, const File& file)
{
  XmlImporter importer(context, file);
  return importer.isOpened() ? importer.load() : Variable();
}

void Variable::saveToXml(XmlExporter& exporter) const
{
  if (!type->isMissingValue(value))
    type->saveToXml(exporter, value);
}

bool Variable::saveToFile(ExecutionContext& context, const File& file) const
{
  XmlExporter exporter;
  exporter.saveVariable(String::empty, *this, TypePtr());
  return exporter.saveToFile(context, file);
}

int Variable::compare(const Variable& otherValue) const
{
  TypePtr type2 = otherValue.type;
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

Variable Variable::clone(ExecutionContext& context) const
{
  if (isObject())
  {
    ObjectPtr object = getObject();
    return Variable(object ? object->clone(context) : ObjectPtr(), type);
  }
  else
    return *this;
}

static void printVariableLine(const Variable& value, std::ostream& ostr, size_t variableNumber, const String& name, int currentDepth, bool displayType, bool displayValue)
{
  for (int i = 0; i < currentDepth; ++i)
    ostr << "  ";
  if (variableNumber != (size_t)-1)
    ostr << "[" << variableNumber << "]";
  if (displayType)
    ostr << " " << (const char* )value.getTypeName();
  if (name.isNotEmpty())
    ostr << " " << name;
  if (displayValue)
  {
    String v = value.toString();
    if (v.length() > 30)
      v = v.substring(0, 30) + T("...");
    ostr << " = " << v;
  }
  ostr << std::endl;
}

static bool hasPrintChildren(const Variable& variable, int maxDepth, int currentDepth)
{
  if (maxDepth >= 0 && currentDepth >= maxDepth)
    return false;

  if (variable.size() > 0)
    return true;

  TypePtr type = variable.getType();
  if (type->inheritsFrom(objectClass))
  {
    ObjectPtr object = variable.getObject();
    if (object && type->getObjectNumVariables())
      return true;
  }

  return false;
}

static void printVariablesRecursively(const Variable& variable, std::ostream& ostr, int maxDepth, int currentDepth, bool displayMissingValues, bool displayTypes)
{
  if (maxDepth >= 0 && currentDepth >= maxDepth)
    return;
  TypePtr type = variable.getType();
  if (type->inheritsFrom(objectClass))
  {
    ObjectPtr object = variable.getObject();
    if (object)
      for (size_t i = 0; i < type->getObjectNumVariables(); ++i)
      {
        Variable subVariable = object->getVariable(i);
        if (displayMissingValues || subVariable.exists())
        {
          printVariableLine(subVariable, ostr, i, type->getObjectVariableName(i), currentDepth, displayTypes, !hasPrintChildren(subVariable, maxDepth, currentDepth + 1));
          printVariablesRecursively(subVariable, ostr, maxDepth, currentDepth + 1, displayMissingValues, displayTypes);
        }
      }
  }
  for (size_t i = 0; i < variable.size(); ++i)
  {
    Variable subVariable = variable[i];
    if (displayMissingValues || subVariable.exists())
    {
      printVariableLine(subVariable, ostr, (size_t)-1, variable.getName(i), currentDepth, displayTypes, !hasPrintChildren(subVariable, maxDepth, currentDepth + 1));
      printVariablesRecursively(subVariable, ostr, maxDepth, currentDepth + 1, displayMissingValues, displayTypes);
    }
  }
}

void Variable::printRecursively(std::ostream& ostr, int maxDepth, bool displayMissingValues, bool displayTypes)
{
  printVariableLine(*this, ostr, (size_t)-1, String::empty, 0, displayTypes, false);
  printVariablesRecursively(*this, ostr, maxDepth, 1, displayMissingValues, displayTypes);
}

static bool printDifferencesRecursively(std::ostream& ostr, const Variable& variable1, const Variable& variable2, const String& name)
{
  static bool countTypeDifferences = false;

  if (variable1.isNil() || variable2.isNil() || variable1.isMissingValue() || variable2.isMissingValue() || !variable1.isObject() || !variable2.isObject())
  {
    if (!countTypeDifferences && variable1.isMissingValue() && variable2.isMissingValue())
      return true;

    if (variable1 == variable2)
      return true;
    else
    {
      ostr << name << " variable1 = " << variable1.toShortString()
                   << " variable2 = " << variable2.toShortString() << std::endl;
      return false;
    }
  }

  if (countTypeDifferences && variable1.getType() != variable2.getType())
  {
    ostr << name << " type1 = " << variable1.getType()->getName()
                 << " type2 = " << variable2.getType()->getName() << std::endl;
  }

  bool res = true;
  ObjectPtr object1 = variable1.getObject();
  ObjectPtr object2 = variable2.getObject();
  jassert(object1 && object2);

  if (object1->getNumVariables() != object2->getNumVariables())
  {
    ostr << name << " numVariables1 = " << object1->getNumVariables() << " numVariable2 = " << object2->getNumVariables() << std::endl;
    return false;
  }

  if (object1.dynamicCast<Type>())
  {
    if (!countTypeDifferences || object1->getClassName() == T("DynamicClass"))
      return true; // ignore this
    if (object1 == object2)
      return true;
    else
    {
      ostr << name << " typeValue1 = " << variable1.toShortString()
                   << " typeValue2 = " << variable2.toShortString() << std::endl;
      return false;
    }
  }

  size_t n = object1->getNumVariables();
  jassert(object2->getNumVariables() == n);
  for (size_t i = 0; i < n; ++i)
  {
    if (object1->getVariableName(i) != object2->getVariableName(i))
    {
      ostr << name << " varName1 = " << object1->getVariableName(i)
                   << " varName2 = " << object2->getVariableName(i) << std::endl;
    }
    String newName = object1->getVariableName(i);
    if (name.isNotEmpty())
      newName = name + T(".") + newName;
    res &= printDifferencesRecursively(ostr, object1->getVariable(i), object2->getVariable(i), newName);
  }
  
  ContainerPtr container1 = object1.dynamicCast<Container>();
  ContainerPtr container2 = object2.dynamicCast<Container>();
  if (container1 && container2)
  {
    n = container1->getNumElements();
    if (n != container2->getNumElements())
    {
      ostr << name << " container1 size = " << n
                   << " container2 size " << container2->getNumElements() << std::endl;
      return false;
    }
    for (size_t i = 0; i < n; ++i)
      res &= printDifferencesRecursively(ostr, container1->getElement(i), container2->getElement(i), name + T("[") + String((int)i) + T("]"));
  }
  return res;
}

bool Variable::printDifferencesRecursively(std::ostream& ostr, const Variable& otherVariable, const String& theseVariablesName) const
  {return ::printDifferencesRecursively(ostr, *this, otherVariable, theseVariablesName);}
