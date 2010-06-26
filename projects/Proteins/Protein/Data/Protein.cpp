/*-----------------------------------------.---------------------------------.
| Filename: Protein.cpp                    | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Protein.h"
using namespace lbcpp;

class ProteinClass : public ObjectClass
{
public:
  ProteinClass() : ObjectClass(T("Protein"), objectClass())
  {
    addVariable(vectorClass(aminoAcidTypeEnumeration()), T("primaryStructure"));
    addVariable(vectorClass(secondaryStructureElementEnumeration()), T("secondaryStructure"));
    addVariable(vectorClass(dsspSecondaryStructureElementEnumeration()), T("dsspSecondaryStructure"));

    //addVariable(vectorClass(probabilityClass()), T("solventAccesibility"));
    //addVariable(vectorClass(probabilityClass()), T("solventAccesibilityAt20p"));
    addVariable(vectorClass(booleanClass()), T("disorderRegions"));
    //addVariable(vectorClass(probabilityClass()), T("disorderRegionProbabilities"));

    /*addVariable(symmetricMatrixClass(probabilityClass()), T("contactMap8Ca"));
    addVariable(symmetricMatrixClass(probabilityClass()), T("contactMap8Cb"));
    addVariable(symmetricMatrixClass(angstromDistanceClass()), T("distanceMapCa"));
    addVariable(symmetricMatrixClass(angstromDistanceClass()), T("distanceMapCb"));*/
  }
};

void declareProteinClass()
{
  Class::declare(new ProteinClass());
}
