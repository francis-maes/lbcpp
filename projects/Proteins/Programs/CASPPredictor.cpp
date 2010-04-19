/*-----------------------------------------.---------------------------------.
| Filename: CASPPredictor.cpp              | CASP Predictor                  |
| Author  : Francis Maes                   |                                 |
| Started : 17/04/2010 14:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../ProteinInference/Protein.h"
#include "../ProteinInference/AminoAcidDictionary.h"
#include "../ProteinInference/SecondaryStructureDictionary.h"
#include "../ProteinInference/CASPFileGenerator.h"
#include "../ProteinInference/PDBFileGenerator.h"
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
  ScoreVectorSequencePtr orderDisorderScoreSequence =
    new ScoreVectorSequence(T("OrderDisorderScoreSequence"), OrderDisorderDictionary::getInstance(), n);
  for (size_t i = 0; i < n; ++i)
  {
    orderDisorderScoreSequence->setScore(i, OrderDisorderDictionary::order, 0.5);
    orderDisorderScoreSequence->setScore(i, OrderDisorderDictionary::disorder, 0.5);
  }
  protein->setObject(orderDisorderScoreSequence);

  /*
  ** Residue-residue contact
  */
  ScoreSymmetricMatrixPtr rrContactMatrix = 
    new ScoreSymmetricMatrix(T("ResidueResidueContactProbabilityMatrix"), n, 0.5);
  protein->setObject(rrContactMatrix);

  /*
  ** Tertiary structure
  */
  ProteinCarbonTracePtr trace = new ProteinCarbonTrace(T("CAlphaTrace"), n);
  for (size_t i = 0; i < n; ++i)
  {
    static const double constantLength = 2.0;
    trace->setPosition(i, Vector3(i * constantLength, 0.0, 0.0));
  }
  protein->setObject(trace);
  protein->setObject(ProteinTertiaryStructure::createFromCAlphaTrace(aminoAcidSequence, trace));
}

int main(int argc, char* argv[])
{
  declareProteinClasses();

  if (argc < 5)
  {
    std::cerr << "Usage: " << argv[0] << " modelDirectory targetName aminoAcidSequence outputBaseName" << std::endl;
    return 1;
  }
  File cwd = File::getCurrentWorkingDirectory();
  File modelDirectory = cwd.getChildFile(argv[1]);
  std::cout << "Model directory: " << modelDirectory.getFullPathName() << std::endl;
  String targetName = argv[2];
  std::cout << "Target Name: " << targetName << std::endl;
  String aminoAcidSequence = argv[3];
  std::cout << "Amino Acid Sequence: " << aminoAcidSequence << std::endl;
  String outputBaseName = argv[4];
  std::cout << "Output Base Name: " << outputBaseName << std::endl;

  ProteinPtr protein = Protein::createFromAminoAcidSequence(targetName, aminoAcidSequence);
 /* ProteinPtr protein = Protein::createFromPDB(File(T("C:\\Users\\Francis\\Downloads\\1crn.pdb")));
  if (!protein)
    return 2;

  ProteinCarbonTracePtr trace = ProteinCarbonTrace::createCAlphaTrace(protein->getTertiaryStructure());
  protein->setObject(trace);
  protein->setObject(ProteinTertiaryStructure::createFromCAlphaTrace(protein->getAminoAcidSequence(), trace));

  protein->saveToPDBFile(cwd.getChildFile(outputBaseName + T(".pdb")));
  return 0;*/

  addDefaultPredictions(protein);
  //std::cout << "===========================" << std::endl << protein->toString() << std::endl;
  
  String method = T("This files contains a default prediction. No prediction methods are applied yet.\nWe have to quickly develop our code !!!");

  caspTertiaryStructureFileGenerator     (cwd.getChildFile(outputBaseName + T(".TS")), method)->consume(protein);
  caspResidueResidueDistanceFileGenerator(cwd.getChildFile(outputBaseName + T(".RR")), method)->consume(protein);
  caspOrderDisorderRegionFileGenerator   (cwd.getChildFile(outputBaseName + T(".DR")), method)->consume(protein);

  protein->saveToPDBFile(cwd.getChildFile(outputBaseName + T(".PDB")));
  return 0;
}
