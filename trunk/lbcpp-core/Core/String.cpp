/*-----------------------------------------.---------------------------------.
| Filename: string.cpp                     | string                          |
| Author  : Francis Maes                   |                                 |
| Started : 12/11/2012 17:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/string.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Execution/ExecutionContext.h>
using namespace lbcpp;

/*
** String
*/
FilePtr File::create(const juce::File& file)
{
  if (file.isDirectory())
    return new Directory(file);
  else
    return new File(file);
}

string String::toShortString() const
  {return value;}

string String::toString() const
  {return value.quoted();}

int String::compare(const ObjectPtr& otherObject) const
{
  const NewStringPtr& other = otherObject.staticCast<String>();
  if (value < other->get())
    return -1;
  else if (value > other->get())
    return 1;
  else
    return 0;
}

void String::clone(ExecutionContext& context, const ObjectPtr& target) const
  {target.staticCast<String>()->value = value;}
  
bool String::loadFromString(ExecutionContext& context, const string& str)
{
  value = str.trim();
  if (value.startsWithChar('"'))
    value = value.unquoted();
  return true;
}

bool String::loadFromXml(XmlImporter& importer)
  {return loadFromString(importer.getContext(), importer.getAllSubText());}

void String::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}

/*
** File
*/
string File::toShortString() const
  {return get().getFileName();}

string File::toString() const
  {return defaultExecutionContext().getFilePath(get());}

void File::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}

bool File::loadFromString(ExecutionContext& context, const string& str)
{
  juce::File file = context.getFile(str);
  if (file == juce::File::nonexistent)
  {
    context.errorCallback(T("Could not find file ") + str);
    return false;
  }
  set(file);
  return true;
}
