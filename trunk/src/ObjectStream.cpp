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

bool ObjectStream::checkContentClassName(const String& expectedClassName) const
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
    
  virtual String getContentClassName() const
    {return function->getOutputClassName(stream->getContentClassName());}

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
    
TextObjectParser::TextObjectParser(const File& file)
  : istr(NULL)
{
  if (file == File::nonexistent)
  {
    Object::error(T("TextObjectParser::parseFile"), T("No filename specified"));
    return;
  }
  std::ifstream* istr = new std::ifstream((const char* )file.getFullPathName());
  if (istr->is_open())
    this->istr = istr;
  else
  {
    Object::error(T("TextObjectParser::parseFile"), T("Could not open file ") + file.getFullPathName());
    delete istr;
  }
}

TextObjectParser::~TextObjectParser()
{
  if (istr)
    delete istr;
}
 
inline int indexOfAnyNotOf(const String& str, const String& characters, int startPosition = 0)
{
  for (int i = startPosition; i < str.length(); ++i)
    if (characters.indexOfChar(str[i]) < 0)
      return i;
  return -1;
}
 
void TextObjectParser::tokenize(const String& line, std::vector<String>& columns, const juce::tchar* separators)
{
  int b = indexOfAnyNotOf(line, separators);
  while (b >= 0)
  {
    int e = line.indexOfAnyOf(separators, b);
    if (e < 0)
      columns.push_back(line.substring(b));
    else
      columns.push_back(line.substring(b, e));
    b = indexOfAnyNotOf(line, separators, e);
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
    std::string l;
    std::getline(*istr, l);
    size_t n = l.find_last_not_of("\r\n");
    if (n != std::string::npos)
      l = l.substr(0, n + 1);
    String line = l.c_str();
    if (!parseLine(line))
    {
      Object::error(T("TextParserObjectStream::parse"), T("Could not parse line '") + line + T("'"));
      delete istr;
      istr = NULL;
      return ObjectPtr();
    }
    if (currentObject)
      return currentObject;
  }
  
  if (!parseEnd())
    Object::error(T("TextObjectParser::next"), T("Error in parse end"));
  delete istr;
  istr = NULL;
  return ObjectPtr();
}

/*
** LearningDataObjectParser
*/
bool LearningDataObjectParser::parseLine(const String& line)
{
  int begin = indexOfAnyNotOf(line, T(" \t"));
  bool isEmpty = begin < 0;
  if (isEmpty)
    return parseEmptyLine();
  if (line[begin] == '#')
    return parseCommentLine(line.substring(begin + 1));
  std::vector<String> columns;
  tokenize(line, columns);
  return parseDataLine(columns);
}

bool LearningDataObjectParser::parseFeatureList(const std::vector<String>& columns, size_t firstColumn, SparseVectorPtr& res)
{
  assert(features);
  res = new SparseVector(features);
  for (size_t i = firstColumn; i < columns.size(); ++i)
  {
    String identifier;
    double value;
    if (!parseFeature(columns[i], identifier, value))
      return false;
    std::vector<String> path;
    if (!parseFeatureIdentifier(identifier, path))
      return false;
    res->set(path, value);
  }
  return true;
}

bool LearningDataObjectParser::parseFeature(const String& str, String& featureId, double& featureValue)
{
  int n = str.indexOfChar(':');
  if (n < 0)
  {
    featureId = str;
    featureValue = 1.0;
    return true;
  }
  else
  {
    featureId = str.substring(0, n);
    String featureStringValue = str.substring(n + 1);
    return TextObjectParser::parse(featureStringValue, featureValue);
  }
}

bool LearningDataObjectParser::parseFeatureIdentifier(const String& identifier, std::vector<String>& path)
{
  tokenize(identifier, path, T("."));
  return true;
}


ObjectStreamPtr lbcpp::classificationExamplesParser(
          const File& file, FeatureDictionaryPtr features, StringDictionaryPtr labels)
{
  assert(features && labels);
  return new ClassificationExamplesParser(file, features, labels);
}

ObjectStreamPtr lbcpp::regressionExamplesParser(const File& file, FeatureDictionaryPtr features)
{
  assert(features);
  return new RegressionExamplesParser(file, features);
}

ObjectStreamPtr lbcpp::classificationExamplesSyntheticGenerator(size_t numFeatures, size_t numClasses)
{
  assert(numClasses >= 2);
  return new ClassificationExamplesSyntheticGenerator(numFeatures, numClasses);
}
