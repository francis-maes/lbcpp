/*-----------------------------------------.---------------------------------.
| Filename: Protein.cpp                    | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Protein.h"
using namespace lbcpp;

class ProteinClass : public DynamicClass
{
public:
  ProteinClass() : DynamicClass(T("Protein"))
  {
    addVariable(vectorClass(aminoAcidTypeEnumeration()), T("primaryStructure"));
    addVariable(vectorClass(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration())), T("positionSpecificScoringMatrix"));

    addVariable(vectorClass(secondaryStructureElementEnumeration()), T("secondaryStructure"));
    addVariable(vectorClass(dsspSecondaryStructureElementEnumeration()), T("dsspSecondaryStructure"));

    addVariable(vectorClass(probabilityType()), T("solventAccesibility"));
    addVariable(vectorClass(probabilityType()), T("solventAccesibilityAt20p"));
    addVariable(vectorClass(booleanType()), T("disorderRegions"));
    addVariable(vectorClass(probabilityType()), T("disorderRegionProbabilities"));

    addVariable(symmetricMatrixClass(probabilityType()), T("contactMap8Ca"));
    addVariable(symmetricMatrixClass(probabilityType()), T("contactMap8Cb"));
    addVariable(symmetricMatrixClass(angstromDistanceType()), T("distanceMapCa"));
    addVariable(symmetricMatrixClass(angstromDistanceType()), T("distanceMapCb"));
  }
};

void declareProteinClass()
{
  Class::declare(new ProteinClass());
}

extern ClassPtr lbcpp::proteinClass()
  {static ClassPtr res = Class::get(T("Protein")); return res;}

///////////////////

Variable Protein::getVariable(size_t index) const
{
  switch (index)
  {
  case 0: return primaryStructure;
  case 1: return positionSpecificScoringMatrix;
  case 2: return secondaryStructure;
  case 3: return dsspSecondaryStructure;
  case 4: return solventAccessibility;
  case 5: return solventAccessibilityAt20p;
  case 6: return disorderRegions;
  case 7: return disorderRegionProbabilities;
  case 8: return contactMap8Ca;
  case 9: return contactMap8Cb;
  case 10: return distanceMapCa;
  case 11: return distanceMapCb;
  };
  jassert(false);
  return Variable();
}

void Protein::computeMissingVariables()
{
  if (primaryStructure)
  {
  }
}
