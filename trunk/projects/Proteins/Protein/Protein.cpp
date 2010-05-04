/*-----------------------------------------.---------------------------------.
| Filename: Protein.cpp                    | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 17/04/2010 14:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Protein.h"
#include "AminoAcidDictionary.h"
#include "SecondaryStructureDictionary.h"
#include "Formats/FASTAFileParser.h"
#include "Formats/FASTAFileGenerator.h"
#include "Formats/PDBFileParser.h"
#include "Formats/PDBFileGenerator.h"

using namespace lbcpp;

ProteinPtr Protein::createFromAminoAcidSequence(const String& name, const String& aminoAcidString)
{
  ProteinPtr res = new Protein(name);
  FeatureDictionaryPtr aminoAcidDictionary = AminoAcidDictionary::getInstance();
  LabelSequencePtr aminoAcidSequence = new LabelSequence(T("AminoAcidSequence"), aminoAcidDictionary, aminoAcidString.length());
  for (size_t i = 0; i < aminoAcidSequence->size(); ++i)
  {
    String aa;
    aa += aminoAcidString[i];
    int index = aminoAcidDictionary->getFeatures()->getIndex(aa);
    if (index < 0)
    {
      Object::error(T("Protein::createFromAminoAcidSequence"), T("Unknown amino acid: ") + aa);
      return ProteinPtr();
    }
    aminoAcidSequence->setIndex(i, (size_t)index);
  }
  res->setObject(aminoAcidSequence);
  return res;
}

ProteinPtr Protein::createFromFASTA(const File& fastaFile)
{
  ObjectStreamPtr parser(new FASTAFileParser(fastaFile));
  return parser->nextAndCast<Protein>();
}

ProteinPtr Protein::createFromPDB(const File& pdbFile, bool beTolerant)
{
  ReferenceCountedObjectPtr<PDBFileParser> parser(new PDBFileParser(pdbFile, beTolerant));
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
        Object::error(T("Protein::createFromPDB"), T("This file contains chains of different size, I do not know which one to choose"));
        return ProteinPtr();
      }
  }

  LabelSequencePtr aminoAcidSequence = res->getAminoAcidSequence();
  jassert(aminoAcidSequence);
  ProteinTertiaryStructurePtr tertiaryStructure = res->getTertiaryStructure();
  jassert(tertiaryStructure && tertiaryStructure->size() == aminoAcidSequence->size());
  return res;
}

void Protein::saveToPDBFile(const File& pdbFile)
  {ObjectConsumerPtr(new PDBFileGenerator(pdbFile))->consume(ProteinPtr(this));}

void Protein::saveToFASTAFile(const File& fastaFile)
  {ObjectConsumerPtr(new FASTAFileGenerator(fastaFile))->consume(ProteinPtr(this));}

void Protein::computeMissingFields()
{
  LabelSequencePtr aminoAcidSequence = getAminoAcidSequence();
  if (!aminoAcidSequence)
    return;

  LabelSequencePtr secondaryStructureSequence = getSecondaryStructureSequence();
  ScoreVectorSequencePtr secondaryStructureProbabilities = getSecondaryStructureProbabilities();
  LabelSequencePtr dsspSecondaryStructureSequence = getDSSPSecondaryStructureSequence();
  ScoreVectorSequencePtr dsspSecondaryStructureProbabilities = getDSSPSecondaryStructureProbabilities();
  ScalarSequencePtr normalizedSolventAccessibilitySequence = getNormalizedSolventAccessibilitySequence();
  LabelSequencePtr disorderSequence = getDisorderSequence();
  ScalarSequencePtr disorderProbabilities = getDisorderProbabilitySequence();
  
  ScoreSymmetricMatrixPtr residueResidueDistanceMatrixCb = getResidueResidueDistanceMatrixCb();
  ScoreSymmetricMatrixPtr residueResidueContactMatrix8Cb = getResidueResidueContactMatrix8Cb();
  ScoreSymmetricMatrixPtr residueResidueDistanceMatrixCa = getResidueResidueDistanceMatrixCa();
  ScoreSymmetricMatrixPtr residueResidueContactMatrix8Ca = getResidueResidueContactMatrix8Ca();


  /*
  ** Secondary Structure
  */
  // SS8 probabilities => SS8 LabelSequence
  if (dsspSecondaryStructureProbabilities && !dsspSecondaryStructureSequence)
    setObject(dsspSecondaryStructureSequence = dsspSecondaryStructureProbabilities->makeArgmaxLabelSequence(T("DSSPSecondaryStructureSequence")));

  // 8-state DSSP SS => 3-state SS
  if (dsspSecondaryStructureSequence && !secondaryStructureSequence)
    setObject(secondaryStructureSequence = SecondaryStructureDictionary::createSequenceFromDSSPSequence
      (T("SecondaryStructureSequence"), dsspSecondaryStructureSequence));

  // SS3 probabilities => SS3 LabelSequence
  if (secondaryStructureProbabilities && !secondaryStructureSequence)
    setObject(secondaryStructureSequence = secondaryStructureProbabilities->makeArgmaxLabelSequence(T("SecondaryStructureSequence")));

  /*
  ** Disorder Regions
  */
  // Disorder probabilities => Disorder Sequence
  if (disorderProbabilities && !disorderSequence)
    setObject(disorderSequence = disorderProbabilities->makeBinaryLabelSequence(T("DisorderSequence")));

  /*
  ** Solvent Accesiblity
  */
  if (normalizedSolventAccessibilitySequence)
  {
    // Normalized SA => SA 20%
    LabelSequencePtr solventAccessibility20 = getSolventAccessibilityThreshold20();
    if (!solventAccessibility20)
      setObject(solventAccessibility20 = normalizedSolventAccessibilitySequence->makeBinaryLabelSequence(T("SolventAccessibilityThreshold20"), 0.2));
  }

  /*
  ** Tertiary Structure
  */
  ProteinTertiaryStructurePtr tertiaryStructure = getTertiaryStructure();
  ProteinBackboneBondSequencePtr backbone = getBackboneBondSequence();
  CartesianCoordinatesSequencePtr calphaTrace = getCAlphaTrace();
  BondCoordinatesSequencePtr calphaBondSequence = getCAlphaBondSequence();

  // Tertiary Structure => CAlpha trace
  if (tertiaryStructure && !calphaTrace)
    setObject(calphaTrace = tertiaryStructure->makeCAlphaTrace());

  // Tertiary Structure => Backbone bonds
  if (tertiaryStructure && !backbone)
    setObject(backbone = tertiaryStructure->makeBackbone());

  // Backbone bonds => Tertiary Structure
  if (backbone && !tertiaryStructure)
    setObject(tertiaryStructure = ProteinTertiaryStructure::createFromBackbone(aminoAcidSequence, backbone));

  // CAlpha trace => CAlpha bonds
  if (calphaTrace && !calphaBondSequence)
    setObject(calphaBondSequence = new BondCoordinatesSequence(T("CAlphaBondSequence"), calphaTrace));
  

  /*
  ** Contact maps
  */
  // tertiary structure => distance matrix
  if (tertiaryStructure && !residueResidueDistanceMatrixCa && tertiaryStructure->hasCAlphaAtoms())
    setObject(residueResidueDistanceMatrixCa = tertiaryStructure->makeCAlphaDistanceMatrix());

  if (tertiaryStructure && !residueResidueDistanceMatrixCb && tertiaryStructure->hasBackboneAndCBetaAtoms())
    setObject(residueResidueDistanceMatrixCb = tertiaryStructure->makeCBetaDistanceMatrix());

  // distance matrix => contact matrix
  if (residueResidueDistanceMatrixCa && !residueResidueContactMatrix8Ca)
    setObject(residueResidueContactMatrix8Ca = residueResidueDistanceMatrixCa->makeThresholdedMatrix(T("ResidueResidueContactMatrix8Ca"), 8.0, 1.0, 0.0));
  if (residueResidueDistanceMatrixCb && !residueResidueContactMatrix8Cb)
    setObject(residueResidueContactMatrix8Cb = residueResidueDistanceMatrixCb->makeThresholdedMatrix(T("ResidueResidueContactMatrix8Cb"), 8.0, 1.0, 0.0));
}

