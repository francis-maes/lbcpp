/*-----------------------------------------.---------------------------------.
| Filename: LibSVMDataParser.h             | LibSVMDataParser                |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_VARIABLE_STREAM_LIBSVM_DATA_PARSER_H_
# define LBCPP_VARIABLE_STREAM_LIBSVM_DATA_PARSER_H_

# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

/**
 ** @class LibSVMDataParser
 ** @brief Base class for data parsers using libSVM inspired formats.
 **
 ** @see classificationExamplesParser, regressionExamplesParser
 **
 */
class LibSVMDataParser : public TextParser
{
public:
  LibSVMDataParser(ExecutionContext& context, const File& file)
    : TextParser(context, file) {}
  
  /**
   ** Constructor
   **
   ** @param newInputStream : new input stream. This object is
   ** responsible for deleting the input stream, when no longer used.
   ** @param features : feature dictionary.
   **
   ** @return a @a LearningDataObjectParser.
   */
  LibSVMDataParser(ExecutionContext& context, InputStream* newInputStream)
    : TextParser(context, newInputStream) {}
  
  LibSVMDataParser() {}
  /**
   ** This function is called to parse an empty line.
   **
   ** @return False if parsing failed. In this case
   **  inherited class are responsible for throwing an error
   **  to the ErrorManager.
   */
  virtual bool parseEmptyLine()
    {return true;}
  
  /**
   ** This function is called to parse a line containing data.
   **
   ** @param columns : tokenized data line.
   **
   ** @return False if parsing failed. In this case
   **  inherited class are responsible for throwing an error
   **  to the ErrorManager.
   */
  virtual bool parseDataLine(const std::vector<String>& columns)
    {return false;}
  
  /**
   ** This function is called to parse a comment line (starting by '#').
   **
   ** @param comment : the comment (the line without '#')
   **
   ** @return False if parsing failed. In this case
   **  inherited class are responsible for throwing an error
   **  to the ErrorManager.
   */
  virtual bool parseCommentLine(const String& comment)
    {return true;}
  
  /**
   ** This function is called when parsing finishes.
   **
   ** By default, end-of-file is considered as an empty line.
   **
   ** @return False if parsing failed. In this case
   **  inherited class are responsible for throwing an error
   **  to the ErrorManager.
   */
  virtual bool parseEnd()
    {return parseEmptyLine();}
  
  /**
   ** Parses one line.
   **
   ** The following rules are applied:
   ** - if the line is empty, parseLine() calls parseEmptyLine()
   ** - if the line starts by '#', it calls parseCommentLine()
   ** - otherwise, it tokenizes the line and calls parseDataLine()
   **
   ** @param line : text line.
   **
   ** @return a boolean.
   ** @see LearningDataObjectParser::parseEmptyLine
   ** @see LearningDataObjectParser::parseCommentLine
   ** @see LearningDataObjectParser::parseDataLine
   */
  virtual bool parseLine(const String& line);
  
protected:
  /**
   ** Parses a list of features.
   **
   ** Feature lists have the following grammar:
   ** \verbatim featureList ::= feature featureList | feature \endverbatim
   **
   ** @param columns : the columns to parse.
   ** @param firstColumn : start column number.
   */
  SparseDoubleVectorPtr parseFeatureList(DefaultEnumerationPtr features, const std::vector<String>& columns, size_t firstIndex) const;
  
  
  /**
   ** Parses a feature
   **
   ** Features have the following grammar:
   ** \verbatim feature ::= featureId : featureValue | featureId \endverbatim
   **
   ** @param str : the string to parse.
   ** @param featureId : feature ID container.
   ** @param featureValue : feature value container.
   **
   ** @return False if any error occurs.
   */
  static bool parseFeature(const String& str, String& featureId, double& featureValue);
  
  /**
   ** Parses a feature identifier
   **
   ** Feature identifiers have the following grammar:
   ** \verbatim featureId ::= name . featureId  | name \endverbatim
   **
   ** @param identifier : the string to parse.
   ** @param path : feature identifier path container.
   **
   ** @return True.
   */
  static bool parseFeatureIdentifier(const String& identifier, std::vector<String>& path)
    {tokenize(identifier, path, T(".")); return true;}
};

