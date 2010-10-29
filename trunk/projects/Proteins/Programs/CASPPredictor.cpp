/*-----------------------------------------.---------------------------------.
| Filename: CASPPredictor.cpp              | CASP Predictor                  |
| Author  : Francis Maes                   |                                 |
| Started : 17/04/2010 14:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>

#include "../Data/Formats/CASPFileGenerator.h"
#include "../Data/Formats/PSSMFileParser.h"
#include "Inference/ProteinInference.h"
using namespace lbcpp;

extern void declareProteinClasses();

#if 0
void addDefaultPredictions(ProteinObjectPtr protein)
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
    double disorderProbability = RandomGenerator::getInstance()->sampleDouble();
    orderDisorderScoreSequence->setValue(i, disorderProbability);
  }
  protein->setObject(orderDisorderScoreSequence);

  /*
  ** Residue-residue contact
  */
  ScoreSymmetricMatrixPtr rrContactMatrix = protein->createEmptyObject(T("ResidueResidueContactMatrix8Cb"));
  protein->setObject(rrContactMatrix);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i + 1; j < n; ++j)
      rrContactMatrix->setScore(i, j, RandomGenerator::getInstance()->sampleDouble());

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
#endif // 0


void displayVectorIfExists(String friendlyName, VectorPtr vector)
{
  if (vector)
    std::cout << friendlyName << ": " << vector->toString() << std::endl;
}

int main(int argc, char* argv[])
{
  lbcpp::initialize();
  declareProteinClasses();

  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " fastaFile pssmFile [modelFile]" << std::endl;
    return 1;
  }
  File cwd = File::getCurrentWorkingDirectory();
  File fastaFile = cwd.getChildFile(argv[1]);
  File pssmFile = cwd.getChildFile(argv[2]);

  File modelFile;
  if (argc > 3)
    modelFile = cwd.getChildFile(argv[3]);
  else
  {
    File thisExeFile = cwd.getChildFile(argv[0]);
    modelFile = thisExeFile.getParentDirectory().getChildFile(T("protein.inference"));
  }

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
  VectorPtr primaryStructure = protein->getPrimaryStructure();
  std::cout << "Amino Acid Sequence: " << primaryStructure->toString() << std::endl;

  // FIXME
  VectorPtr pssm;// = (new PSSMFileParser(pssmFile, aminoAcidSequence))->next().getObjectAndCast<ScoreVectorSequence>();
  if (!pssm || pssm->getNumElements() != primaryStructure->getNumElements())
  {
    std::cerr << "Could not load PSSM file" << std::endl;
    return 1;
  }
  protein->setPositionSpecificScoringMatrix(pssm);
  //  std::cout << "Loaded pssm: " << pssm->toString() << std::endl;

  std::cout << "Model file: " << modelFile.getFullPathName() << std::endl;
  ProteinSequentialInferencePtr inference = Variable::createFromFile(modelFile).getObjectAndCast<ProteinSequentialInference>();
  if (!inference)
  {
    std::cerr << "Could not load model" << std::endl;
    return 1;
  }
  std::cout << "Loaded " << inference->getNumSubInferences() << " inference steps" << std::endl;
  
  std::cout << "Making predictions ..." << std::endl;

  Inference::ReturnCode returnCode = Inference::finishedReturnCode;
  protein = singleThreadedInferenceContext()->run(inference, protein, ObjectPtr(), returnCode).dynamicCast<Protein>();
  if (returnCode != Inference::finishedReturnCode)
  {
    std::cerr << "Invalid return code in inference" << std::endl;
    return 1;
  }
  if (!protein)
  {
    std::cerr << "Could not complete inference" << std::endl;
    return 1;
  }

  //addDefaultPredictions(protein);
  //std::cout << std::endl;
  //std::cout << "===========================" << std::endl << protein->toString() << std::endl;
  
  String method = T("The model used to perform these predictions is a multi-task sequence labeling model. Here, only two tasks are taken into account: solvent accesibility prediction and disorder region prediction. The model used here is SA_REG_300 with the six first passes (3 passes of SA and 3 passes of DR)");

  int numFilesGenerated = 0;
  if (protein->getContactMap())
  {
    File rrFile = outputDirectory.getChildFile(outputBaseName + T(".rr"));
    std::cout << "Write residue-residue distance file " << rrFile.getFullPathName() << std::endl;
    caspResidueResidueDistanceFileGenerator(rrFile, method)->consume(protein);
    ++numFilesGenerated;
  }

  displayVectorIfExists(T("SecondaryStructure"), protein->getSecondaryStructure());
  displayVectorIfExists(T("DSSPSecondaryStructure"), protein->getDSSPSecondaryStructure());
  displayVectorIfExists(T("SolventAccessibilityThreshold20"), protein->getSolventAccessibilityAt20p());
  displayVectorIfExists(T("DisorderProbability"), protein->getDisorderRegions());
  
  if (protein->getDisorderRegions())
  {
    File drFile = outputDirectory.getChildFile(outputBaseName + T(".dr"));
    std::cout << "Write Disorder region prediction file " << drFile.getFullPathName() << std::endl;
    caspOrderDisorderRegionFileGenerator(drFile, method)->consume(protein);
    ++numFilesGenerated;
  }

  if (protein->getCAlphaTrace())
  {
    File pdbFile = outputDirectory.getChildFile(outputBaseName + T(".pdbca"));
    std::cout << "Write C-alpha chain in PDB file " << pdbFile.getFullPathName() << std::endl;
    protein->saveToPDBFile(pdbFile);
    ++numFilesGenerated;
  }

  if (!numFilesGenerated)
  {
    std::cerr << "Not any predicted files" << std::endl;
    return 1;
  }

  std::cout << "Generated " << numFilesGenerated << " file(s)" << std::endl;
  return 0;
}