ObjectPtr Protein::createEmptyObject(const String& name) const
{
  size_t n = getLength();

  if (name == T("AminoAcidSequence"))
    return new LabelSequence(name, AminoAcidDictionary::getInstance(), n);
  else if (name == T("PositionSpecificScoringMatrix"))
    return new ScoreVectorSequence(name, AminoAcidDictionary::getInstance(), n, AminoAcidDictionary::numAminoAcids);
  else if (name == T("SecondaryStructureSequence"))
    return new LabelSequence(name, SecondaryStructureDictionary::getInstance(), n);
  else if (name == T("SecondaryStructureProbabilities"))
    return new ScoreVectorSequence(name, SecondaryStructureDictionary::getInstance(), n);
  else if (name == T("DSSPSecondaryStructureSequence"))
    return new LabelSequence(name, DSSPSecondaryStructureDictionary::getInstance(), n);
  else if (name == T("DSSPSecondaryStructureProbabilities"))
    return new ScoreVectorSequence(name, DSSPSecondaryStructureDictionary::getInstance(), n);
  else if (name == T("NormalizedSolventAccessibilitySequence"))
    return new ScalarSequence(name, n);
  else if (name.startsWith(T("SolventAccessibilityThreshold")))
    return new LabelSequence(name, BinaryClassificationDictionary::getInstance(), n);
  else if (name == T("DisorderSequence"))
    return new LabelSequence(name, BinaryClassificationDictionary::getInstance(), n);
  else if (name == T("DisorderProbabilitySequence"))
    return new ScalarSequence(name, n);
  else if (name == T("ResidueResidueContactMatrix8Ca"))
    return new ScoreSymmetricMatrix(name, n);
  else if (name == T("ResidueResidueDistanceMatrixCa"))
    return new ScoreSymmetricMatrix(name, n);
  else if (name == T("ResidueResidueContactMatrix8Cb"))
    return new ScoreSymmetricMatrix(name, n);
  else if (name == T("ResidueResidueDistanceMatrixCb"))
    return new ScoreSymmetricMatrix(name, n);
  else if (name == T("BackboneBondSequence"))
    return new ProteinBackboneBondSequence(n);
  else if (name == T("CAlphaTrace"))
    return new CartesianCoordinatesSequence(name, n);
  else if (name == T("CAlphaBondSequence"))
    return new BondCoordinatesSequence(name, n - 1);
  else if (name == T("TertiaryStructure"))
    return new ProteinTertiaryStructure(n);
  else
  {
    jassert(false);
    return ObjectPtr();
  }
}