class RegressionLibSVMDataParser : public LibSVMDataParser
{
public:
  RegressionLibSVMDataParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features)
    : LibSVMDataParser(context, file), features(features)
    {elementsType = pairClass(sparseDoubleVectorClass(features), doubleType);}

  RegressionLibSVMDataParser() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    SparseDoubleVectorPtr featuresVector = parseFeatureList(features, columns, 1);
    if (!featuresVector)
      return false;
    setResult(new Pair(elementsType, featuresVector, columns[0].getDoubleValue()));
    return true;
  }

private:
  DefaultEnumerationPtr features;
  TypePtr elementsType;
};

class BinaryClassificationLibSVMDataParser : public LibSVMDataParser
{
public:
  BinaryClassificationLibSVMDataParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features)
    : LibSVMDataParser(context, file), features(features)
    {elementsType = pairClass(sparseDoubleVectorClass(features), booleanType);}

  BinaryClassificationLibSVMDataParser() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    String label = columns[0];
    jassert(label.isNotEmpty());
    juce::tchar lowerCase = juce::CharacterFunctions::toLowerCase(label[0]);
    bool supervision = (lowerCase == 'y' || lowerCase == 't' || lowerCase == '+' || lowerCase == '1');
    SparseDoubleVectorPtr featuresVector = parseFeatureList(features, columns, 1);
    if (!featuresVector)
      return false;
    setResult(new Pair(elementsType, featuresVector, supervision));
    return true;
  }

private:
  DefaultEnumerationPtr features;
  TypePtr elementsType;
};

class ClassificationLibSVMDataParser : public LibSVMDataParser
{
public:
  ClassificationLibSVMDataParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features, DefaultEnumerationPtr labels)
    : LibSVMDataParser(context, file), features(features), labels(labels)
    {elementsType = pairClass(sparseDoubleVectorClass(features), labels);}

  ClassificationLibSVMDataParser() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    size_t label = labels->findOrAddElement(context, columns[0]);
    SparseDoubleVectorPtr featuresVector = parseFeatureList(features, columns, 1);
    if (!featuresVector)
      return false;
    setResult(new Pair(elementsType, featuresVector, Variable(label, labels)));
    return true;
  }

private:
  DefaultEnumerationPtr features;
  DefaultEnumerationPtr labels;
  TypePtr elementsType;
};

class MultiLabelClassificationLibSVMDataParser : public LibSVMDataParser
{
public:
  MultiLabelClassificationLibSVMDataParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features, DefaultEnumerationPtr labels)
    : LibSVMDataParser(context, file), features(features), labels(labels)
    {elementsType = pairClass(sparseDoubleVectorClass(features), sparseDoubleVectorClass(labels, probabilityType));}

  MultiLabelClassificationLibSVMDataParser() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  SparseDoubleVectorPtr parseLabelsList(const String& text) const
  {
    StringArray tokens;
    tokens.addTokens(text, T(","), NULL);
    SparseDoubleVectorPtr res = new SparseDoubleVector(labels, probabilityType);
    for (int i = 0; i < tokens.size(); ++i)
    {
      size_t label = labels->findOrAddElement(context, tokens[i]);
      res->setElement(label, 1.0);
    }
    return res;
  }

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    SparseDoubleVectorPtr labelsVector = parseLabelsList(columns[0]);
    SparseDoubleVectorPtr featuresVector = parseFeatureList(features, columns, 1);
    if (!labelsVector || !featuresVector)
      return false;
    setResult(new Pair(elementsType, featuresVector, labelsVector));
    return true;
  }

private:
  DefaultEnumerationPtr features;
  DefaultEnumerationPtr labels;
  TypePtr elementsType;
};

};

#endif // !LBCPP_VARIABLE_STREAM_LIBSVM_DATA_PARSER_H_
