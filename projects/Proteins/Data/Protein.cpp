/*-----------------------------------------.---------------------------------.
| Filename: Protein.cpp                    | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Protein.h"
#include "Formats/PDBFileParser.h"
#include "Formats/PDBFileGenerator.h"
#include "Formats/FASTAFileParser.h"
#include "Formats/FASTAFileGenerator.h"
using namespace lbcpp;

TypePtr lbcpp::sequenceSeparationDistanceType()
  {static TypeCache cache(T("SequenceSeparationDistance")); return cache();}

///////////////////

ProteinPtr Protein::createFromPDB(const File& pdbFile, bool beTolerant, ErrorHandler& callback)
{
  ReferenceCountedObjectPtr<PDBFileParser> parser(new PDBFileParser(pdbFile, beTolerant, callback));
  if (!parser->next())
    return ProteinPtr();
  
  std::vector<ProteinPtr> proteins = parser->getAllChains();
  jassert(proteins.size());
  ProteinPtr res = proteins[0];
  if (proteins.size() > 1)
  {
    size_t chainSize = proteins[0]->getLength();
    for (size_t i = 1; i < proteins.size(); ++i)
      if (proteins[i]->getLength() != chainSize)
      {
        for (size_t j = 0; j < proteins.size(); ++j)
          std::cerr << "Chain Size: " << proteins[j]->getLength() << std::endl;
        callback.errorMessage(T("ProteinObject::createFromPDB"), T("This file contains chains of different size, I do not know which one to choose"));
        return ProteinPtr();
      }
  }
  
  VectorPtr primaryStructure = res->getPrimaryStructure();
  jassert(primaryStructure);
  TertiaryStructurePtr tertiaryStructure = res->getTertiaryStructure();
  jassert(tertiaryStructure && tertiaryStructure->getNumResidues() == primaryStructure->getNumVariables());
  return res;
}

ProteinPtr Protein::createFromXml(const File& file, ErrorHandler& callback)
  {return Variable::createFromFile(file, callback).getObjectAndCast<Protein>();}

ProteinPtr Protein::createFromFASTA(const File& file, ErrorHandler& callback)
  {return StreamPtr(new FASTAFileParser(file, callback))->next().getObjectAndCast<Protein>(callback);}

void Protein::saveToPDBFile(const File& pdbFile, ErrorHandler& callback) const
  {ConsumerPtr(new PDBFileGenerator(pdbFile, callback))->consume(ProteinPtr(const_cast<Protein* >(this)));}

void Protein::saveToXmlFile(const File& xmlFile, ErrorHandler& callback) const
  {Variable(const_cast<Protein* >(this)).saveToFile(xmlFile, callback);}

void Protein::saveToFASTAFile(const File& fastaFile, ErrorHandler& callback) const
  {ConsumerPtr(new FASTAFileGenerator(fastaFile, callback))->consume(ProteinPtr(const_cast<Protein* >(this)));}

void Protein::setPrimaryStructure(const String& primaryStructure)
{
  jassert(!this->primaryStructure);
  size_t n = primaryStructure.length();

  this->primaryStructure = new Vector(aminoAcidTypeEnumeration(), n);
  for (size_t i = 0; i < n; ++i)
    this->primaryStructure->setVariable(i, AminoAcid::fromOneLetterCode(primaryStructure[i]));
}

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

SymmetricMatrixPtr Protein::getDistanceMap(bool betweenCBetaAtoms) const
{
  return betweenCBetaAtoms ? distanceMapCb : distanceMapCa;
}

void Protein::setDistanceMap(SymmetricMatrixPtr distanceMap, bool betweenCBetaAtoms)
{
  if (betweenCBetaAtoms)
    distanceMapCb = distanceMap;
  else
    distanceMapCa = distanceMap;
}

String Protein::getTargetFriendlyName(size_t index)
{
  // skip base class variables
  size_t baseClassVariables = nameableObjectClass()->getNumStaticVariables();
  if (index < baseClassVariables)
    return String::empty;
  index -= baseClassVariables;

  switch (index)
  {
  case 0: return T("Primary Structure");
  case 1: return T("PSSM");
  case 2: return T("Secondary Structure");
  case 3: return T("DSSP Secondary Structure");
  case 4: return T("Structural Alphabet");
  case 5: return T("Solvent Accessibility");
  case 6: return T("Solvent Accessibility at 20%"); 
  case 7: return T("Disorder Regions");
  case 8: return T("Contact Map C-alpha 8");
  case 9: return T("Contact Map C-beta 8");
  case 10: return T("Distance Map C-alpha");
  case 11: return T("Distance Map C-beta");
  case 12: return T("C-alpha Trace");
  case 13: return T("Tertiary Structure");
  default:
    jassert(false); 
    return String::empty;
  }
}



/*
** Compute Missing Targets
*/
void Protein::computeMissingVariables()
{
  jassert(primaryStructure);
  if (dsspSecondaryStructure && !secondaryStructure)
    secondaryStructure = computeSecondaryStructureFromDSSPSecondaryStructure(dsspSecondaryStructure);
  
  if (solventAccessibility && !solventAccessibilityAt20p)
    solventAccessibilityAt20p = computeBinarySolventAccessibilityFromSolventAccessibility(solventAccessibility, 0.2);

  if (tertiaryStructure)
  {
    if (!calphaTrace)
      calphaTrace = computeCAlphaTraceFromTertiaryStructure(tertiaryStructure);
    if (!distanceMapCa && tertiaryStructure->hasCAlphaAtoms())
      distanceMapCa = computeDistanceMapFromTertiaryStructure(tertiaryStructure, false);
    if (!distanceMapCb && tertiaryStructure->hasBackboneAndCBetaAtoms())
      distanceMapCb = computeDistanceMapFromTertiaryStructure(tertiaryStructure, true);
  }

  if (calphaTrace && !structuralAlphabetSequence)
    structuralAlphabetSequence = computeStructuralAlphabetSequenceFromCAlphaTrace(calphaTrace);

  if (distanceMapCa && !contactMap8Ca)
    contactMap8Ca = computeContactMapFromDistanceMap(distanceMapCa, 8);
  if (distanceMapCb && !contactMap8Cb)
    contactMap8Cb = computeContactMapFromDistanceMap(distanceMapCb, 8);
}

