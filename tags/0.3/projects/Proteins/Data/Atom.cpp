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

void Atom::saveToXml(XmlElement* xml) const
  {saveVariablesToXmlAttributes(xml);}

bool Atom::loadFromXml(XmlElement* xml, ErrorHandler& callback)
  {return loadVariablesFromXmlAttributes(xml, callback);}

ClassPtr lbcpp::atomClass()
  {static TypeCache cache(T("Atom")); return cache();}

namespace lbcpp
{

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

  LBCPP_DECLARE_VARIABLE_BEGIN(Atom)
    LBCPP_DECLARE_VARIABLE(elementSymbol);
    LBCPP_DECLARE_VARIABLE(position);
    LBCPP_DECLARE_VARIABLE(occupancy);
    LBCPP_DECLARE_VARIABLE(temperatureFactor);
  LBCPP_DECLARE_VARIABLE_END()
};

}; /* namespace lbcpp */

void declareAtomClasses()
{
  Class::declare(new AtomClass());
}