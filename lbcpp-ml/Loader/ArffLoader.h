/*-----------------------------------------.---------------------------------.
| Filename: ArffLoader.h                   | ARFF Loader                     |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 19:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_LOADER_ARFF_H_
# define LBCPP_ML_LOADER_ARFF_H_

# include <lbcpp/Core/Loader.h>
# include <lbcpp/Data/Table.h>
# include <lbcpp-ml/Expression.h>

namespace lbcpp
{
 
# ifdef JUCE_WIN32
#  pragma warning(disable:4996) // microsoft visual does not like fopen()/fclose()
# endif // JUCE_WIN32

class ArffLoader : public TextLoader
{
public:
  virtual string getFileExtensions() const
    {return "arff";}

  virtual ClassPtr getTargetClass() const
    {return tableClass;}

  bool shouldReadData;

  virtual void parseBegin(ExecutionContext& context)
  {
    table = new Table();
    shouldReadData = false;
  }

  virtual bool parseLine(ExecutionContext& context, const string& fullLine)
  {
    string line = fullLine.trim();
    if (line.startsWith(T("%")))                              // Skip comment line
      return true;
    if (line == string::empty)
      return true;                // an empty line is considered as a comment line
    if (shouldReadData)                                   // line *must* be a data
    {
      if (line.startsWithChar(T('{')))
        return parseSparseDataLine(context, line);
      return parseDataLine(context, line);
    }
    string record = line.substring(0, juce::jmin(10, line.length())).toLowerCase();
    if (record.startsWith(T("@attribute")))
      return parseAttributeLine(context, line);
    if (record.startsWith(T("@relation")))  // @relation *must* be declared before
    {                                       // any @attribute declaration
      if (table->getNumColumns() > 0)
      {
        context.warningCallback(T("ARFFDataParser::parseLine"), T("@relation must be declared before any @attribute declaration"));
        return false;
      }
      return true;                                     // skip the relation's name
    }
    if (record.startsWith(T("@data")))
    {
      if (!table || table->getNumColumns() < 2)
      {
        context.warningCallback(T("ARFFDataParser::parseLine"), T("Not enough attribute descriptor"));
        return false;
      }
      shouldReadData = true;     // Indicate that the remaining lines will be data
      return true;
    }
    context.warningCallback(T("ARFFDataParser::parseLine"), T("Unknown expression: ") + line.quoted());
    return false;
  }

  virtual ObjectPtr parseEnd(ExecutionContext& context)
    {return table;}

protected:
  TablePtr table;

  bool parseAttributeLine(ExecutionContext& context, const string& line)
  {
    if (line.getLastCharacter() == T('}'))
      return parseEnumerationAttributeLine(context, line);

  /*  int b = line.lastIndexOfAnyOf(T(" \t"));
    if (b < 0)
    {
      context.errorCallback(T("ARFFDataParser::parseAttributeLine"), T("Malformatted attribute description: ") + line.quoted());
      return false;
    }*/

    StringArray tokens;
    tokens.addTokens(line.trim(), T(" \t"), T("'"));
    if (tokens.size() < 3)
    {
      context.errorCallback(T("ARFFDataParser::parseAttributeLine"), T("Malformatted attribute name: ") + line.quoted());
      return false;
    }
    int tokenNumber = 1;
    while (tokenNumber < tokens.size() && tokens[tokenNumber].trim().isEmpty())
      ++tokenNumber;
    string attributeName = tokens[tokenNumber].unquoted();
    ++tokenNumber;
    while (tokenNumber < tokens.size() && tokens[tokenNumber].trim().isEmpty())
      ++tokenNumber;

    string attributeTypeName = tokens[tokenNumber].toLowerCase();
    ClassPtr attributeType;
    if (attributeTypeName == T("float")
        || attributeTypeName == T("real")
        || attributeTypeName == T("numeric"))
      attributeType = doubleClass;
    else if (attributeTypeName == T("integer"))
      attributeType = integerClass;
    else if (attributeTypeName == T("string"))
    {
      attributeType = stringClass;
      context.warningCallback(T("ARFFDataParser::parseAttributeLine"), T("string format is not (yet) supported: ") + line.quoted());
    }
    else
    {
      context.errorCallback(T("ARFFDataParser::parseAttributeLine"), T("Unknown attribute type: ") + attributeTypeName.quoted());
      return false;
    }
    table->addColumn(new VariableExpression(attributeType, attributeName, table->getNumColumns()), attributeType);
    return true;
  }

  static inline bool comparePairOfStrings(const string& a, const string& b, const string& x, const string& y)
    {return (a == x && b == y) || (a == y && b == x);}

  static bool shouldBeABooleanType(const StringArray& tokens)
  {
    if (tokens.size() != 2)
      return false;
    string first = tokens[0].toLowerCase();
    string second = tokens[1].toLowerCase();
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

  bool parseEnumerationAttributeLine(ExecutionContext& context, const string& line)
  {
    // get enumeration name
    string trimmedLine = line.substring(10).trim();
    int e = trimmedLine.indexOfAnyOf(T(" \t"));
    if (e < 0)
    {
      context.errorCallback(T("ARFFDataParser::parseEnumerationAttributeLine"), T("Malformatted enumeration description: ") + trimmedLine.quoted());
      return false;
    }
    string attributeName = trimmedLine.substring(0, e).unquoted();
    trimmedLine = trimmedLine.substring(e);
    // get enumeration values
    int b = trimmedLine.lastIndexOfChar(T('{'));
    if (b < 0)
    {
      context.errorCallback(T("ARFFDataParser::parseEnumerationAttributeLine"), T("Malformatted enumeration description: ") + trimmedLine.quoted());
      return false;
    }

    string values = trimmedLine.substring(b + 1, trimmedLine.length() - 1);
    StringArray tokens;
    tokens.addTokens(values, T(","), T("'\""));
    if (tokens.size() == 0)
    {
      context.errorCallback(T("ARFFDataParser::parseEnumerationAttributeLine"), T("No enumeration element found in: ") + trimmedLine.quoted());
      return false;
    }
    for (size_t i = 0; i < (size_t)tokens.size(); ++i)
      tokens.set(i, tokens[i].trim().unquoted());
    
    ClassPtr attributeType;
    // It can be a booleanClass ?
    if (shouldBeABooleanType(tokens))
    {
      attributeType = booleanClass;
    }
    else
    {
      // create enumeration
      DefaultEnumerationPtr enumClass = new DefaultEnumeration(attributeName);
      for (size_t i = 0; i < (size_t)tokens.size(); ++i)
        if (enumClass->findOrAddElement(context, tokens[i].unquoted().trim()) != i)
        {
          context.errorCallback(T("ARFFDataParser::parseEnumerationAttributeLine"), T("Duplicate enumeration element found: ") + tokens[i].quoted());
          return false;
        }
      attributeType = enumClass;
    }
    table->addColumn(attributeName, attributeType);
    return true;
  }
  
  bool parseDataLine(ExecutionContext& context, const string& line)
  {
    bool ok = true;
  
    size_t n = table->getNumColumns();
  
    size_t lineLength = line.length();
    char* str = new char[lineLength + 1];
    memcpy(str, (const char* )line, lineLength + 1);

    std::vector<ObjectPtr> row(n);
    for (size_t i = 0; i < n; ++i)
    {
      char* token = strtok(i == 0 ? str : NULL, ",\t");
      if (!token)
      {
        context.errorCallback(T("ARFFDataParser::parseDataLine"), T("Invalid number of values in: ") + line.quoted());
        ok = false;
        break;
      }
      if (strcmp(token, "?"))
        row[i] = Object::createFromString(context, table->getType(i), string(token).unquoted());
    }
    table->addRow(row);
    delete [] str;
    return true;
  }

  bool parseSparseDataLine(ExecutionContext& context, const string& line)
  {
    size_t n = table->getNumColumns();
    StringArray tokens;
    tokens.addTokens(line.substring(1, line.length() - 1), T(", "), T("'\""));
    size_t numTokens = tokens.size();
    if (numTokens > n + 1)
    {
      context.errorCallback(T("ARFFDataParser::parseSparseDataLine"), T("Too many values in: ") + line.quoted());
      return false;
    }
  
    // get attributes
    // broken
    jassertfalse;
  #if 0
    ObjectPtr supervision;
    DenseGenericObjectPtr inputs = new DenseGenericObject(features);
    for (size_t i = 0; i < numTokens; ++i)
    {
      int e = tokens[i].indexOfAnyOf(T(" \t"));
      if (e < 0)
      {
        context.errorCallback(T("ARFFDataParser::parseSparseDataLine"), T("Bad index in: ") + tokens[i].quoted());
        return false;
      }
      ObjectPtr v = Object::createFromString(context, positiveIntegerClass, tokens[i].substring(0, e));
      if (!v)
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
          context.errorCallback(T("ARFFDataParser::parseSparseDataLine"), T("Duplicate index '" + string((int)index) + "' in: ") + tokens[i].quoted());
          return false;
        }
        inputs->setVariable(index, createFromString(context, attributesType[index], tokens[i].substring(e).trim()));
      }
    }
    setResult(finalizeData(new Pair(getElementsType(), inputs, supervision)));
  #endif // 0
    return true;
  }
};

}; /* namespace lbcpp */

#endif // LBCPP_ML_LOADER_ARFF_H_