size_t Protein::getLength() const
  {return getAminoAcidSequence()->size();}

LabelSequencePtr Protein::getAminoAcidSequence() const
  {return getObject(T("AminoAcidSequence"));}

ScoreVectorSequencePtr Protein::getPositionSpecificScoringMatrix() const
  {return getObject(T("PositionSpecificScoringMatrix"));}

ScoreVectorSequencePtr Protein::getAminoAcidProperty() const
  {return getObject(T("AminoAcidProperty"));}

LabelSequencePtr Protein::getSecondaryStructureSequence() const
  {return getObject(T("SecondaryStructureSequence"));}

ScoreVectorSequencePtr Protein::getSecondaryStructureProbabilities() const
  {return getObject(T("SecondaryStructureProbabilities"));}

LabelSequencePtr Protein::getDSSPSecondaryStructureSequence() const
  {return getObject(T("DSSPSecondaryStructureSequence"));}

ScoreVectorSequencePtr Protein::getDSSPSecondaryStructureProbabilities() const
  {return getObject(T("DSSPSecondaryStructureProbabilities"));}

ScalarSequencePtr Protein::getNormalizedSolventAccessibilitySequence() const
  {return getObject(T("NormalizedSolventAccessibilitySequence"));}

LabelSequencePtr Protein::getSolventAccessibilityThreshold20() const
  {return getObject(T("SolventAccessibilityThreshold20"));}

LabelSequencePtr Protein::getDisorderSequence() const
  {return getObject(T("DisorderSequence"));}

ScalarSequencePtr Protein::getDisorderProbabilitySequence() const
  {return getObject(T("DisorderProbabilitySequence"));}

ScoreSymmetricMatrixPtr Protein::getResidueResidueContactMatrix8Ca() const
  {return getObject(T("ResidueResidueContactMatrix8Ca"));}

ScoreSymmetricMatrixPtr Protein::getResidueResidueDistanceMatrixCa() const
  {return getObject(T("ResidueResidueDistanceMatrixCa"));}

ScoreSymmetricMatrixPtr Protein::getResidueResidueContactMatrix8Cb() const
  {return getObject(T("ResidueResidueContactMatrix8Cb"));}

ScoreSymmetricMatrixPtr Protein::getResidueResidueDistanceMatrixCb() const
  {return getObject(T("ResidueResidueDistanceMatrixCb"));}

CartesianCoordinatesSequencePtr Protein::getCAlphaTrace() const
  {return getObject(T("CAlphaTrace"));}

BondCoordinatesSequencePtr Protein::getCAlphaBondSequence() const
  {return getObject(T("CAlphaBondSequence"));}

ProteinBackboneBondSequencePtr Protein::getBackboneBondSequence() const
  {return getObject(T("BackboneBondSequence"));}

ProteinTertiaryStructurePtr Protein::getTertiaryStructure() const
  {return getObject(T("TertiaryStructure"));}

