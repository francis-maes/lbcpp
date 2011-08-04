/*-----------------------------------------.---------------------------------.
| Filename: CASPPredictor.cpp              | CASP Predictor                  |
| Author  : Francis Maes                   |                                 |
| Started : 17/04/2010 14:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../Protein/ProteinObject.h"
#include "../Protein/AminoAcidDictionary.h"
#include "../Protein/SecondaryStructureDictionary.h"
#include "../Protein/Inference/ProteinInference.h"

#include "../Data/Formats/CASPFileGenerator.h"
#include "../Data/Formats/PSSMFileParser.h"
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
    double disorderProbability = RandomGenerator::getInstance().sampleDouble();
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
#endif // 0

void displayObjectIfExists(ProteinObjectPtr protein, const String& objectName)
{
  ObjectPtr object = protein->getObject(objectName);
  if (object)
    std::cout << object->getName() << ": " << object->toString() << std::endl;
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
  ProteinObjectPtr protein = ProteinObject::createFromFASTA(fastaFile);
  if (!protein)
  {
    std::cerr << "Could not load FASTA file" << std::endl;
    return 1;
  }

  std::cout << "Target Name: " << protein->getName() << std::endl;
  LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
  std::cout << "Amino Acid Sequence: " << aminoAcidSequence->toString() << std::endl;

  // FIXME
  ScoreVectorSequencePtr pssm;// = (new PSSMFileParser(pssmFile, aminoAcidSequence))->next().getObjectAndCast<ScoreVectorSequence>();
  if (!pssm || pssm->size() != aminoAcidSequence->size())
  {
    std::cerr << "Could not load PSSM file" << std::endl;
    return 1;
  }
  protein->setObject(pssm);
  //  std::cout << "Loaded pssm: " << pssm->toString() << std::endl;

  std::cout << "Model file: " << modelFile.getFullPathName() << std::endl;
  ProteinInferencePtr inference = Inference::createFromFileAndCast<ProteinInference>(modelFile);
  if (!inference)
  {
    std::cerr << "Could not load model" << std::endl;
    return 1;
  }
  std::cout << "Loaded " << inference->getNumSubInferences() << " inference steps" << std::endl;
  
  std::cout << "Making predictions ..." << std::endl;

  Inference::ReturnCode returnCode = Inference::finishedReturnCode;
  protein = singleThreadedInferenceContext()->runInference(inference, protein, ObjectPtr(), returnCode).dynamicCast<ProteinObject>();
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
  protein->computeMissingFields();

  //addDefaultPredictions(protein);
  //std::cout << std::endl;
  //std::cout << "===========================" << std::endl << protein->toString() << std::endl;
  
  String method = T("The model used to perform these predictions is a multi-task sequence labeling model. Here, only two tasks are taken into account: solvent accesibility prediction and disorder region prediction. The model used here is SA_REG_300 with the six first passes (3 passes of SA and 3 passes of DR)");

  int numFilesGenerated = 0;
  if (protein->getResidueResidueContactMatrix8Cb())
  {
    File rrFile = outputDirectory.getChildFile(outputBaseName + T(".rr"));
    std::cout << "Write residue-residue distance file " << rrFile.getFullPathName() << std::endl;
    caspResidueResidueDistanceFileGenerator(rrFile, method)->consume(protein);
    ++numFilesGenerated;
  }

  displayObjectIfExists(protein, T("SecondaryStructureSequence"));
  displayObjectIfExists(protein, T("DSSPSecondaryStructureSequence"));
  displayObjectIfExists(protein, T("SolventAccessibilityThreshold20"));
  displayObjectIfExists(protein, T("DisorderProbabilitySequence"));
  
  if (protein->getDisorderProbabilitySequence())
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