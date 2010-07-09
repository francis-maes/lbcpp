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
  ProteinClass() : DynamicClass(T("Protein"), nameableObjectClass())
  {
    addVariable(vectorClass(aminoAcidTypeEnumeration()), T("primaryStructure"));
    addVariable(vectorClass(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration())), T("positionSpecificScoringMatrix"));

    addVariable(vectorClass(secondaryStructureElementEnumeration()), T("secondaryStructure"));
    addVariable(vectorClass(dsspSecondaryStructureElementEnumeration()), T("dsspSecondaryStructure"));

    addVariable(vectorClass(probabilityType()), T("solventAccesibility"));
    addVariable(vectorClass(probabilityType()), T("solventAccesibilityAt20p"));
    addVariable(vectorClass(probabilityType()), T("disorderRegions"));

    addVariable(symmetricMatrixClass(probabilityType()), T("contactMap8Ca"));
    addVariable(symmetricMatrixClass(probabilityType()), T("contactMap8Cb"));
    addVariable(symmetricMatrixClass(angstromDistanceType()), T("distanceMapCa"));
    addVariable(symmetricMatrixClass(angstromDistanceType()), T("distanceMapCb"));
  }

  virtual VariableValue create() const
    {return new Protein();}
};

void declareProteinClass()
{
  Class::declare(new ProteinClass());
}

extern ClassPtr lbcpp::proteinClass()
  {static ClassPtr res = Class::get(T("Protein")); return res;}

///////////////////

SymmetricMatrixPtr Protein::getContactMap(double threshold, bool betweenCBetaAtoms) const
{
  jassert(threshold == 8.0);
  return betweenCBetaAtoms ? contactMap8Cb : contactMap8Ca;
}

void Protein::setContactMap(SymmetricMatrixPtr contactMap, double threshold, bool betweenCBetaAtoms)
{
  jassert(threshold == 8.0);
  if (betweenCBetaAtoms)
    contactMap8Cb = contactMap;
  else
    contactMap8Ca = contactMap;
}

Variable Protein::getVariable(size_t index) const
{
  size_t baseClassVariables = nameableObjectClass()->getNumStaticVariables();
  if (index < baseClassVariables)
    return NameableObject::getVariable(index);
  index -= baseClassVariables;
  switch (index)
  {
  case 0: return primaryStructure;
  case 1: return positionSpecificScoringMatrix;
  case 2: return secondaryStructure;
  case 3: return dsspSecondaryStructure;
  case 4: return solventAccessibility;
  case 5: return solventAccessibilityAt20p;
  case 6: return disorderRegions;
  case 7: return contactMap8Ca;
  case 8: return contactMap8Cb;
  case 9: return distanceMapCa;
  case 10: return distanceMapCb;
  };
  jassert(false);
  return Variable();
}

void Protein::setVariable(size_t index, const Variable& value)
{
  size_t baseClassVariables = nameableObjectClass()->getNumStaticVariables();
  if (index < baseClassVariables)
  {
    NameableObject::setVariable(index, value);
    return;
  }
  index -= baseClassVariables;
  switch (index)
  {
  case 0: primaryStructure = value.getObjectAndCast<Vector>(); break;
  case 1: positionSpecificScoringMatrix = value.getObjectAndCast<Vector>(); break;
  case 2: secondaryStructure = value.getObjectAndCast<Vector>(); break;
  case 3: dsspSecondaryStructure = value.getObjectAndCast<Vector>(); break;
  case 4: solventAccessibility = value.getObjectAndCast<Vector>(); break;
  case 5: solventAccessibilityAt20p = value.getObjectAndCast<Vector>(); break;
  case 6: disorderRegions = value.getObjectAndCast<Vector>(); break;
  case 7: contactMap8Ca = value.getObjectAndCast<SymmetricMatrix>(); break;
  case 8: contactMap8Cb = value.getObjectAndCast<SymmetricMatrix>(); break;
  case 9: distanceMapCa = value.getObjectAndCast<SymmetricMatrix>(); break;
  case 10: distanceMapCb = value.getObjectAndCast<SymmetricMatrix>(); break;
  default: jassert(false);
  };
}

void Protein::computeMissingVariables()
{
  if (primaryStructure)
  {
  }
}
