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
  ** Amino Acid Sequence - Reduced Alphabet
  */
  LabelSequencePtr reducedAlphabetSequence = getObject(T("ReducedAminoAcidAlphabetSequence"));
  // J.Y. Yang and M.Q. Yang - Predicting protein disorder by analyzing amino acid sequence - BMC Genomics 2008
  static const AminoAcidDictionary::Type alphabetModel1[AminoAcidDictionary::unknown + 1] = {
    AminoAcidDictionary::alanine,
    AminoAcidDictionary::histidine,
    AminoAcidDictionary::asparagine,
    AminoAcidDictionary::asparticAcid,
    AminoAcidDictionary::cysteine,
    AminoAcidDictionary::asparticAcid,
    AminoAcidDictionary::asparagine,
    AminoAcidDictionary::alanine,
    AminoAcidDictionary::histidine,
    AminoAcidDictionary::isoleucine,
    AminoAcidDictionary::isoleucine,
    AminoAcidDictionary::histidine,
    AminoAcidDictionary::methionine,
    AminoAcidDictionary::phenylalanine,
    AminoAcidDictionary::alanine,
    AminoAcidDictionary::serine,
    AminoAcidDictionary::serine,
    AminoAcidDictionary::phenylalanine,
    AminoAcidDictionary::phenylalanine,
    AminoAcidDictionary::isoleucine,
    AminoAcidDictionary::asparticAcid,
    AminoAcidDictionary::asparticAcid,
    AminoAcidDictionary::isoleucine,
    AminoAcidDictionary::unknown
  };
  // Lynne Reed Murphy et al. - Simplified amino  acid alphabets for protein fold recognition and implications for folding - Protein Engineering 2000
  // Three groups
  static const AminoAcidDictionary::Type alphabetModel2[AminoAcidDictionary::unknown + 1] = {
    AminoAcidDictionary::alanine,          // A  -  Ala
    AminoAcidDictionary::glutamicAcid,         // R  -  Arg
    AminoAcidDictionary::glutamicAcid,       // N  -  Asn
    AminoAcidDictionary::glutamicAcid,     // D  -  Asp
    AminoAcidDictionary::alanine,         // C  -  Cys
    AminoAcidDictionary::glutamicAcid,     // E  -  Glu
    AminoAcidDictionary::glutamicAcid,        // Q  -  Gln
    AminoAcidDictionary::alanine,          // G  -  Gly
    AminoAcidDictionary::glutamicAcid,        // H  -  His
    AminoAcidDictionary::alanine,       // I  -  Ile
    AminoAcidDictionary::alanine,          // L  -  Leu
    AminoAcidDictionary::glutamicAcid,           // K  -  Lys
    AminoAcidDictionary::alanine,       // M  -  Met
    AminoAcidDictionary::phenylalanine,    // F  -  Phe
    AminoAcidDictionary::alanine,          // P  -  Pro
    AminoAcidDictionary::alanine,           // S  -  Ser
    AminoAcidDictionary::alanine,        // T  -  Thr
    AminoAcidDictionary::phenylalanine,       // W  -  Trp
    AminoAcidDictionary::phenylalanine,         // Y  -  Tyr
    AminoAcidDictionary::alanine,           // V  -  Val
    AminoAcidDictionary::unknown,
    AminoAcidDictionary::unknown,
    AminoAcidDictionary::unknown,
    AminoAcidDictionary::unknown
  };
  // Five groups
  static const AminoAcidDictionary::Type alphabetModel3[AminoAcidDictionary::unknown + 1] = {
    AminoAcidDictionary::alanine,          // A  -  Ala
    AminoAcidDictionary::histidine,         // R  -  Arg
    AminoAcidDictionary::asparticAcid,       // N  -  Asn
    AminoAcidDictionary::asparticAcid,     // D  -  Asp
    AminoAcidDictionary::cysteine,         // C  -  Cys
    AminoAcidDictionary::asparticAcid,     // E  -  Glu
    AminoAcidDictionary::asparticAcid,        // Q  -  Gln
    AminoAcidDictionary::alanine,          // G  -  Gly
    AminoAcidDictionary::histidine,        // H  -  His
    AminoAcidDictionary::cysteine,       // I  -  Ile
    AminoAcidDictionary::cysteine,          // L  -  Leu
    AminoAcidDictionary::histidine,           // K  -  Lys
    AminoAcidDictionary::cysteine,       // M  -  Met
    AminoAcidDictionary::phenylalanine,    // F  -  Phe
    AminoAcidDictionary::alanine,          // P  -  Pro
    AminoAcidDictionary::alanine,           // S  -  Ser
    AminoAcidDictionary::alanine,        // T  -  Thr
    AminoAcidDictionary::phenylalanine,       // W  -  Trp
    AminoAcidDictionary::phenylalanine,         // Y  -  Tyr
    AminoAcidDictionary::cysteine,           // V  -  Val
    AminoAcidDictionary::unknown,
    AminoAcidDictionary::unknown,
    AminoAcidDictionary::unknown,
    AminoAcidDictionary::unknown
  };
  // Six groups
  static const AminoAcidDictionary::Type alphabetModel4[AminoAcidDictionary::unknown + 1] = {
    AminoAcidDictionary::alanine,      // A  -  Ala
    AminoAcidDictionary::lysine,         // R  -  Arg
    AminoAcidDictionary::asparticAcid,       // N  -  Asn
    AminoAcidDictionary::asparticAcid,     // D  -  Asp
    AminoAcidDictionary::cysteine,         // C  -  Cys
    AminoAcidDictionary::asparticAcid,     // E  -  Glu
    AminoAcidDictionary::asparticAcid,        // Q  -  Gln
    AminoAcidDictionary::alanine,          // G  -  Gly
    AminoAcidDictionary::cysteine,        // H  -  His
    AminoAcidDictionary::isoleucine,       // I  -  Ile
    AminoAcidDictionary::isoleucine,          // L  -  Leu
    AminoAcidDictionary::lysine,           // K  -  Lys
    AminoAcidDictionary::isoleucine,       // M  -  Met
    AminoAcidDictionary::phenylalanine,    // F  -  Phe
    AminoAcidDictionary::cysteine,          // P  -  Pro
    AminoAcidDictionary::alanine,           // S  -  Ser
    AminoAcidDictionary::alanine,        // T  -  Thr
    AminoAcidDictionary::phenylalanine,       // W  -  Trp
    AminoAcidDictionary::phenylalanine,         // Y  -  Tyr
    AminoAcidDictionary::isoleucine,           // V  -  Val
    AminoAcidDictionary::unknown,
    AminoAcidDictionary::unknown,
    AminoAcidDictionary::unknown,
    AminoAcidDictionary::unknown
  };
  // C. Etchebest et al. - A reduced amino acid alphabet for understanding and designing protein adaptation to mutation - European Biophysics Journal 2007
  static const AminoAcidDictionary::Type alphabetModel5[AminoAcidDictionary::unknown + 1] = {
    AminoAcidDictionary::alanine,      // A  -  Ala
    AminoAcidDictionary::alanine,         // R  -  Arg
    AminoAcidDictionary::cysteine,       // N  -  Asn
    AminoAcidDictionary::cysteine,     // D  -  Asp
    AminoAcidDictionary::cysteine,         // C  -  Cys
    AminoAcidDictionary::alanine,     // E  -  Glu
    AminoAcidDictionary::alanine,        // Q  -  Gln
    AminoAcidDictionary::glycine,          // G  -  Gly
    AminoAcidDictionary::cysteine,        // H  -  His
    AminoAcidDictionary::phenylalanine,       // I  -  Ile
    AminoAcidDictionary::alanine,          // L  -  Leu
    AminoAcidDictionary::alanine,           // K  -  Lys
    AminoAcidDictionary::alanine,       // M  -  Met
    AminoAcidDictionary::phenylalanine,    // F  -  Phe
    AminoAcidDictionary::proline,          // P  -  Pro
    AminoAcidDictionary::cysteine,           // S  -  Ser
    AminoAcidDictionary::cysteine,        // T  -  Thr
    AminoAcidDictionary::phenylalanine,       // W  -  Trp
    AminoAcidDictionary::phenylalanine,         // Y  -  Tyr
    AminoAcidDictionary::phenylalanine,           // V  -  Val
    AminoAcidDictionary::unknown,
    AminoAcidDictionary::unknown,
    AminoAcidDictionary::unknown,
    AminoAcidDictionary::unknown
  };
  
  static const AminoAcidDictionary::Type* aminoAcidMap = alphabetModel2;
  
  if (!reducedAlphabetSequence)
  {
//    std::cout << "Reduced alphabet: Model 5" << std::endl;
    reducedAlphabetSequence = new LabelSequence(T("ReducedAminoAcidAlphabetSequence"), AminoAcidDictionary::getInstance(), getLength());
    for (size_t i = 0; i < getLength(); ++i)
      reducedAlphabetSequence->setIndex(i, aminoAcidMap[aminoAcidSequence->getIndex(i)]);
    setObject(reducedAlphabetSequence);
//    std::cout << "AA: " << aminoAcidSequence->toString() << std::endl;
//    std::cout << "RA1:" << reducedAlphabetSequence->toString() << std::endl;
  }

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
    setObject(residueResidueContactMatrix8Ca = residueResidueDistanceMatrixCa->makeProbabilityMatrix(T("ResidueResidueContactMatrix8Ca"), 8.0));
  if (residueResidueDistanceMatrixCb && !residueResidueContactMatrix8Cb)
    setObject(residueResidueContactMatrix8Cb = residueResidueDistanceMatrixCb->makeProbabilityMatrix(T("ResidueResidueContactMatrix8Cb"), 8.0));

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

  LabelSequencePtr structuralAlphabetSequence = getObject(T("StructuralAlphabetSequence"));
  if (calphaTrace && !structuralAlphabetSequence)
  {
    structuralAlphabetSequence = new LabelSequence(T("StructuralAlphabetSequence"), StructuralAlphabetDictionary::getInstance(), getLength());  
    
    for (size_t i = 3; i < calphaTrace->size(); ++i)
    {
      Vector3ObjectPtr v3o_a = static_cast<Vector3ObjectPtr>(calphaTrace->get(i - 3));
      Vector3ObjectPtr v3o_b = static_cast<Vector3ObjectPtr>(calphaTrace->get(i - 2));
      Vector3ObjectPtr v3o_c = static_cast<Vector3ObjectPtr>(calphaTrace->get(i - 1));
      Vector3ObjectPtr v3o_d = static_cast<Vector3ObjectPtr>(calphaTrace->get(i));
      
      if (!v3o_a || !v3o_b || !v3o_c || !v3o_d)
        continue;
      
      Vector3 a = v3o_a->getValue();
      Vector3 b = v3o_b->getValue();
      Vector3 c = v3o_c->getValue();
      Vector3 d = v3o_d->getValue();
      
      double d1 = (a - c).l2norm();
      double d2 = (a - d).l2norm();
      double d3 = (b - d).l2norm();
      
      Vector3 normalVector =  (b - a).crossProduct(c - a);
      
      double d4 = (normalVector.dotProduct(d) - normalVector.dotProduct(a)) / normalVector.l2norm();
      
      size_t bestGroup = 0;
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

      structuralAlphabetSequence->setIndex(i, bestGroup);
      //std::cout << d1 << "\t" << d2 << "\t" << d3 << "\t" << d4 << "\t\t" << bestGroup << std::endl;
    }
  
    setObject(structuralAlphabetSequence);
    //std::cout << "SAS:" << structuralAlphabetSequence->toString() << std::endl;
  }
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
  else if (name == T("StructuralAlphabetSequence"))
    return new LabelSequence(name, StructuralAlphabetDictionary::getInstance(), n);
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

LabelSequencePtr Protein::getReducedAminoAcidAlphabetSequence() const
  {return getObject(T("ReducedAminoAcidAlphabetSequence"));}

LabelSequencePtr Protein::getStructuralAlphabetSequence() const
  {return getObject(T("StructuralAlphabetSequence"));}

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

std::vector<LabelSequencePtr>& Protein::getLabelSequences()
{
  sequences.clear();
//  sequences.push_back(getAminoAcidSequence());
  
  SequencePtr toAdd;
  toAdd = getStructuralAlphabetSequence();
  if (toAdd)
    sequences.push_back(toAdd);
  
  toAdd = getSecondaryStructureSequence();
  if (toAdd)
    sequences.push_back(toAdd);
  
  toAdd = getDSSPSecondaryStructureSequence();
  if (toAdd)
    sequences.push_back(toAdd);
  
  toAdd = getSolventAccessibilityThreshold20();
  if (toAdd)
    sequences.push_back(toAdd);
  
  toAdd = getDisorderSequence();
  if (toAdd)
    sequences.push_back(toAdd);
  
  return sequences;
}
