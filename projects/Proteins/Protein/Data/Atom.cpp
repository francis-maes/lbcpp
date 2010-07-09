/*-----------------------------------------.---------------------------------.
| Filename: Atom.cpp                       | Atom                            |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "Atom.h"
using namespace lbcpp;

String Atom::toString() const
{
  return getName() + T(" ") + position->toString() + T(" ")
    + lbcpp::toString(occupancy) + T(" ") + lbcpp::toString(temperatureFactor);
}

VariableReference Atom::getVariableReference(size_t index)
{
  if (index == 0)
    return NameableObject::getVariableReference(index);
  --index;
  switch (index)
  {
  case 0: return elementSymbol;
  case 1: return position;
  case 2: return occupancy;
  case 3: return temperatureFactor;
  default: jassert(false); return VariableReference();
  };
}

void Atom::saveToXml(XmlElement* xml) const
  {saveVariablesToXmlAttributes(xml);}

bool Atom::loadFromXml(XmlElement* xml, ErrorHandler& callback)
  {return loadVariablesFromXmlAttributes(xml, callback);}

ClassPtr lbcpp::atomClass()
  {static TypeCache cache(T("Atom")); return cache();}

class AtomClass : public DynamicClass
{
public:
  AtomClass() : DynamicClass(T("Atom"), nameableObjectClass())
  {
    addVariable(stringType(), T("elementSymbol"));
    addVariable(vector3Class(), T("position"));
    addVariable(doubleType(), T("occupancy"));
    addVariable(doubleType(), T("temperatureFactor"));
  }

  virtual VariableValue create() const
    {return new Atom();}
};

void declareAtomClasses()
{
  Class::declare(new AtomClass());
}