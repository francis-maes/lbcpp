/*-----------------------------------------.---------------------------------.
| Filename: Stream.cpp                     | Variable Streams                |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Stream.h>
#include <lbcpp/Core/Vector.h>
#include <lbcpp/Core/DynamicObject.h>

using namespace lbcpp;

/*
** Stream
*/
VectorPtr Stream::load(size_t maximumCount, bool doProgression)
{
  VectorPtr res = vector(getElementsType());
  juce::uint32 lastTimeProgressionWasSent = 0;
  while (maximumCount == 0 || res->getNumElements() < maximumCount)
  {
    if (doProgression)
    {
      juce::uint32 t = Time::getApproximateMillisecondCounter();
      if (!lastTimeProgressionWasSent || (t - lastTimeProgressionWasSent) > 100)
      {
        context.progressCallback(getCurrentPosition());
        lastTimeProgressionWasSent = t;
      }
    }

    Variable variable = next();
    if (!variable.isNil())
      res->append(variable);

    if (isExhausted())
    {
      if (doProgression)
        context.progressCallback(getCurrentPosition());
      break;
    }
  }
  return res;
}

bool Stream::iterate(size_t maximumCount)
{
  size_t count = 0;
  while (maximumCount == 0 || count < maximumCount)
  {
    Variable variable = next();
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

StreamPtr Stream::apply(FunctionPtr function) const
{
  return context.checkInheritance(getElementsType(), function->getInputType())
    ? applyFunctionStream(refCountedPointerFromThis(this), function)
    : StreamPtr();
}

/*
** TextParser
*/
TextParser::TextParser(ExecutionContext& context, InputStream* newInputStream)
  : Stream(context), istr(NULL) {initialize(newInputStream);}

TextParser::TextParser(ExecutionContext& context, const File& file)
  : Stream(context), istr(NULL)
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
  
  initialize(inputStream);
}

void TextParser::initialize(InputStream* istr)
{
  this->istr = istr;
  progression = new ProgressionState(0, (size_t)(istr->getTotalLength() / 1024), T("kb"));
}

TextParser::~TextParser()
{
  if (istr)
    delete istr;
}

ProgressionStatePtr TextParser::getCurrentPosition() const
{
  const_cast<TextParser* >(this)->progression->setValue(istr ? (double)(istr->getPosition() / 1024) : progression->getTotal());
  return progression;
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
  if (!currentResult.exists())
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
      context.errorCallback(T("TextParser::parse"), T("Could not parse line '") + line + T("'"));
      delete istr;
      istr = NULL;
      return Variable();
    }
    if (currentResult.exists())
      return currentResult;
  }
  
  if (!parseEnd())
    context.errorCallback(T("TextParser::next"), T("Error in parse end"));
  delete istr;
  istr = NULL;
  return currentResult;
}
