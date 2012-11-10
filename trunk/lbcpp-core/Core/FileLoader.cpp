/*-----------------------------------------.---------------------------------.
| Filename: FileLoader.cpp                 | File Loader Base Class          |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 13:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Core/FileLoader.h>
using namespace lbcpp;

/*
** FileLoader
*/
bool FileLoader::canUnderstand(ExecutionContext& context, juce::InputStream& istr) const
{
  jassertfalse;
  return false;
}

bool FileLoader::canUnderstand(ExecutionContext& context, const File& file) const
{
  String failureReason;
  
  juce::InputStream* istr = openFile(context, file, false);
  bool ok = false; 
  if (istr)
  {
    ok = canUnderstand(context, *istr);
    delete istr;
  }
  return ok;
}

ObjectPtr FileLoader::loadFromStream(ExecutionContext& context, juce::InputStream& istr, const String& streamName) const
{
  jassertfalse;
  return ObjectPtr();
}

ObjectPtr FileLoader::loadFromFile(ExecutionContext& context, const File& file) const
{
  juce::InputStream* stream = openFile(context, file);
  if (!stream)
    return ObjectPtr();
  ObjectPtr res = loadFromStream(context, *stream, file.getFileName());
  delete stream;
  return res;
}

juce::InputStream* FileLoader::openFile(ExecutionContext& context, const File& file, bool doErrorMessages)
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

String FileLoader::readFirstLine(juce::InputStream& istr, size_t maxLength)
{
  juce::int64 position = istr.getPosition();
  String res;
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

bool FileLoader::guessIfIsText(juce::InputStream& istr)
{
  enum {maxLength = 256};
  juce::int64 position = istr.getPosition();
  String res;
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
