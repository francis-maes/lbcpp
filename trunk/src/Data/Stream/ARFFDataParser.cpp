/*-----------------------------------------.---------------------------------.
| Filename: ARFFDataParser.cpp             | ARFF Data Parser                |
| Author  : Julien Becker                  |                                 |
| Started : 10/07/2010 15:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ARFFDataParser.h"

using namespace lbcpp;

bool ARFFDataParser::parseLine(const String& line)
{
  line.trim();
  if (line.startsWith(T("%")))                              // Skip comment line
    return true;
  if (line == String::empty)
    return true;                // an empty line is considered as a comment line
  if (shouldReadData)                                   // line *must* be a data
  {
    if (line.startsWithChar(T('{')))
        return parseSparseDataLine(line);
    return parseDataLine(line);
  }
  String record = line.substring(0, juce::jmin(10, line.length())).toLowerCase();
  if (record.startsWith(T("@attribute")))
      return parseAttributeLine(line);
  if (record.startsWith(T("@relation")))  // @relation *must* be declared before
  {                                       // any @attribute declaration
    if (attributesType.size())
    {
      context.warningCallback(T("ARFFDataParser::parseLine"), T("@relation must be declared before any @attribute declaration"));
      return false;
    }
    return true;                                     // skip the relation's name
  }
  if (record.startsWith(T("@data")))
  {
    if (attributesType.size() < 2)
    {
      context.warningCallback(T("ARFFDataParser::parseLine"), T("Not enough attribute descriptor"));
      return false;
    }
    supervisionType = attributesType.back(); // The last attribute is considered
    attributesType.pop_back();               // as the supervision
    
    shouldReadData = true;     // Indicate that the remaining lines will be data
    return checkOrAddAttributesTypeToFeatures() && checkSupervisionType();
  }
  context.warningCallback(T("ARFFDataParser::parseLine"), T("Unknown expression: ") + line.quoted());
  return false;
}

bool ARFFDataParser::parseAttributeLine(const String& line)
{
  if (line.getLastCharacter() == T('}'))
    return parseEnumerationAttributeLine(line);

  int b = line.lastIndexOfAnyOf(T(" \t"));
  if (b < 0)
  {
    context.errorCallback(T("ARFFDataParser::parseAttributeLine"), T("Malformatted attribute description: ") + line.quoted());
    return false;
  }

  StringArray tokens;
  tokens.addTokens(line.substring(0, b).trim(), T(" \t"), T("'\""));
  if (tokens.size() != 2)
  {
    context.errorCallback(T("ARFFDataParser::parseAttributeLine"), T("Malformatted attribute name: ") + line.quoted());
    return false;
  }
  attributesName.push_back(tokens[1].unquoted());
  
  String attributeTypeName = line.substring(b + 1).toLowerCase();
  if (attributeTypeName == T("float")
      || attributeTypeName == T("real")
      || attributeTypeName == T("numeric"))
  {
    attributesType.push_back(doubleType);
    return true;
  }

  if (attributeTypeName == T("integer"))
  {
    attributesType.push_back(integerType);
    return true;
  }

  if (attributeTypeName == T("string"))
  {
    context.errorCallback(T("ARFFDataParser::parseAttributeLine"), T("String format is not (yet) supported: ") + line.quoted());
    return false;
  }

  context.errorCallback(T("ARFFDataParser::parseAttributeLine"), T("Unknown attribute type: ") + attributeTypeName.quoted());
  return false;
}

static inline bool comparePairOfStrings(const String& a, const String& b, const String& x, const String& y)
  {return (a == x && b == y) || (a == y && b == x);}

static bool shouldBeABooleanType(const StringArray& tokens)
{
  if (tokens.size() != 2)
    return false;
  String first = tokens[0].toLowerCase();
  String second = tokens[1].toLowerCase();
  if (comparePairOfStrings(first, second, T("yes"), T("no")))
    return true;
  if (comparePairOfStrings(first, second, T("true"), T("false")))
    return true;
  if (comparePairOfStrings(first, second, T("+"), T("-")))
    return true;
  if (comparePairOfStrings(first, second, T("0"), T("1")))
    return true;
  return false;
}

static Variable createBooleanFromString(ExecutionContext& context, const String& str)
{
  String value = str.unquoted().toLowerCase();
  if (value == T("yes"))
    return true;
  if (value == T("no"))
    return false;
  if (value == T("true"))
    return true;
  if (value == T("false"))
    return false;
  if (value == T("+"))
    return true;
  if (value == T("-"))
    return false;
  if (value == T("0"))
    return false;
  if (value == T("1"))
    return true;
  context.errorCallback(T("ARFFDataParser::createBooleanFromString"), T("The value is not a boolean: ") + str.quoted());
  return Variable::missingValue(booleanType);
}

bool ARFFDataParser::parseEnumerationAttributeLine(const String& line)
{
  // get enumeration name
  String trimmedLine = line.substring(10).trim();
  int e = trimmedLine.indexOfAnyOf(T(" \t"));
  if (e < 0)
  {
    context.errorCallback(T("ARFFDataParser::parseEnumerationAttributeLine"), T("Malformatted enumeration description: ") + trimmedLine.quoted());
    return false;
  }
  String enumerationName = trimmedLine.substring(0, e).unquoted();
  attributesName.push_back(enumerationName);
  trimmedLine = trimmedLine.substring(e);
  // get enumeration values
  int b = trimmedLine.lastIndexOfChar(T('{'));
  if (b < 0)
  {
    context.errorCallback(T("ARFFDataParser::parseEnumerationAttributeLine"), T("Malformatted enumeration description: ") + trimmedLine.quoted());
    return false;
  }

  String values = trimmedLine.substring(b + 1, trimmedLine.length() - 1);
  StringArray tokens;
  tokens.addTokens(values, T(","), T("'\""));
  if (tokens.size() == 0)
  {
    context.errorCallback(T("ARFFDataParser::parseEnumerationAttributeLine"), T("No enumeration element found in: ") + trimmedLine.quoted());
    return false;
  }
  for (size_t i = 0; i < (size_t)tokens.size(); ++i)
    tokens.set(i, tokens[i].trim().unquoted());
  // It can be a booleanType ?
  if (shouldBeABooleanType(tokens))
  {
    attributesType.push_back(booleanType);
    return true;
  }
  // create enumeration
  DefaultEnumerationPtr enumClass = new DefaultEnumeration(enumerationName + T("ARFFEnum"));
  for (size_t i = 0; i < (size_t)tokens.size(); ++i)
    if (enumClass->findOrAddElement(context, tokens[i].unquoted().trim().replaceCharacters(T(" \t"), T("--"))) != i)
    {
      context.errorCallback(T("ARFFDataParser::parseEnumerationAttributeLine"), T("Duplicate enumeration element found: ") + tokens[i].quoted());
      return false;
    }
  attributesType.push_back(enumClass);
  return true;
}

bool ARFFDataParser::checkOrAddAttributesTypeToFeatures()
{
  size_t n = attributesType.size();
  if (features->getNumMemberVariables() == 0)
  {
    for (size_t i = 0; i < n; ++i)
      if (features->addMemberVariable(context, attributesType[i], attributesName[i]) == (size_t)-1)
        return false;
    return true;
  }

  if (features->getNumMemberVariables() != n)
  {
    context.errorCallback(T("ARFFDataParser::checkOrAddAttributesTypeToFeatures"), T("The number of variables of the expected type does not match the number of attributes"));
    return false;
  }
  for (size_t i = 0; i < n; ++i)
    if (attributesType[i]->inheritsFrom(enumValueType))
    {
      if (attributesType[i].staticCast<Enumeration>()->compare(features->getMemberVariable(i)->getType()) != 0)
        return false;
    }
    else if (attributesType[i] != features->getMemberVariable(i)->getType())
    {
      context.errorCallback(T("ARFFDataParser::checkOrAddAttributesTypeToFeatures"), T("The attribute type does not match the expected type"));
      return false;
    }
  return true;
}

static Variable createFromString(ExecutionContext& context, const TypePtr& type, const String& str)
{
  if (str.length() == 1 && str == T("?"))
    return Variable::missingValue(type);
  if (type == booleanType)
    return createBooleanFromString(context, str);
  return type->createFromString(context, str.unquoted());
}

bool ARFFDataParser::parseDataLine(const String& line)
{
  size_t n = attributesType.size();
  StringArray tokens;
  tokens.addTokens(line, T(", \t"), T("'\""));
  if ((size_t)tokens.size() != n + 1) // attributes + supervision
  {
    context.errorCallback(T("ARFFDataParser::parseDataLine"), T("Invalid number of values in: ") + line.quoted());
    return false;
  }
  // get attributes
  DenseGenericObjectPtr inputs = new DenseGenericObject(features);
  for (size_t i = 0; i < n; ++i)
    inputs->setVariable(i, createFromString(context, attributesType[i], tokens[i]));

  setResult(finalizeData(Variable::pair(inputs, createFromString(context, supervisionType, tokens[n]), getElementsType())));
  return true;
}

bool ARFFDataParser::parseSparseDataLine(const String& line)
{
  size_t n = attributesType.size();
  StringArray tokens;
  tokens.addTokens(line.substring(1, line.length() - 1), T(", "), T("'\""));
  size_t numTokens = tokens.size();
  if (numTokens > n + 1)
  {
    context.errorCallback(T("ARFFDataParser::parseSparseDataLine"), T("Too many values in: ") + line.quoted());
    return false;
  }
  
  // get attributes
  Variable supervision;
  DenseGenericObjectPtr inputs = new DenseGenericObject(features);
  for (size_t i = 0; i < numTokens; ++i)
  {
    int e = tokens[i].indexOfAnyOf(T(" \t"));
    if (e < 0)
    {
      context.errorCallback(T("ARFFDataParser::parseSparseDataLine"), T("Bad index in: ") + tokens[i].quoted());
      return false;
    }
    Variable v = Variable::createFromString(context, positiveIntegerType, tokens[i].substring(0, e));
    if (!v.exists())
    {
      context.errorCallback(T("ARFFDataParser::parseSparseDataLine"), T("Bad index in: ") + tokens[i].quoted());
      return false;
    }
    size_t index = v.getInteger();
    if (index == n)
      supervision = createFromString(context, supervisionType, tokens[i].substring(e).trim());
    else
    {
      if (inputs->getVariable(index).exists())
      {
        context.errorCallback(T("ARFFDataParser::parseSparseDataLine"), T("Duplicate index '" + String((int)index) + "' in: ") + tokens[i].quoted());
        return false;
      }
      inputs->setVariable(index, createFromString(context, attributesType[index], tokens[i].substring(e).trim()));
    }
  }
  setResult(finalizeData(Variable::pair(inputs, supervision, getElementsType())));
  return true;
}

/* Classification ARFFDataParser */
static bool checkOrAddSupervisionTypeToLabels(ExecutionContext& context, TypePtr& supervisionType, DefaultEnumerationPtr& labels)
{
  if (!context.checkInheritance(supervisionType, enumValueType))
    return false;
  if (labels->getNumElements() == 0)
  {
    EnumerationPtr enumType = supervisionType.dynamicCast<Enumeration>();
    for (size_t i = 0; i < enumType->getNumElements(); ++i)
      labels->addElement(context, enumType->getElement(i)->getName());
    supervisionType = labels;
    return true;
  }
  if (labels->compare(supervisionType) == 0)
  {
    supervisionType = labels;
    return true;
  }
  context.errorCallback(T("ARFFDataParser::checkOrAddSupervisionTypeToLabels"), T("An enumeration already exists as supervision type but does not match the current supervision type"));
  return false;
}

bool ClassificationARFFDataParser::checkSupervisionType() const
{
  return checkOrAddSupervisionTypeToLabels(context,
                                           const_cast<ClassificationARFFDataParser*>(this)->supervisionType,
                                           const_cast<ClassificationARFFDataParser*>(this)->labels);
}

bool MultiLabelClassificationARFFDataParser::checkSupervisionType() const
{
  return checkOrAddSupervisionTypeToLabels(context,
                                           const_cast<MultiLabelClassificationARFFDataParser*>(this)->supervisionType,
                                           const_cast<MultiLabelClassificationARFFDataParser*>(this)->labels);
}
