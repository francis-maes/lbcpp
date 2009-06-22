/*-----------------------------------------.---------------------------------.
| Filename: ObjectStream.cpp               | Object Streams                  |
| Author  : Francis Maes                   |                                 |
| Started : 08/06/2009 14:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/ObjectStream.h>
#include <lbcpp/ObjectContainer.h>
#include <lbcpp/SparseVector.h>
#include <lbcpp/DenseVector.h>
#include <lbcpp/LearningExample.h>

#include "ObjectStream/ClassificationExamplesParser.h"
#include "ObjectStream/RegressionExamplesParser.h"
#include "ObjectStream/ClassificationExamplesSyntheticGenerator.h"

#include <fstream>
using namespace lbcpp;

/*
** ObjectStream
*/
ObjectContainerPtr ObjectStream::load(size_t maximumCount)
{
  VectorObjectContainerPtr res = new VectorObjectContainer(getContentClassName());
  while (maximumCount == 0 || res->size() < maximumCount)
  {
    ObjectPtr object = next();
    if (object)
      res->append(object);
    else
      break;
  }
  return res;
}

bool ObjectStream::iterate(size_t maximumCount)
{
  size_t count = 0;
  while (maximumCount == 0 || count < maximumCount)
  {
    ObjectPtr object = next();
    if (object)
      ++count;
    else
      break;
  }
  return true;
}

bool ObjectStream::checkContentClassName(const std::string& expectedClassName)
{
  if (getContentClassName() != expectedClassName)
  {
    Object::error("ObjectStream::checkContentClassName",
        "Expected objects of class '" + expectedClassName + "', found objects of class '" + getContentClassName() + "'");
    return false;
  }
  return true;
}

class ApplyFunctionObjectStream : public ObjectStream
{
public:
  ApplyFunctionObjectStream(ObjectStreamPtr stream, ObjectFunctionPtr function)
    : stream(stream), function(function) {}
    
  virtual std::string getContentClassName() const
    {return function->getOutputClassName();}

  virtual bool isValid() const
    {return stream->isValid();}
    
  virtual ObjectPtr next()
  {
    ObjectPtr object = stream->next();
    return object ? function->function(object) : ObjectPtr();
  }

private:
  ObjectStreamPtr stream;
  ObjectFunctionPtr function;
};

ObjectStreamPtr ObjectStream::apply(ObjectFunctionPtr function)
{
  return new ApplyFunctionObjectStream(this, function);
}

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
    Object::error("TextObjectParser::parseFile", "No filename specified");
    return;
  }
  std::ifstream* istr = new std::ifstream(filename.c_str());
  if (istr->is_open())
    this->istr = istr;
  else
  {
    Object::error("TextObjectParser::parseFile", "Could not open file '" + filename + "'");
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
    Object::error("TextObjectParser::next", "Error in parse end");
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


ObjectStreamPtr lbcpp::classificationExamplesParser(
          const std::string& filename, FeatureDictionaryPtr features, StringDictionaryPtr labels)
{
  assert(features && labels);
  return new ClassificationExamplesParser(filename, features, labels);
}

ObjectStreamPtr lbcpp::regressionExamplesParser(const std::string& filename, FeatureDictionaryPtr features)
{
  assert(features);
  return new RegressionExamplesParser(filename, features);
}

ObjectStreamPtr lbcpp::classificationExamplesSyntheticGenerator(size_t numFeatures, size_t numClasses)
{
  assert(numClasses >= 2);
  return new ClassificationExamplesSyntheticGenerator(numFeatures, numClasses);
}
