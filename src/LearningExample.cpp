/*-----------------------------------------.---------------------------------.
| Filename: LearningExample.cpp            | Learning examples               |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 16:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/LearningExample.h>
#include <cralgo/SparseVector.h>
using namespace cralgo;

bool LearningExamplesParser::parseLine(const std::string& line)
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

bool LearningExamplesParser::parse(std::istream& istr, FeatureDictionary& dictionary)
{
  this->dictionary = &dictionary;
  parseBegin();
  while (!istr.eof())
  {
    std::string line;
    std::getline(istr, line);
    if (!parseLine(line))
    {
      ErrorHandler::error("LearningExamplesParser::parse", "Could not parse line '" + line + "'");
      return false;
    }
  }
  if (!parseEnd())
  {
    ErrorHandler::error("LearningExamplesParser::parse", "Error in parse end");
      return false;
  }
  this->dictionary = NULL;
  return true;
}

void LearningExamplesParser::tokenize(const std::string& line, std::vector< std::string >& columns, const char* separators)
{
  //std::cout << "Tokenize " << cralgo::toString(line) << " => ";
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
  //std::cout << cralgo::toString(columns) << std::endl;
}

bool LearningExamplesParser::parseFeatureList(const std::vector<std::string>& columns, size_t firstColumn, SparseVectorPtr& res)
{
  assert(dictionary);
  res = new SparseVector(*dictionary);
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

bool LearningExamplesParser::parseFeature(const std::string& str, std::string& featureId, double& featureValue)
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
    return parse(featureStringValue, featureValue);
  }
}

bool LearningExamplesParser::parseFeatureIdentifier(const std::string& identifier, std::vector<std::string>& path)
{
  tokenize(identifier, path, ".");
  return true;
}

class ClassificationExamplesParser : public LearningExamplesParser
{
public:
  ClassificationExamplesParser(std::vector<ClassificationExample>& target, StringDictionary& labels)
    : target(target), labels(labels) {}

  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    std::string label;
    if (!parse(columns[0], label))
      return false;
    SparseVectorPtr x;
    if (!parseFeatureList(columns, 1, x))
      return false;
    target.push_back(ClassificationExample(x, labels.add(label)));
    return true;
  }
  
private:
  std::vector<ClassificationExample>& target;
  StringDictionary& labels;
};

bool cralgo::parseClassificationExamples(std::istream& istr, FeatureDictionary& dictionary, StringDictionary& labels, std::vector<ClassificationExample>& res)
{
  ClassificationExamplesParser parser(res, labels);
  return parser.parse(istr, dictionary);
}

class RegressionExamplesParser : public LearningExamplesParser
{
public:
  RegressionExamplesParser(std::vector<RegressionExample>& target)
    : target(&target) {}

  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    double y;
    if (!parse(columns[0], y))
      return false;
    SparseVectorPtr x;
    if (!parseFeatureList(columns, 1, x))
      return false;
    target->push_back(RegressionExample(x, y));
    return true;
  }

private:
  std::vector<RegressionExample>* target;
};

bool cralgo::parseRegressionExamples(std::istream& istr, FeatureDictionary& dictionary, std::vector<RegressionExample>& res)
{
  RegressionExamplesParser parser(res);
  return parser.parse(istr, dictionary);
}
