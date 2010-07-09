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
  return getName() + T(" ") + position.toString() + T(" ")
    + lbcpp::toString(occupancy) + T(" ") + lbcpp::toString(temperatureFactor);
}

void Atom::saveToXml(XmlElement* xml) const
{
  xml->setAttribute(T("name"), name);
  xml->setAttribute(T("elementSymbol"), elementSymbol);
  xml->setAttribute(T("position"), position.toString());
  xml->setAttribute(T("occupancy"), occupancy);
  xml->setAttribute(T("temperatureFactor"), temperatureFactor);
}

bool Atom::loadFromXml(XmlElement* xml, ErrorHandler& callback)
{
  name = xml->getStringAttribute(T("name"));
  elementSymbol = xml->getStringAttribute(T("elementSymbol"));
  position = impl::Vector3::fromString(xml->getStringAttribute(T("position")), callback);
  occupancy = xml->getDoubleAttribute(T("occupancy"));
  temperatureFactor = xml->getDoubleAttribute(T("temperatureFactor"));
  return true;
}

ClassPtr lbcpp::atomClass()
  {static TypeCache cache(T("Atom")); return cache();}

class AtomClass : public DynamicClass
{
public:
  AtomClass() : DynamicClass(T("Atom"), objectClass())
  {
  }

  virtual VariableValue create() const
    {return new Atom();}
};

void declareAtomClasses()
{
  Class::declare(new AtomClass());
}