VectorPtr Protein::computeSecondaryStructureFromDSSPSecondaryStructure(VectorPtr dsspSecondaryStructure)
{
  size_t n = dsspSecondaryStructure->getNumVariables();
  VectorPtr res = new Vector(secondaryStructureElementEnumeration(), n);
  for (size_t i = 0; i < n; ++i)
  {
    Variable var = dsspSecondaryStructure->getVariable(i);
    if (var)
      res->setVariable(i, Variable(dsspSecondaryStructureToSecondaryStructure((DSSPSecondaryStructureElement)var.getInteger()), res->getElementsType()));
  }
  return res;
}

VectorPtr Protein::computeBinarySolventAccessibilityFromSolventAccessibility(VectorPtr solventAccessibility, double threshold)
{
  size_t n = solventAccessibility->size();
  VectorPtr res = new Vector(probabilityType(), n);
  for (size_t i = 0; i < n; ++i)
  {
    Variable sa = solventAccessibility->getVariable(i);
    if (sa)
      res->setVariable(i, Variable(sa.getDouble() > threshold ? 1.0 : 0.0, probabilityType()));
  }
  return res;
}

SymmetricMatrixPtr Protein::computeContactMapFromDistanceMap(SymmetricMatrixPtr distanceMap, double threshold)
{
  size_t n = distanceMap->getDimension();
  SymmetricMatrixPtr res = new SymmetricMatrix(probabilityType(), distanceMap->getDimension());
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i; j < n; ++j)
    {
      Variable distance = distanceMap->getElement(i, j);
      if (distance)
      {
        jassert(distance.isDouble());
        res->setElement(i, j, Variable(distance.getDouble() <= threshold ? 1.0 : 0.0, probabilityType()));
      }
    }
  return res;
}

VectorPtr Protein::computeStructuralAlphabetSequenceFromCAlphaTrace(CartesianPositionVectorPtr calphaTrace)
{
  /*
  ** Descriptors from "A HMM derived structural alphabet for proteins" (2004)
  */
  static const double structuralAlphabetDescriptiors[27][4] = {
    {5.39 ,5.09 ,5.38 ,2.92},
    {5.43 ,5.09 ,5.42 ,2.94},
    {5.41 ,5.23 ,5.61 ,2.86},
    {5.62 ,5.25 ,5.42 ,2.87},
    {5.59 ,5.49 ,5.78 ,2.58},
    {5.40 ,5.58 ,5.42 ,3.39},
    {5.78 ,5.68 ,6.07 ,1.46},
    {5.55 ,7.74 ,5.60 ,-3.31},
    {5.60 ,6.71 ,5.50 ,3.69},
    {5.69 ,8.09 ,5.67 ,3.09},
    {5.66 ,8.95 ,6.54 ,2.09},
    {5.66 ,8.91 ,6.66 ,-1.46},
    {5.66 ,8.07 ,6.70 ,2.96},
    {5.70 ,7.26 ,7.02 ,0.88},
    {6.03 ,6.85 ,5.64 ,-0.63},
    {6.47 ,5.92 ,5.56 ,0.53},
    {6.57 ,8.96 ,5.58 ,-2.19},
    {6.71 ,8.27 ,5.47 ,-3.56},
    {6.21 ,9.21 ,5.77 ,0.27},
    {6.87 ,8.28 ,6.03 ,-3.44},
    {6.89 ,8.94 ,6.76 ,-0.48},
    {6.72 ,9.12 ,6.41 ,-3.31},
    {6.71 ,9.64 ,6.50 ,-2.60},
    {6.39 ,9.93 ,6.75 ,-1.07},
    {6.87 ,10.06 ,6.51 ,-1.41},
    {6.48 ,10.17 ,7.09 ,0.66},
    {6.80 ,10.35 ,6.85 ,-0.25}
  };

  size_t n = calphaTrace->size();
  VectorPtr res = new Vector(structuralAlphabetElementEnumeration(), n);
  for (size_t i = 3; i < n; ++i)
  {
    impl::Vector3 a = calphaTrace->getPosition(i - 3);
    impl::Vector3 b = calphaTrace->getPosition(i - 2);
    impl::Vector3 c = calphaTrace->getPosition(i - 1);
    impl::Vector3 d = calphaTrace->getPosition(i);
    if (!a.exists() || !b.exists() || !c.exists() || !d.exists())
      continue;
    
    double d1 = (a - c).l2norm();
    double d2 = (a - d).l2norm();
    double d3 = (b - d).l2norm();
    
    impl::Vector3 normalVector =  (b - a).crossProduct(c - a);
    
    double d4 = (normalVector.dotProduct(d) - normalVector.dotProduct(a)) / normalVector.l2norm();
    
    int bestGroup = 0;
    double minDistance = DBL_MAX;
    for (size_t j = 0; j < 27; ++j)
    {
      double diff_1 = d1 - structuralAlphabetDescriptiors[j][0];
      double diff_2 = d2 - structuralAlphabetDescriptiors[j][1];
      double diff_3 = d3 - structuralAlphabetDescriptiors[j][2];
      double diff_4 = d4 - structuralAlphabetDescriptiors[j][3];
      
      diff_1 *= diff_1;
      diff_2 *= diff_2;
      diff_3 *= diff_3;
      diff_4 *= diff_4;
      
      double dist = sqrt(diff_1 + diff_2 + diff_3 + diff_4);
      
      if (dist < minDistance)
      {
        minDistance = dist;
        bestGroup = j;
      }
    }
    res->setVariable(i - 2, Variable(bestGroup, structuralAlphabetElementEnumeration()));
    //std::cout << d1 << "\t" << d2 << "\t" << d3 << "\t" << d4 << "\t\t" << bestGroup << std::endl;
  }
  return res;
}

