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

/*
** ApplyFunctionObjectStream
*/
class ApplyFunctionObjectStream : public ObjectStream
{
public:
  ApplyFunctionObjectStream(ObjectStreamPtr stream, ObjectFunctionPtr function)
    : stream(stream), function(function) {}
    
  virtual String getContentClassName() const
    {return function->getOutputClassName(stream->getContentClassName());}

  virtual bool rewind()
    {return stream->rewind();}

  virtual bool isExhausted() const
    {return stream->isExhausted();}
    
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
TextObjectParser::TextObjectParser(InputStream* newInputStream)
  : istr(newInputStream) {}
    
TextObjectParser::TextObjectParser(const File& file)
  : istr(NULL)
{
  if (file == File::nonexistent)
  {
    Object::error(T("TextObjectParser::parseFile"), T("No filename specified"));
    return;
  }
  InputStream* inputStream = file.createInputStream();
  if (!inputStream)
  {
    Object::error(T("TextObjectParser::parseFile"), T("Could not open file ") + file.getFullPathName());
    return;
  }

  this->istr = inputStream;
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
    {
      columns.push_back(line.substring(b));
      break;
    }
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
  
  while (!istr->isExhausted()/* && !parsingBreaked*/)
  {
    String line = istr->readNextLine().trim();
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
    return parseCommentLine(line.substring(begin + 1).trim());
  std::vector<String> columns;
  tokenize(line, columns);
  return parseDataLine(columns);
}

bool LearningDataObjectParser::parseFeatureList(const std::vector<String>& columns, size_t firstColumn, SparseVectorPtr& res)
{
  jassert(features);
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
  }
  else
  {
    featureId = str.substring(0, n);
    featureValue = str.substring(n + 1).getDoubleValue();
  }
  return true;
}

bool LearningDataObjectParser::parseFeatureIdentifier(const String& identifier, std::vector<String>& path)
{
  tokenize(identifier, path, T("."));
  return true;
}


ObjectStreamPtr lbcpp::classificationExamplesParser(
          const File& file, FeatureDictionaryPtr features, FeatureDictionaryPtr labels)
{
  jassert(features && labels);
  return new ClassificationExamplesParser(file, features, labels);
}

ObjectStreamPtr lbcpp::regressionExamplesParser(const File& file, FeatureDictionaryPtr features)
{
  jassert(features);
  return new RegressionExamplesParser(file, features);
}

ObjectStreamPtr lbcpp::classificationExamplesSyntheticGenerator(size_t numFeatures, size_t numClasses)
{
  jassert(numClasses >= 2);
  return new ClassificationExamplesSyntheticGenerator(numFeatures, numClasses);
}

/*
** DirectoryObjectStream
*/
DirectoryObjectStream::DirectoryObjectStream(const File& directory, const String& wildCardPattern, bool searchFilesRecursively)
  : nextFilePosition(0)
{
  directory.findChildFiles(files, File::findFiles, searchFilesRecursively, wildCardPattern);
}

bool DirectoryObjectStream::isExhausted() const
  {return nextFilePosition >= files.size();}

bool DirectoryObjectStream::rewind()
{
  nextFilePosition = 0;
  return true;
}

ObjectPtr DirectoryObjectStream::next()
{
  if (isExhausted())
    return ObjectPtr();
  File file = *files[nextFilePosition];
  ++nextFilePosition;
  return parseFile(file);
}

ObjectPtr DirectoryObjectStream::parseFile(const File& file)
  {return Object::loadFromFile(file);}
