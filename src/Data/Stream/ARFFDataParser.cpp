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
  if (line.startsWith(T("@attribute")))
      return parseAttributeLine(line);
  if (line.startsWith(T("@relation")))    // @relation *must* be declared before
  {                                       // any @attribute declaration
    if (attributesType.size())
    {
      context.warningCallback(T("ARFFDataParser::parseLine"), T("@relation must be declared before any @attribute declaration"));
      return false;
    }
    return true;                                     // skip the relation's name
  }
  if (line.startsWith(T("@data")))
  {
    if (attributesType.size() < 2)
    {
      context.warningCallback(T("ARFFDataParser::parseLine"), T("Not enough attribute descriptor"));
      return false;
    }
    supervisionType = attributesType.back(); // The last attribute is considered
    attributesType.pop_back();               // as the supervision
    shouldReadData = true;     // Indicate that the remaining lines will be data
    return checkSupervisionType();
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
  // create enumeration
  DefaultEnumerationPtr enumClass = new DefaultEnumeration(enumerationName + T("ARFFEnum"));
  for (size_t i = 0; i < (size_t)tokens.size(); ++i)
    if (enumClass->findOrAddElement(context, tokens[i].unquoted().replaceCharacters(T(" \t"), T("-"))) != i)
    {
      context.errorCallback(T("ARFFDataParser::parseEnumerationAttributeLine"), T("Duplicate enumeration element found: ") + tokens[i].quoted());
      return false;
    }
  attributesType.push_back(enumClass);
  return true;
}

static Variable createFromString(ExecutionContext& context, const TypePtr& type, const String& str)
{
  if (str.length() == 1 && str == T("?"))
    return Variable::missingValue(type);
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
  VectorPtr inputs = variableVector(n);
  for (size_t i = 0; i < n; ++i)
    inputs->setElement(i, createFromString(context, attributesType[i], tokens[i]));
  // get supervision
  Variable supervison = createFromString(context, supervisionType, tokens[n]); // FIXME: trouble with booleanType ??
  setResult(Variable::pair(inputs, supervison, getElementsType()));
  return true;
}

bool ARFFDataParser::parseSparseDataLine(const String& line)
{
  jassertfalse; // FIXME: not yet implemented
  return false;
}
