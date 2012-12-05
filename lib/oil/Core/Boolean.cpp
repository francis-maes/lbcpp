/*-----------------------------------------.---------------------------------.
| Filename: Boolean.cpp                    | Boolean                         |
| Author  : Francis Maes                   |                                 |
| Started : 12/11/2012 17:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <oil/Core/Boolean.h>
#include <oil/Core/XmlSerialisation.h>
#include <oil/Execution/ExecutionContext.h>

namespace lbcpp { // compilation problem under macosx, since there is a name conflict with "Boolean" in mactypes.h

string Boolean::toShortString() const
  {return value ? "true" : "false";}

string Boolean::toString() const
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

bool Boolean::loadFromString(ExecutionContext& context, const string& str)
{
  string v = str.trim().toLowerCase();
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
  
}; /* namespace lbcpp */
