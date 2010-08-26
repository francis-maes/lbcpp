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

void Atom::saveToXml(XmlExporter& exporter) const
  {saveVariablesToXmlAttributes(exporter);}

bool Atom::loadFromXml(XmlImporter& importer)
  {return loadVariablesFromXmlAttributes(importer);}
