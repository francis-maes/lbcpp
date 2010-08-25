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
    + String(occupancy) + T(" ") + String(temperatureFactor);
}

void Atom::saveToXml(XmlElement* xml) const
  {saveVariablesToXmlAttributes(xml);}

bool Atom::loadFromXml(XmlElement* xml, ErrorHandler& callback)
  {return loadVariablesFromXmlAttributes(xml, callback);}
