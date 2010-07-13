/*-----------------------------------------.---------------------------------.
| Filename: SecondaryStructure.cpp         | Secondary Structure related     |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 01:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "SecondaryStructure.h"
using namespace lbcpp;

static const juce::tchar* secondaryStructureElementStrings[] = {
  T("Helix"), T("Sheet"), T("Other"), NULL
};

EnumerationPtr lbcpp::secondaryStructureElementEnumeration()
  {static EnumerationPtr res = Enumeration::get(T("SecondaryStructureElement")); return res;}

static const juce::tchar* dsspSecondaryStructureElementStrings[] = {
  T("Three Turn Helix"),
  T("Alpha Helix"),
  T("Pi Helix"),
  T("Hydrogen bonded turn"),
  T("Extended Strand in Sheet"),
  T("Residue in Isolated Bridge"),
  T("Bend"),
  T("Coil"),
  NULL
};

EnumerationPtr lbcpp::dsspSecondaryStructureElementEnumeration()
  {static EnumerationPtr res = Enumeration::get(T("DSSPSecondaryStructureElement")); return res;}

EnumerationPtr lbcpp::structuralAlphaElementEnumeration()
  {static EnumerationPtr res = Enumeration::get(T("StructuralAlphabetElement")); return res;}

void declareSecondaryStructureClasses()
{
  Class::declare(new Enumeration(T("SecondaryStructureElement"), secondaryStructureElementStrings, T("HEC")));
  Class::declare(new Enumeration(T("DSSPSecondaryStructureElement"), dsspSecondaryStructureElementStrings, T("GHITEBSC")));
  Class::declare(new Enumeration(T("StructuralAlphabetElement"), T("ABCDEFGHIJKLMNOPQRSTUVWXYZa")));
}
