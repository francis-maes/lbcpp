/*-----------------------------------------.---------------------------------.
| Filename: Stream.cpp                     | Variable Streams                |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Stream.h>
#include <lbcpp/Data/Vector.h>
using namespace lbcpp;

/*
** Stream
*/
VectorPtr Stream::load(size_t maximumCount)
{
  VectorPtr res = new Vector(getElementsType());
  while (maximumCount == 0 || res->size() < maximumCount)
  {
    Variable variable = next();
    if (variable)
      res->append(variable);
    else
      break;
  }
  return res;
}

bool Stream::iterate(size_t maximumCount)
{
  size_t count = 0;
  while (maximumCount == 0 || count < maximumCount)
  {
    Variable variable = next();
    if (variable)
      ++count;
    else
      break;
  }
  return true;
}

/*
 ** directoryFileStream
 */
#include "Stream/DirectoryFileStream.h"
#include "Stream/ApplyFunctionStream.h"

StreamPtr lbcpp::directoryFileStream(const File& directory, const String& wildCardPattern, bool searchFilesRecursively)
  {return new DirectoryFileStream(directory, wildCardPattern, searchFilesRecursively);}

StreamPtr Stream::apply(FunctionPtr function) const
{
  return checkInheritance(getElementsType(), function->getInputType())
    ? StreamPtr(new ApplyFunctionStream(StreamPtr(const_cast<Stream* >(this)), function))
    : StreamPtr();
}

/*
 ** TextParser
 */
TextParser::TextParser(InputStream* newInputStream)
: istr(newInputStream) {}

TextParser::TextParser(const File& file)
: istr(NULL)
{
  if (file == File::nonexistent)
  {
    Object::error(T("TextParser::parseFile"), T("No filename specified"));
    return;
  }
  InputStream* inputStream = file.createInputStream();
  if (!inputStream)
  {
    Object::error(T("TextParser::parseFile"), T("Could not open file ") + file.getFullPathName());
    return;
  }
  
  this->istr = inputStream;
}

TextParser::~TextParser()
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

void TextParser::tokenize(const String& line, std::vector<String>& columns, const juce::tchar* separators)
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

Variable TextParser::next()
{
  if (!istr)
    return Variable();
  if (!currentResult)
  {
    //parsingBreaked = false;
    parseBegin();
  }
  currentResult = Variable();
  
  while (!istr->isExhausted()/* && !parsingBreaked*/)
  {
    String line = istr->readNextLine();
    if (!parseLine(line))
    {
      Object::error(T("TextParser::parse"), T("Could not parse line '") + line + T("'"));
      delete istr;
      istr = NULL;
      return Variable();
    }
    if (currentResult)
      return currentResult;
  }
  
  if (!parseEnd())
    Object::error(T("TextParser::next"), T("Error in parse end"));
  delete istr;
  istr = NULL;
  return currentResult;
}

void declareStreamClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Stream, Object);
    LBCPP_DECLARE_CLASS(DirectoryFileStream, Stream);
    LBCPP_DECLARE_ABSTRACT_CLASS(TextParser, Stream);
}
