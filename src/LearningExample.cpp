/*-----------------------------------------.---------------------------------.
| Filename: LearningExample.cpp            | Learning examples               |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 16:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/LearningExample.h>
#include <lbcpp/SparseVector.h>
#include <fstream>
using namespace lbcpp;

/*
** LearningExampleParser
*/
bool LearningExamplesParser::parseFeatureList(const std::vector<std::string>& columns, size_t firstColumn, SparseVectorPtr& res)
{
  assert(dictionary);
  res = new SparseVector(dictionary);
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
    return TextFileParser::parse(featureStringValue, featureValue);
  }
}

bool LearningExamplesParser::parseFeatureIdentifier(const std::string& identifier, std::vector<std::string>& path)
{
  tokenize(identifier, path, ".");
  return true;
}

bool LearningExamplesParser::parse(std::istream& istr, FeatureDictionaryPtr dictionary)
{
  this->dictionary = dictionary;
  parseStream(istr);
  this->dictionary = FeatureDictionaryPtr();
  return true;
}

class ClassificationExamplesParser : public LearningExamplesParser
{
public:
  ClassificationExamplesParser(std::vector<ClassificationExample>& target, StringDictionaryPtr labels)
    : target(target), labels(labels) {}

  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    std::string label;
    if (!TextFileParser::parse(columns[0], label))
      return false;
    SparseVectorPtr x;
    if (!parseFeatureList(columns, 1, x))
      return false;
    target.push_back(ClassificationExample(x, labels->add(label)));
    return true;
  }
  
private:
  std::vector<ClassificationExample>& target;
  StringDictionaryPtr labels;
};

bool lbcpp::parseClassificationExamples(std::istream& istr, FeatureDictionaryPtr dictionary, StringDictionaryPtr labels, std::vector<ClassificationExample>& res)
{
  ClassificationExamplesParser parser(res, labels);
  return parser.parse(istr, dictionary);
}

class RegressionExamplesParser : public LearningExamplesParser
{
public:
  RegressionExamplesParser(std::vector<RegressionExample>& target)
    : target(target) {}

  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    double y;
    if (!TextFileParser::parse(columns[0], y))
      return false;
    SparseVectorPtr x;
    if (!parseFeatureList(columns, 1, x))
      return false;
    target.push_back(RegressionExample(x, y));
    return true;
  }

private:
  std::vector<RegressionExample>& target;
};

bool lbcpp::parseRegressionExamples(std::istream& istr, FeatureDictionaryPtr dictionary, std::vector<RegressionExample>& res)
{
  RegressionExamplesParser parser(res);
  return parser.parse(istr, dictionary);
}
