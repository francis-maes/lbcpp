/*-----------------------------------------.---------------------------------.
| Filename: ObjectStream.cpp               | Object Streams                  |
| Author  : Francis Maes                   |                                 |
| Started : 08/06/2009 14:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/ObjectStream.h>
#include <lbcpp/SparseVector.h>
#include <lbcpp/LearningExample.h>
#include <fstream>
using namespace lbcpp;

/*
** TextObjectParser
*/
TextObjectParser::TextObjectParser(std::istream* newInputStream)
  : istr(newInputStream) {}
    
TextObjectParser::TextObjectParser(const std::string& filename)
  : istr(NULL)
{
  if (filename.empty())
  {
    Object::error("TextFileParser::parseFile", "No filename specified");
    return;
  }
  std::ifstream* istr = new std::ifstream(filename.c_str());
  if (istr->is_open())
    this->istr = istr;
  else
  {
    Object::error("TextFileParser::parseFile", "Could not open file '" + filename + "'");
    delete istr;
  }
}

TextObjectParser::~TextObjectParser()
{
  if (istr)
    delete istr;
}
 
void TextObjectParser::tokenize(const std::string& line, std::vector< std::string >& columns, const char* separators)
{
  size_t b = line.find_first_not_of(separators);
  while (b != std::string::npos)
  {
    size_t e = line.find_first_of(separators, b);
    if (e == std::string::npos)
      columns.push_back(line.substr(b));
    else
      columns.push_back(line.substr(b, e - b));
    b = line.find_first_not_of(separators, e);
  }
}

ObjectPtr TextObjectParser::next()
{
  if (!istr)
    return ObjectPtr();
  if (!currentObject)
  {
    //parsingBreaked = false;
    parseBegin();
  }
  currentObject = ObjectPtr();
  
  while (!istr->eof()/* && !parsingBreaked*/)
  {
    std::string line;
    std::getline(*istr, line);
    size_t n = line.find_last_not_of("\r\n");
    if (n != std::string::npos)
      line = line.substr(0, n + 1);
    if (!parseLine(line))
    {
      Object::error("TextParserObjectStream::parse", "Could not parse line '" + line + "'");
      delete istr;
      istr = NULL;
      return ObjectPtr();
    }
    if (currentObject)
      return currentObject;
  }
  
  if (!parseEnd())
    Object::error("TextFileParser::parse", "Error in parse end");
  delete istr;
  istr = NULL;
  return ObjectPtr();
}

/*
** LearningDataObjectParser
*/
bool LearningDataObjectParser::parseLine(const std::string& line)
{
  size_t begin = line.find_first_not_of(" \t");
  bool isEmpty = begin == std::string::npos;
  if (isEmpty)
    return parseEmptyLine();
  if (line[begin] == '#')
    return parseCommentLine(line.substr(begin + 1));
  std::vector<std::string> columns;
  tokenize(line, columns);
  return parseDataLine(columns);
}

bool LearningDataObjectParser::parseFeatureList(const std::vector<std::string>& columns, size_t firstColumn, SparseVectorPtr& res)
{
  assert(features);
  res = new SparseVector(features);
  for (size_t i = firstColumn; i < columns.size(); ++i)
  {
    std::string identifier;
    double value;
    if (!parseFeature(columns[i], identifier, value))
      return false;
    std::vector<std::string> path;
    if (!parseFeatureIdentifier(identifier, path))
      return false;
    res->set(path, value);
  }
  return true;
}

bool LearningDataObjectParser::parseFeature(const std::string& str, std::string& featureId, double& featureValue)
{
  size_t n = str.find(':');
  if (n == std::string::npos)
  {
    featureId = str;
    featureValue = 1.0;
    return true;
  }
  else
  {
    featureId = str.substr(0, n);
    std::string featureStringValue = str.substr(n + 1);
    return TextObjectParser::parse(featureStringValue, featureValue);
  }
}

bool LearningDataObjectParser::parseFeatureIdentifier(const std::string& identifier, std::vector<std::string>& path)
{
  tokenize(identifier, path, ".");
  return true;
}

/*
** ClassificationExamplesParser
*/
class ClassificationExamplesParser : public LearningDataObjectParser
{
public:
  ClassificationExamplesParser(const std::string& filename, FeatureDictionaryPtr features, StringDictionaryPtr labels)
    : LearningDataObjectParser(filename, features), labels(labels) {}

  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    std::string label;
    if (!TextObjectParser::parse(columns[0], label))
      return false;
    SparseVectorPtr x;
    if (!parseFeatureList(columns, 1, x))
      return false;
    setResult(new ClassificationExample(x, labels->add(label)));
    return true;
  }
  
private:
  StringDictionaryPtr labels;
};

ObjectStreamPtr ObjectStream::createClassificationExamplesParser(
          const std::string& filename, FeatureDictionaryPtr features, StringDictionaryPtr labels)
{
  LearningDataObjectParserPtr res = new ClassificationExamplesParser(filename, features, labels);
  return res->isValid() ? res : LearningDataObjectParserPtr();
}

class RegressionExamplesParser : public LearningDataObjectParser
{
public:
  RegressionExamplesParser(const std::string& filename, FeatureDictionaryPtr features)
    : LearningDataObjectParser(filename, features) {}

  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    double y;
    if (!TextObjectParser::parse(columns[0], y))
      return false;
    SparseVectorPtr x;
    if (!parseFeatureList(columns, 1, x))
      return false;
    setResult(new RegressionExample(x, y));
    return true;
  }
};

ObjectStreamPtr ObjectStream::createRegressionExamplesParser(const std::string& filename, FeatureDictionaryPtr features)
{
  LearningDataObjectParserPtr res = new RegressionExamplesParser(filename, features);
  return res->isValid() ? res : LearningDataObjectParserPtr();
}
