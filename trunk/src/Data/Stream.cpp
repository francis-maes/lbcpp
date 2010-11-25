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
VectorPtr Stream::load(ExecutionContext& context, size_t maximumCount)
{
  VectorPtr res = vector(getElementsType());
  while (maximumCount == 0 || res->getNumElements() < maximumCount)
  {
    Variable variable = next(context);
    if (!variable.isNil())
      res->append(variable);
    if (isExhausted())
      break;
  }
  return res;
}

bool Stream::iterate(ExecutionContext& context, size_t maximumCount)
{
  size_t count = 0;
  while (maximumCount == 0 || count < maximumCount)
  {
    Variable variable = next(context);
    if (variable.exists())
      ++count;
    else
      break;
  }
  return true;
}

namespace lbcpp
{
  extern StreamPtr applyFunctionStream(StreamPtr stream, FunctionPtr function);
};

StreamPtr Stream::apply(ExecutionContext& context, FunctionPtr function) const
{
  return context.checkInheritance(getElementsType(), function->getInputType())
    ? applyFunctionStream(refCountedPointerFromThis(this), function)
    : StreamPtr();
}

/*
** TextParser
*/
TextParser::TextParser(InputStream* newInputStream)
  : istr(newInputStream) {}

TextParser::TextParser(ExecutionContext& context, const File& file)
  : istr(NULL)
{
  if (file == File::nonexistent)
  {
    context.errorCallback(T("TextParser::parseFile"), T("No filename specified"));
    return;
  }
  InputStream* inputStream = file.createInputStream();
  if (!inputStream)
  {
    context.errorCallback(T("TextParser::parseFile"), T("Could not open file ") + file.getFullPathName());
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

Variable TextParser::next(ExecutionContext& context)
{
  if (!istr)
    return Variable();
  if (!currentResult.exists())
  {
    //parsingBreaked = false;
    parseBegin(context);
  }
  currentResult = Variable();
  
  while (!istr->isExhausted()/* && !parsingBreaked*/)
  {
    String line = istr->readNextLine();
    if (!parseLine(context, line))
    {
      context.errorCallback(T("TextParser::parse"), T("Could not parse line '") + line + T("'"));
      delete istr;
      istr = NULL;
      return Variable();
    }
    if (currentResult.exists())
      return currentResult;
  }
  
  if (!parseEnd(context))
    context.errorCallback(T("TextParser::next"), T("Error in parse end"));
  delete istr;
  istr = NULL;
  return currentResult;
}
