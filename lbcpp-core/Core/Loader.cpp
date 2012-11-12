/*-----------------------------------------.---------------------------------.
| Filename: Loader.cpp                     | File Loader Base Class          |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 13:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Core/Loader.h>
using namespace lbcpp;

/*
** Loader
*/
bool Loader::canUnderstand(ExecutionContext& context, juce::InputStream& istr) const
{
  jassertfalse;
  return false;
}

bool Loader::canUnderstand(ExecutionContext& context, const juce::File& file) const
{
  string failureReason;
  
  juce::InputStream* istr = openFile(context, file, false);
  bool ok = false; 
  if (istr)
  {
    ok = canUnderstand(context, *istr);
    delete istr;
  }
  return ok;
}

ObjectPtr Loader::loadFromStream(ExecutionContext& context, juce::InputStream& istr, const string& streamName) const
{
  jassertfalse;
  return ObjectPtr();
}

ObjectPtr Loader::loadFromFile(ExecutionContext& context, const juce::File& file) const
{
  juce::InputStream* stream = openFile(context, file);
  if (!stream)
    return ObjectPtr();
  ObjectPtr res = loadFromStream(context, *stream, file.getFileName());
  delete stream;
  return res;
}

juce::InputStream* Loader::openFile(ExecutionContext& context, const juce::File& file, bool doErrorMessages)
{
  if (!file.existsAsFile())
  {
    if (doErrorMessages)
      context.errorCallback("File " + file.getFullPathName() + " does not exists");
    return NULL;
  }
  juce::InputStream* stream = file.createInputStream();
  if (!stream)
  {
    if (doErrorMessages)
      context.errorCallback("Could not open file " + file.getFullPathName());
    return NULL;
  }
  return stream;
}

string Loader::readFirstLine(juce::InputStream& istr, size_t maxLength)
{
  juce::int64 position = istr.getPosition();
  string res;
  for (size_t i = 0; i < maxLength; ++i)
  {
    if (istr.isExhausted())
      break;
    char c = istr.readByte();
    if (c == 0)
      break;
    else
      res += c;
  }
  istr.setPosition(position);
  return res;
}

bool Loader::guessIfIsText(juce::InputStream& istr)
{
  enum {maxLength = 256};
  juce::int64 position = istr.getPosition();
  string res;
  for (size_t i = 0; i < maxLength; ++i)
  {
    if (istr.isExhausted())
      break;
    char c = istr.readByte();
    if (c > 0 && c < 32 && c != 9 && c != 10 && c != 13)
      return false;
  }
  istr.setPosition(position);
  return true;
}

/*
** TextLoader
*/
bool TextLoader::canUnderstand(ExecutionContext& context, juce::InputStream& istr) const
  {return guessIfIsText(istr);}

# ifdef JUCE_WIN32
#  pragma warning(disable:4996) // microsoft visual does not like fopen()/fclose()
# endif // JUCE_WIN32

ObjectPtr TextLoader::loadFromFile(ExecutionContext& context, const juce::File& file) const
{
  FILE* f = fopen(file.getFullPathName(), "r");
  if (!f)
  {
    context.errorCallback("Could not open file " + file.getFullPathName());
    return ObjectPtr();
  }

  size_t maxLineLength = 1024;
  char* line = (char* )malloc(sizeof (char) * maxLineLength);
  bool failed = false;

  TextLoader* pthis = const_cast<TextLoader* >(this);
  pthis->parseBegin(context);
  for (size_t lineNumber = 0; true; ++lineNumber)
  {
    if (!readNextLine(f, line, maxLineLength))
      break;    
    string lineString(line);
    if (!pthis->parseLine(context, lineString))
    {
      context.errorCallback(T("-> Could not parse line ") + string((int)lineNumber) + T(": ") + (lineString.length() > 10 ? lineString.substring(0, 10) + T("...") : lineString));
      failed = true;
      break;
    }
  }
  fclose(f);
  free(line);
  if (failed)
    return ObjectPtr();
  ObjectPtr res = pthis->parseEnd(context);
  if (!res)
    context.errorCallback(T("TextParser::next"), T("Error in parse end"));
  return res;
}

ObjectPtr TextLoader::loadFromStream(ExecutionContext& context, juce::InputStream& istr, const string& streamName) const
{
  jassertfalse; // not implemented yet
  return ObjectPtr();
}

bool TextLoader::readNextLine(FILE* f, char* line, size_t& maxLineLength)
{
  if (fgets(line, maxLineLength, f) == NULL)
    return false;

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
  return true;
}
