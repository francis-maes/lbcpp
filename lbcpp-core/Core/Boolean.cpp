/*-----------------------------------------.---------------------------------.
| Filename: Boolean.cpp                    | Boolean                         |
| Author  : Francis Maes                   |                                 |
| Started : 12/11/2012 17:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Boolean.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Execution/ExecutionContext.h>
using namespace lbcpp;

String NewBoolean::toShortString() const
  {return value ? "true" : "false";}

String NewBoolean::toString() const
  {return value ? "true" : "false";}

double NewBoolean::toDouble() const
  {return value ? 1.0 : 0.0;}
  
bool NewBoolean::toBoolean() const
  {return value;}

int NewBoolean::compare(const ObjectPtr& otherObject) const
{
  const NewBooleanPtr& other = otherObject.staticCast<NewBoolean>();
  return (value ? 1 : 0) - (other->value ? 1 : 0);
}

void NewBoolean::clone(ExecutionContext& context, const ObjectPtr& target) const
  {target.staticCast<NewBoolean>()->value = value;}

bool NewBoolean::loadFromString(ExecutionContext& context, const String& str)
{
  String v = str.trim().toLowerCase();
  if (v == T("true"))
  {
    value = true;
    return true;
  }
  else if (v == T("false"))
  {
    value = false;
    return true;
  }
  else
  {
    context.errorCallback(T("NewBooleanClass::loadFromString"), T("Could not read boolean value ") + str.quoted());
    return false;
  }
}

bool NewBoolean::loadFromXml(XmlImporter& importer)
  {return loadFromString(importer.getContext(), importer.getAllSubText());}

void NewBoolean::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}