/*
** Create Empty Targets
*/
VectorPtr Protein::createEmptyPositionSpecificScoringMatrix() const
  {return new Vector(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration()), getLength());}

VectorPtr Protein::createEmptySecondaryStructure() const
  {return new Vector(secondaryStructureElementEnumeration(), getLength());}

VectorPtr Protein::createEmptyDSSPSecondaryStructure() const
  {return new Vector(dsspSecondaryStructureElementEnumeration(), getLength());}

VectorPtr Protein::createEmptyProbabilitySequence() const
  {return new Vector(probabilityType(), getLength());}

SymmetricMatrixPtr Protein::createEmptyContactMap() const
  {return new SymmetricMatrix(probabilityType(), getLength());}

SymmetricMatrixPtr Protein::createEmptyDistanceMap() const
  {return new SymmetricMatrix(angstromDistanceType(), getLength());}

Variable Protein::createEmptyTarget(size_t index) const
{
  size_t n = getLength();

  // skip base class variables
  size_t baseClassVariables = nameableObjectClass()->getNumStaticVariables();
  jassert(index >= baseClassVariables)
  index -= baseClassVariables;

  switch (index)
  {
  case 1: return createEmptyPositionSpecificScoringMatrix();
  case 2: return createEmptySecondaryStructure();
  case 3: return createEmptyDSSPSecondaryStructure();
  case 4: return new Vector(structuralAlphabetElementEnumeration(), n);
  case 5: return createEmptyProbabilitySequence();
  case 6: return createEmptyProbabilitySequence();
  case 7: return createEmptyProbabilitySequence();
  case 8: case 9: return createEmptyContactMap();
  case 10: case 11: return createEmptyDistanceMap();
  default:
    jassert(false); return Variable();
  }
}

/*
** ProteinLengthFunction
*/
class ProteinLengthFunction : public Function
{
public:
  virtual TypePtr getInputType() const
    {return proteinClass();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return sequenceSeparationDistanceType();}

  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const
  {
    ProteinPtr protein = input.getObjectAndCast<Protein>();
    jassert(protein);
    return protein->getLength();
  }
};

FunctionPtr lbcpp::proteinLengthFunction()
  {return new ProteinLengthFunction();}

/*
** ProteinToInputOutputPairFunction
*/
class ProteinToInputOutputPairFunction : public Function
{
public:
  virtual TypePtr getInputType() const
    {return proteinClass();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return pairType(proteinClass(), proteinClass());}

  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const
  {
    ProteinPtr protein = input.getObjectAndCast<Protein>();
    jassert(protein);
    protein->computeMissingVariables();
    ProteinPtr inputProtein = new Protein(protein->getName());
    inputProtein->setPrimaryStructure(protein->getPrimaryStructure());
    inputProtein->setPositionSpecificScoringMatrix(protein->getPositionSpecificScoringMatrix());
    return Variable::pair(inputProtein, protein);
  }
};

FunctionPtr lbcpp::proteinToInputOutputPairFunction()
  {return new ProteinToInputOutputPairFunction();}


void declareProteinMiscTypes()
{
  Class::declare(new IntegerType(T("SequenceSeparationDistance"), integerType()));

  LBCPP_DECLARE_CLASS(ProteinLengthFunction, Function);
  LBCPP_DECLARE_CLASS(ProteinToInputOutputPairFunction, Function);
}
