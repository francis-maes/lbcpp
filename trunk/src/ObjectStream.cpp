/*-----------------------------------------.---------------------------------.
| Filename: ObjectStream.cpp               | Object Streams                  |
| Author  : Francis Maes                   |                                 |
| Started : 08/06/2009 14:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object/ObjectStream.h>
#include <lbcpp/Object/ObjectContainer.h>
#include <lbcpp/FeatureGenerator/SparseVector.h>
#include <lbcpp/FeatureGenerator/DenseVector.h>

#include "ObjectStream/ApplyFunctionObjectStream.h"
#include "ObjectStream/ClassificationExamplesParser.h"
#include "ObjectStream/RegressionExamplesParser.h"
#include "ObjectStream/ClassificationExamplesSyntheticGenerator.h"
#include "ObjectStream/DirectoriesObjectPairStream.h"

#include <fstream>
using namespace lbcpp;

/*
** ObjectStream
*/
ObjectContainerPtr ObjectStream::load(size_t maximumCount)
{
  VectorObjectContainerPtr res = new VectorObjectContainer(getContentClassName());
  res->setName(getName());
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
** TextObjectParser
*/
TextObjectParser::TextObjectParser(InputStream* newInputStream)
  : ObjectStream(T("stream")), istr(newInputStream) {}
    
TextObjectParser::TextObjectParser(const File& file)
  : ObjectStream(file.getFullPathName()), istr(NULL)
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
    String line = istr->readNextLine();
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
  return currentObject;
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

/*
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
*/

/*
** DirectoryObjectStream
*/
DirectoryObjectStream::DirectoryObjectStream(const File& directory, const String& wildCardPattern, bool searchFilesRecursively)
  : ObjectStream(directory.getFullPathName() + T("/") + wildCardPattern + (searchFilesRecursively ? T(" rec") : T(""))),
    directory(directory), wildCardPattern(wildCardPattern), searchFilesRecursively(searchFilesRecursively)
  {initialize();}

DirectoryObjectStream::DirectoryObjectStream() : nextFilePosition(0) {}

void DirectoryObjectStream::initialize()
{
  directory.findChildFiles(files, File::findFiles, searchFilesRecursively, wildCardPattern);
  nextFilePosition = 0;
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
  while (nextFilePosition < files.size())
  {
    File file = *files[nextFilePosition];
    ++nextFilePosition;
    ObjectPtr res = parseFile(file);
    if (res)
      return res;
  }
  return ObjectPtr();
}

ObjectPtr DirectoryObjectStream::parseFile(const File& file)
  {return Object::createFromFile(file);}

bool DirectoryObjectStream::load(InputStream& istr)
{
  bool ok = NameableObject::load(istr) && lbcpp::read(istr, directory) && 
    lbcpp::read(istr, wildCardPattern) && lbcpp::read(istr, searchFilesRecursively);
  if (!ok)
    return false;
  initialize();
  return true;
}

void DirectoryObjectStream::save(OutputStream& ostr) const
{
  ObjectStream::save(ostr);
  lbcpp::write(ostr, directory);
  lbcpp::write(ostr, wildCardPattern);
  lbcpp::write(ostr, searchFilesRecursively);
}

ObjectStreamPtr lbcpp::directoriesObjectPairStream(const File& directory1, const File& directory2, const String& wildCardPattern)
  {return new DirectoriesObjectPairStream(directory1, directory2, wildCardPattern);}

ObjectStreamPtr ObjectStream::apply(ObjectFunctionPtr function)
  {return new ApplyFunctionObjectStream(this, function);}

void declareObjectStreamClasses()
{
  LBCPP_DECLARE_CLASS(DirectoryObjectStream);
  LBCPP_DECLARE_CLASS(DirectoriesObjectPairStream);
}
