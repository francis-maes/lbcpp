/*-----------------------------------------.---------------------------------.
| Filename: ConvertProteinsToNewFormat.cpp | Protein Converter               |
| Author  : Francis Maes                   |                                 |
| Started : 09/07/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Protein/ProteinObject.h" // old
#include "Data/Protein.h" // new
using namespace lbcpp;

extern void declareProteinClasses();

ObjectContainerPtr loadProteins(const File& directory, size_t maxCount = 0)
{
  ObjectStreamPtr proteinsStream = directoryObjectStream(directory, T("*.protein"));
#ifdef JUCE_DEBUG
  ObjectContainerPtr res = proteinsStream->load(maxCount ? maxCount : 7)->randomize();
#else
  ObjectContainerPtr res = proteinsStream->load(maxCount)->randomize();
#endif
  for (size_t i = 0; i < res->size(); ++i)
  {
    ProteinObjectPtr protein = res->getAndCast<ProteinObject>(i);
    jassert(protein);
    protein->computeMissingFields();
  }
  return res;
}

static VectorPtr convertLabelSequence(LabelSequencePtr sequence, EnumerationPtr targetType)
{
  if (!sequence)
    return VectorPtr();
  size_t n = sequence->size();
  VectorPtr res = new Vector(targetType, n);
  for (size_t i = 0; i < n; ++i)
    if (sequence->hasObject(i))
      res->setVariable(i, Variable((int)sequence->getIndex(i), targetType));
  return res;
}

static VectorPtr convertScoreVectorSequence(ScoreVectorSequencePtr sequence, EnumerationPtr targetType)
{
  if (!sequence)
    return VectorPtr();

  size_t n = sequence->size();
  size_t numScores = sequence->getNumScores();

  VectorPtr res = new Vector(discreteProbabilityDistributionClass(targetType), n);
  for (size_t i = 0; i < n; ++i)
    if (sequence->hasObject(i))
    {
      DiscreteProbabilityDistributionPtr distribution = new DiscreteProbabilityDistribution(targetType);
      for (size_t j = 0; j < numScores; ++j)
        distribution->setVariable(j, sequence->getScore(i, j));
      res->setVariable(i, distribution);
    }
  return res;
}

static VectorPtr convertScalarSequenceToProbabilityVector(ScalarSequencePtr sequence)
{
  if (!sequence)
    return VectorPtr();
  size_t n = sequence->size();

  VectorPtr res = new Vector(probabilityType(), n);
  for (size_t i = 0; i < n; ++i)
    if (sequence->hasValue(i))
    {
      double value = sequence->getValue(i);
      jassert(value >= 0 && value <= 1);
      res->setVariable(i, Variable(value, probabilityType()));
    }
  return res;
}

static VectorPtr convertBinaryLabelSequenceToProbabilityVector(LabelSequencePtr sequence)
{
  if (!sequence)
    return VectorPtr();

  jassert(sequence->getDictionary() == BinaryClassificationDictionary::getInstance());
  size_t n = sequence->size();

  VectorPtr res = new Vector(probabilityType(), n);
  for (size_t i = 0; i < n; ++i)
    if (sequence->hasObject(i))
    {
      size_t value = sequence->getIndex(i);
      jassert(value == 0 || value == 1);
      res->setVariable(i, Variable(value ? 1.0 : 0.0, probabilityType()));
    }
  return res;
}

static CartesianPositionVectorPtr convertCartesianPositionVector(CartesianCoordinatesSequencePtr sequence)
{
  if (!sequence)
    return CartesianPositionVectorPtr();

  size_t n = sequence->size();
  CartesianPositionVectorPtr res = new CartesianPositionVector(n);
  for (size_t i = 0; i < n; ++i)
    if (sequence->hasObject(i))
      res->setPosition(i, sequence->getPosition(i));
  return res;
}

static AtomPtr convertAtom(ProteinAtomPtr atom)
{
  if (!atom)
    return AtomPtr();

  AtomPtr res = new Atom(atom->getName(), atom->getElementSymbol(), new Vector3(atom->getPosition()));
  res->setOccupancy(res->getOccupancy());
  res->setTemperatureFactor(res->getTemperatureFactor());
  return res;
}

static ResiduePtr convertResidue(ProteinResidueAtomsPtr residue)
{
  if (!residue)
    return ResiduePtr();

  size_t n = residue->getNumAtoms();
  ResiduePtr res = new Residue((AminoAcidType)residue->getAminoAcid());
  for (size_t i = 0; i < n; ++i)
    res->addAtom(convertAtom(residue->getAtom(i)));
  return res;
}

static TertiaryStructurePtr convertTertiaryStructure(ProteinTertiaryStructurePtr tertiaryStructure)
{
  if (!tertiaryStructure)
    return TertiaryStructurePtr();

  size_t n = tertiaryStructure->size();
  TertiaryStructurePtr res = new TertiaryStructure(n);
  for (size_t i = 0; i < n; ++i)
    res->setResidue(i, convertResidue(tertiaryStructure->getResidue(i)));
  return res;
}

static SymmetricMatrixPtr convertScoreSymmetricMatrix(ScoreSymmetricMatrixPtr matrix, TypePtr elementsType)
{
  if (!matrix)
    return SymmetricMatrixPtr();

  size_t n = matrix->getDimension();
  SymmetricMatrixPtr res = new SymmetricMatrix(elementsType, n);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = 0; j <= i; ++j)
      if (matrix->hasScore(i, j))
        res->setElement(i, j, Variable(matrix->getScore(i, j), elementsType));
  return res;
}

static ProteinPtr convertProtein(ProteinObjectPtr protein)
{
  ProteinPtr res = new Protein(protein->getName());
  res->setPrimaryStructure(convertLabelSequence(protein->getAminoAcidSequence(), aminoAcidTypeEnumeration()));
  res->setPositionSpecificScoringMatrix(convertScoreVectorSequence(protein->getPositionSpecificScoringMatrix(), aminoAcidTypeEnumeration()));

  res->setSecondaryStructure(convertLabelSequence(protein->getSecondaryStructureSequence(), secondaryStructureElementEnumeration()));
  res->setDSSPSecondaryStructure(convertLabelSequence(protein->getDSSPSecondaryStructureSequence(), dsspSecondaryStructureElementEnumeration()));
  res->setStructuralAlphabetSequence(convertLabelSequence(protein->getStructuralAlphabetSequence(), structuralAlphaElementEnumeration()));

  res->setSolventAccessibility(convertScalarSequenceToProbabilityVector(protein->getNormalizedSolventAccessibilitySequence()));
  res->setSolventAccessibilityAt20p(convertBinaryLabelSequenceToProbabilityVector(protein->getSolventAccessibilityThreshold20()));

  if (protein->getDisorderProbabilitySequence())
    res->setDisorderRegions(convertScalarSequenceToProbabilityVector(protein->getDisorderProbabilitySequence()));
  else
    res->setDisorderRegions(convertBinaryLabelSequenceToProbabilityVector(protein->getDisorderSequence()));

  res->setContactMap(convertScoreSymmetricMatrix(protein->getResidueResidueContactMatrix8Ca(), probabilityType()), 8.0, false);
  res->setContactMap(convertScoreSymmetricMatrix(protein->getResidueResidueContactMatrix8Cb(), probabilityType()), 8.0, true);
  res->setDistanceMap(convertScoreSymmetricMatrix(protein->getResidueResidueDistanceMatrixCa(), angstromDistanceType()), false);
  res->setDistanceMap(convertScoreSymmetricMatrix(protein->getResidueResidueDistanceMatrixCb(), angstromDistanceType()), true);

  res->setCAlphaTrace(convertCartesianPositionVector(protein->getCAlphaTrace()));
  res->setTertiaryStructure(convertTertiaryStructure(protein->getTertiaryStructure()));

  //res->computeMissingVariables();
  return res;
}

static VectorPtr convertProteins(ObjectContainerPtr oldStyleProteins, const File& outputDirectory)
{
  VectorPtr proteins = new Vector(proteinClass(), oldStyleProteins->size());
  for (size_t i = 0; i < proteins->size(); ++i)
  {
    ProteinObjectPtr oldStyleProtein = oldStyleProteins->getAndCast<ProteinObject>(i);
    if (!oldStyleProtein)
      continue;

    std::cout << "[" << i << "] Converting protein " << oldStyleProtein->getName() << "..." << std::flush;
    jassert(oldStyleProtein);
    ProteinPtr protein = convertProtein(oldStyleProtein);
    jassert(protein);
    proteins->setVariable(i, protein);
    std::cout << " saving... " << protein->getName() << std::flush;
    Variable(protein).saveToFile(outputDirectory.getChildFile(protein->getName() + T(".xml")));
    std::cout << " ok" << std::endl;
  }
  return proteins;
}


int main(int argc, char** argv)
{
  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " inputDirectory outputDirectory" << std::endl;
    return 1;
  }
  File workingDirectory = File::getCurrentWorkingDirectory();
  File inputDirectory = workingDirectory.getChildFile(argv[1]);
  if (!inputDirectory.isDirectory())
  {
    std::cerr << argv[1] << " is not a directory." << std::endl;
    return 1;
  }
  File outputDirectory = workingDirectory.getChildFile(argv[2]);
  if (!outputDirectory.exists())
    outputDirectory.createDirectory();

  lbcpp::initialize();
  declareProteinClasses();
  ObjectContainerPtr oldStyleProteins = loadProteins(inputDirectory);
  std::cout << oldStyleProteins->size() << " proteins" << std::endl;
  
  // convert proteins
  convertProteins(oldStyleProteins, outputDirectory);
  return 0;
}
