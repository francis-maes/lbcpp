/*-----------------------------------------.---------------------------------.
| Filename: Stream.cpp                     | Variable Streams                |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Data/Stream.h>
#include <lbcpp/Core/Vector.h>
using namespace lbcpp;

# ifdef JUCE_WIN32
#  pragma warning(disable:4996)
# endif // JUCE_WIN32

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
        ProgressionStatePtr progression = getCurrentPosition();
        if (progression)
          context.progressCallback(progression);
        lastTimeProgressionWasSent = t;
      }
    }

    Variable variable = next();
    if (!variable.isNil())
      res->append(variable);

    if (isExhausted())
    {
      if (doProgression)
      {
        ProgressionStatePtr progression = getCurrentPosition();
        if (progression)
          context.progressCallback(progression);
      }
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

/*
** TextParser
*/
TextParser::TextParser(ExecutionContext& context, const File& file)
  : Stream(context), f(NULL), maxLineLength(1024), lineNumber(0)
{
  if (file == File::nonexistent)
  {
    context.errorCallback(T("TextParser::parseFile"), T("No filename specified"));
    return;
  }
  line = (char* )malloc(sizeof (char) * maxLineLength);
  f = fopen(file.getFullPathName(), "r");
  if (!f)
    context.errorCallback(T("Could not open file ") + file.getFullPathName());
}

TextParser::~TextParser()
{
  if (f)
  {
    fclose(f);
    f = NULL;
  }
  free(line);
}

bool TextParser::rewind()
{
  if (f)
  {
    ::rewind(f);
    return true;
  }
  else
    return false;
}

ProgressionStatePtr TextParser::getCurrentPosition() const
  {return new ProgressionState(lineNumber, 0, "Lines");}

inline int indexOfAnyNotOf(const String& str, int strLength, const String& characters, int startPosition = 0)
{
  for (int i = startPosition; i < strLength; ++i)
    if (characters.indexOfChar(str[i]) < 0)
      return i;
  return -1;
}

void TextParser::tokenize(const String& line, std::vector<String>& columns, const juce::tchar* separators)
{
  int lineLength = line.length();
  int b = indexOfAnyNotOf(line, lineLength, separators);
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
    b = indexOfAnyNotOf(line, lineLength, separators, e);
  }
}
  
Variable TextParser::next()
{
  if (!f)
    return Variable();
  if (!currentResult.exists())
  {
    //parsingBreaked = false;
    parseBegin();
  }
  currentResult = Variable();
  
  while (f)
  {
    line = readNextLine();
    if (!line)
    {
      fclose(f); f = NULL;
      if (!parseEnd())
        context.errorCallback(T("TextParser::next"), T("Error in parse end"));
      return currentResult;
    }
    
    ++lineNumber;
    if (!parseLine(line))
    {
      String lineString(line);
      context.errorCallback(T("-> Could not parse line ") + String((int)lineNumber) + T(": ") + (lineString.length() > 10 ? lineString.substring(0, 10) + T("...") : lineString));
      if (f) {fclose(f); f = NULL;}
      return Variable();
    }
    if (currentResult.exists())
      return currentResult;
  }
  return Variable();
}

char* TextParser::readNextLine()
{
  if (fgets(line, maxLineLength, f) == NULL)
    return NULL;

  char* ptr;
  while ((ptr = strrchr(line, '\n')) == NULL)
  {
    maxLineLength *= 2;
    line = (char* )realloc(line, maxLineLength);
    int len = (int)strlen(line);
    if (fgets(line + len, maxLineLength - len, f) == NULL)
      break;
  }
  if (ptr)
    *ptr = 0; // remove '\n' character
  return line;
}
