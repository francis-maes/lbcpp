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

String Boolean::toShortString() const
  {return value ? "true" : "false";}

String Boolean::toString() const
  {return value ? "true" : "false";}

double Boolean::toDouble() const
  {return value ? 1.0 : 0.0;}
  
bool Boolean::toBoolean() const
  {return value;}

int Boolean::compare(const ObjectPtr& otherObject) const
{
  const BooleanPtr& other = otherObject.staticCast<Boolean>();
  return (value ? 1 : 0) - (other->value ? 1 : 0);
}

void Boolean::clone(ExecutionContext& context, const ObjectPtr& target) const
  {target.staticCast<Boolean>()->value = value;}

bool Boolean::loadFromString(ExecutionContext& context, const String& str)
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
    context.errorCallback(T("BooleanClass::loadFromString"), T("Could not read boolean value ") + str.quoted());
    return false;
  }
}

bool Boolean::loadFromXml(XmlImporter& importer)
  {return loadFromString(importer.getContext(), importer.getAllSubText());}

void Boolean::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}
