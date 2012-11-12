/*-----------------------------------------.---------------------------------.
| Filename: String.cpp                     | String                          |
| Author  : Francis Maes                   |                                 |
| Started : 12/11/2012 17:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/String.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Execution/ExecutionContext.h>
using namespace lbcpp;

/*
** NewString
*/
NewFilePtr NewFile::create(const File& file)
{
  if (file.isDirectory())
    return new Directory(file);
  else
    return new NewFile(file);
}

juce::String NewString::toShortString() const
  {return value;}

String NewString::toString() const
  {return value.quoted();}

int NewString::compare(const ObjectPtr& otherObject) const
{
  const NewStringPtr& other = otherObject.staticCast<NewString>();
  if (value < other->get())
    return -1;
  else if (value > other->get())
    return 1;
  else
    return 0;
}

void NewString::clone(ExecutionContext& context, const ObjectPtr& target) const
  {target.staticCast<NewString>()->value = value;}
  
bool NewString::loadFromString(ExecutionContext& context, const String& str)
{
  value = str.trim();
  if (value.startsWithChar('"'))
    value = value.unquoted();
  return true;
}

bool NewString::loadFromXml(XmlImporter& importer)
  {return loadFromString(importer.getContext(), importer.getAllSubText());}

void NewString::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}

/*
** NewFile
*/
juce::String NewFile::toShortString() const
  {return get().getFileName();}

juce::String NewFile::toString() const
  {return defaultExecutionContext().getFilePath(get());}

void NewFile::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}

bool NewFile::loadFromString(ExecutionContext& context, const String& str)
{
  juce::File file = context.getFile(str);
  if (file == File::nonexistent)
  {
    context.errorCallback(T("Could not find file ") + str);
    return false;
  }
  set(file);
  return true;
}