void Protein::computePropertiesFrom(const std::vector< ScalarSequencePtr >& aaindex)
{
  ScoreVectorSequencePtr properties = new ScoreVectorSequence(T("AminoAcidProperty"), AminoAcidPropertyDictionary::getInstance(), getLength(), aaindex.size());
  LabelSequencePtr sequence = getAminoAcidSequence();
  for (size_t i = 0; i < aaindex.size(); ++i)
  {
    for (size_t j = 0; j < getLength(); ++j)
    {
      properties->setScore(j, i, aaindex[i]->getValue(sequence->getIndex(j)));
    }
  }
  
  setObject(properties);
  
  /*
  static const double kiteDoolittle[24] = {0.7, 0, 0.11, 0.11, 0.78, 0.11, 0.11, 0.46, 0.14, 1, 0.92, 0.07, 0.71, 0.81, 0.32, 0.41, 0.42, 0.4, 0.36, 0.97, 0.11, 0.11, 0.96, 0.5};
  static const double hoppWoods[24] = {0.45, 1, 0.56, 1, 0.38, 1, 0.56, 0.53, 0.45, 0.25, 0.25, 1, 0.33, 0.14, 0.53, 0.58, 0.47, 0, 0.17, 0.3, 0.78, 0.78, 0.25, 0.5};
  static const double cornette[24] = {0.38, 0.51, 0.3, 0, 0.82, 0.15, 0.03, 0.35, 0.41, 0.9, 1, 0, 0.83, 0.85, 0.1, 0.3, 0.14, 0.47, 0.72, 0.89, 0.65, 0.09, 0.95, 0.5};
  static const double eisenberg[24] = {0.81, 0, 0.45, 0.42, 0.72, 0.46, 0.43, 0.77, 0.54, 1, 0.92, 0.26, 0.81, 0.95, 0.68, 0.6, 0.63, 0.85, 0.71, 0.92, 0.435, 0.445, 0.96, 0.5};
  static const double rose[24] = {0.56, 0.31, 0.28, 0.26, 1, 0.26, 0.26, 0.51, 0.67, 0.92, 0.85, 0, 0.85, 0.92, 0.31, 0.36, 0.46, 0.85, 0.62, 0.87, 0.27, 0.26, 0.885, 0.5};
  static const double janin[24] = {0.78, 0.15, 0.48, 0.44, 1, 0.41, 0.41, 0.78, 0.63, 0.93, 0.85, 0, 0.81, 0.85, 0.56, 0.63, 0.59, 0.78, 0.52, 0.89, 0.46, 0.41, 0.89, 0.5};
  static const double engelman[24] = {0.87, 0, 0.47, 0.19, 0.89, 0.26, 0.51, 0.83, 0.58, 0.96, 0.94, 0.22, 0.98, 1, 0.76, 0.81, 0.84, 0.89, 0.73, 0.93, 0.34, 0.385, 0.95, 0.5};
  static const double russellLinding[24] = {0.1656, 0.2532, 0.6723, 0.6699, 0.4337, 0.2242, 0.2418, 0.8829, 0.4340, 0.0000, 0.0869, 0.3322, 0.2024, 0.2027, 1.0000, 0.5826, 0.4444, 0.1844, 0.2213, 0.0371, 0.67, 0.23, 0.04, 0.5};
  static const double deleageRoux[24] = {0.2362, 0.2889, 0.6499, 0.6420, 0.3182, 0.2365, 0.3569, 0.7534, 0.4612, 0.1045, 0.1465, 0.3599, 0.1257, 0.1144, 1.0000, 0.5498, 0.4667, 0.2461, 0.4324, 0.0000, 0.645, 0.285, 0.12};
  
  jassert(getAminoAcidProperty() == ObjectPtr());
  
  ScoreVectorSequencePtr properties = new ScoreVectorSequence(T("AminoAcidProperty"), AminoAcidPropertyDictionary::getInstance(), getLength());
  
  LabelSequencePtr sequence = getAminoAcidSequence();
  
  for (size_t i = 0; i < getLength(); ++i) {
    properties->setScore(i, 0, kiteDoolittle[sequence->getIndex(i)]);
    properties->setScore(i, 1, hoppWoods[sequence->getIndex(i)]);
    properties->setScore(i, 2, cornette[sequence->getIndex(i)]);
    properties->setScore(i, 3, eisenberg[sequence->getIndex(i)]);
    properties->setScore(i, 4, rose[sequence->getIndex(i)]);
    properties->setScore(i, 5, janin[sequence->getIndex(i)]);
    properties->setScore(i, 6, engelman[sequence->getIndex(i)]);
    properties->setScore(i, 7, russellLinding[sequence->getIndex(i)]);
    properties->setScore(i, 8, deleageRoux[sequence->getIndex(i)]);
  }
  
  setObject(properties);
  */
}

bool Protein::load(InputStream& istr)
{
  int versionNumber;
  if (!lbcpp::read(istr, versionNumber))
    return false;
  if (versionNumber != 101)
  {
    Object::error(T("Protein::load"), T("Unrecognized version number"));
    return false;
  }
  return StringToObjectMap::load(istr);
}

void Protein::save(OutputStream& ostr) const
{
  int versionNumber = 101;
  lbcpp::write(ostr, versionNumber);
  StringToObjectMap::save(ostr);
}
