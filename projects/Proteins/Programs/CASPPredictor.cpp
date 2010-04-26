/*-----------------------------------------.---------------------------------.
| Filename: CASPPredictor.cpp              | CASP Predictor                  |
| Author  : Francis Maes                   |                                 |
| Started : 17/04/2010 14:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../Protein/Protein.h"
#include "../Protein/AminoAcidDictionary.h"
#include "../Protein/SecondaryStructureDictionary.h"
#include "../Protein/Formats/CASPFileGenerator.h"
using namespace lbcpp;

extern void declareProteinClasses();

void addDefaultPredictions(ProteinPtr protein)
{
  size_t n = protein->getLength();

  LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
  jassert(aminoAcidSequence);

  /*
  ** Order-disorder region
  */
  ScalarSequencePtr orderDisorderScoreSequence = protein->createEmptyObject(T("DisorderProbabilitySequence"));
  for (size_t i = 0; i < n; ++i)
  {
    double disorderProbability = RandomGenerator::getInstance().sampleDouble();
    orderDisorderScoreSequence->setValue(i, disorderProbability);
  }
  protein->setObject(orderDisorderScoreSequence);

  /*
  ** Residue-residue contact
  */
  ScoreSymmetricMatrixPtr rrContactMatrix = 
    new ScoreSymmetricMatrix(T("ResidueResidueContactProbabilityMatrix"), n, 0.5);
  protein->setObject(rrContactMatrix);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i + 1; j < n; ++j)
      rrContactMatrix->setScore(i, j, RandomGenerator::getInstance().sampleDouble());

  /*
  ** Tertiary structure
  */
  CartesianCoordinatesSequencePtr trace = new CartesianCoordinatesSequence(T("CAlphaTrace"), n);
  for (size_t i = 0; i < n; ++i)
  {
    static const double constantLength = 3.8;
    trace->setPosition(i, Vector3(i * constantLength, 0.0, 0.0));
  }
  protein->setObject(trace);
  protein->setObject(ProteinTertiaryStructure::createFromCAlphaTrace(aminoAcidSequence, trace));
}

int main(int argc, char* argv[])
{
  declareProteinClasses();

  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " fastaFile pssmFile" << std::endl;
    return 1;
  }
  File cwd = File::getCurrentWorkingDirectory();
  File fastaFile = cwd.getChildFile(argv[1]);
  File pssmFile = cwd.getChildFile(argv[1]);
  
  File outputDirectory = fastaFile.getParentDirectory();
  String outputBaseName = fastaFile.getFileNameWithoutExtension();
  
  std::cout << "FASTA File: " << fastaFile.getFullPathName() << " PSSM File: " << pssmFile.getFullPathName() << std::endl;
  ProteinPtr protein = Protein::createFromFASTA(fastaFile);
  if (!protein)
  {
    std::cerr << "Could not load FASTA file" << std::endl;
    return 1;
  }
  
  std::cout << "Target Name: " << protein->getName() << std::endl;
  std::cout << "Amino Acid Sequence: " << protein->getAminoAcidSequence()->toString() << std::endl;

  std::cout << "Making predictions ..." << std::endl;
  addDefaultPredictions(protein);
  std::cout << std::endl;
  //std::cout << "===========================" << std::endl << protein->toString() << std::endl;
  
  String method = T("This files contains a default prediction. No prediction methods are applied yet.\nWe have to quickly develop our code !!!");

//  caspTertiaryStructureFileGenerator     (cwd.getChildFile(outputBaseName + T(".TS")), method)->consume(protein);
  File rrFile = outputDirectory.getChildFile(outputBaseName + T(".rr"));
  std::cout << "Write residue-residue distance file " << rrFile.getFullPathName() << std::endl;
  caspResidueResidueDistanceFileGenerator(rrFile, method)->consume(protein);
  
  File drFile = outputDirectory.getChildFile(outputBaseName + T(".dr"));
  std::cout << "Write Disorder region prediction file " << drFile.getFullPathName() << std::endl;
  caspOrderDisorderRegionFileGenerator(drFile, method)->consume(protein);

  File pdbFile = outputDirectory.getChildFile(outputBaseName + T(".pdbca"));
  std::cout << "Write C-alpha chain in PDB file " << pdbFile.getFullPathName() << std::endl;
  protein->saveToPDBFile(pdbFile);
  return 0;
}
