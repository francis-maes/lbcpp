/*-----------------------------------------.---------------------------------.
| Filename: TertiaryStructureRepresen...cpp| Test conversions between ts repr|
| Author  : Francis Maes                   |                                 |
| Started : 20/04/2010 18:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../Protein/Protein.h"
#include "../Protein/AminoAcidDictionary.h"
#include "../Protein/SecondaryStructureDictionary.h"
using namespace lbcpp;

extern void declareProteinClasses();

void printConfidenceInterval(ScalarVariableStatistics& stats)
  {std::cout << stats.getMean() - 3.0 * stats.getStandardDeviation() << " ... " << stats.getMean() + 3.0 * stats.getStandardDeviation() << std::endl;}

int main()
{
  lbcpp::initialize();
  declareProteinClasses();

  {
    File inputDirectory(T("C:\\Projets\\LBC++\\projects\\temp\\SmallPDB\\proteinWithSS3DR"));
    File outputDirectory(T("C:\\Projets\\LBC++\\projects\\temp\\SmallPDB\\smallProteinWithSS3DR"));
    ObjectStreamPtr inputProteins = directoryObjectStream(inputDirectory, T("*.protein"));
    while (!inputProteins->isExhausted())
    {
      ProteinPtr protein = inputProteins->nextAndCast<Protein>();
      jassert(protein);
      if (protein->getLength() < 50)
      {
        std::cout << protein->getName() << ": " << protein->getLength() << " amino acids" << std::endl;
        protein->saveToFile(outputDirectory.getChildFile(protein->getName() + T(".protein")));
      }
    }
  }
  return 0;

  /*  
  File inputFile(T("C:\\Projets\\LBC++\\projects\\temp\\1E4T_correct.pdb"));
  File outputFile(T("C:\\Projets\\LBC++\\projects\\temp\\1E4T_pouet.pdb"));

  ProteinPtr protein = Protein::createFromPDB(inputFile);
  jassert(protein);
  LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
  ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
  jassert(tertiaryStructure->hasCompleteBackbone());
  ProteinBackboneBondSequencePtr backbone = tertiaryStructure->makeBackbone();
  protein->setObject(ProteinTertiaryStructure::createFromBackbone(aminoAcidSequence, backbone));
  protein->saveToPDBFile(outputFile);
  return 0;*/
  

  File proteinDatabase(T("C:\\Projets\\LBC++\\projects\\temp\\SmallPDB\\protein"));
  ObjectStreamPtr proteinsStream = directoryObjectStream(proteinDatabase, T("*.protein"));
  ObjectContainerPtr proteins = proteinsStream->load()->randomize();
  std::cout << proteins->size() << " proteins." << std::endl;
  if (!proteins)
    return 1;

  static ScalarVariableStatistics cacaLength(T("CA--CA length"));
  static ScalarVariableStatistics cacacaAngle(T("CA--CA--CA angle"));
  static ScalarVariableStatistics cacacacaDihedral(T("CA--CA--CA--CA dihedral"));

  for (size_t index = 0; index < proteins->size(); ++index)
  {
    ProteinPtr protein = proteins->getAndCast<Protein>(index);
    std::cout << "Protein " << protein->getName() << std::endl;
    protein->computeMissingFields();
    CartesianCoordinatesSequencePtr calphaTrace = protein->getCAlphaTrace();
    BondCoordinatesSequencePtr calphaTraceBonds = new BondCoordinatesSequence(T("yo"), calphaTrace);
    for (size_t i = 0; i < calphaTraceBonds->size(); ++i)
    {
      BondCoordinates bond = calphaTraceBonds->getCoordinates(i);
      if (bond.hasLength())
        cacaLength.push(bond.getLength());
      if (bond.hasThetaAngle())
        cacacaAngle.push(bond.getThetaAngle());
      if (bond.hasPhiDihedralAngle())
        cacacacaDihedral.push(bond.getPhiDihedralAngle());
    }
  }

  std::cout << cacaLength.toString() << std::endl; printConfidenceInterval(cacaLength);
  std::cout << std::endl << cacacaAngle.toString() << std::endl; printConfidenceInterval(cacacaAngle);
  std::cout << std::endl << cacacacaDihedral.toString() << std::endl; printConfidenceInterval(cacacacaDihedral);
  return 0;
/*
  File ramachadranPlotFile(T("C:\\Projets\\LBC++\\projects\\temp\\ramac.txt"));
  ramachadranPlotFile.deleteRecursively();
  OutputStream* ramachadranPlot = ramachadranPlotFile.createOutputStream();
  jassert(ramachadranPlot);*/

  static ScalarVariableStatistics nCalphaLength(T("N--CA length"));
  static ScalarVariableStatistics calphaCLength(T("CA--C length")); 
  static ScalarVariableStatistics cnLength(T("C--N length")); 

  static ScalarVariableStatistics calphaAngle(T("N--CA--C angle"));
  static ScalarVariableStatistics carbonAngle(T("CA--C--N' angle"));
  static ScalarVariableStatistics nitrogenAngle(T("C--N'--CA' angle"));

  for (size_t index = 0; index < proteins->size(); ++index)
  {
    ProteinPtr protein = proteins->getAndCast<Protein>(index);
    std::cout << "Protein " << protein->getName() << std::endl;

    ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
    jassert(tertiaryStructure);
    if (!tertiaryStructure->hasCompleteBackbone())
      continue;

    LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
    size_t n = aminoAcidSequence->size();

    ProteinBackboneBondSequencePtr backbone = tertiaryStructure->makeBackbone();
    for (size_t i = 0; i < n; ++i)
    {
      ProteinBackboneBondPtr bond = backbone->getBond(i);
      if (bond)
      {
        if (bond->getBond1().hasLength())
          nCalphaLength.push(bond->getBond1().getLength());
        if (bond->getBond2().hasLength())
          calphaCLength.push(bond->getBond2().getLength());
        if (bond->getBond3().hasLength())
          cnLength.push(bond->getBond3().getLength());

        if (bond->getBond1().hasThetaAngle())
          calphaAngle.push(bond->getBond1().getThetaAngle());
        if (bond->getBond2().hasThetaAngle())
          carbonAngle.push(bond->getBond2().getThetaAngle());
        if (bond->getBond3().hasThetaAngle())
          nitrogenAngle.push(bond->getBond3().getThetaAngle());
      }
    }

/*    ProteinTertiaryStructurePtr tertiaryStructure2 = ProteinTertiaryStructure::createFromBackbone(aminoAcidSequence, backbone);

    std::cout << "RMSE = " << tertiaryStructure2->computeCAlphaAtomsRMSE(tertiaryStructure) << std::endl;

    ProteinBackboneBondSequencePtr backbone2 = tertiaryStructure2->makeBackbone();
    for (size_t i = n - 20; i < n; ++i)
    {
      std::cout << (i+1) << "Correct: " << backbone->getBond(i)->toString() << std::endl
        << " Reconstructed: " << backbone2->getBond(i)->toString() << std::endl;
    }

    protein->setObject(tertiaryStructure2);
    protein->saveToPDBFile(File(T("C:/Projets/LBC++/projects/temp/pouet.pdb")));
    break;*/
  }

  std::cout << nCalphaLength.toString() << std::endl;
  std::cout << calphaCLength.toString() << std::endl;
  std::cout << cnLength.toString() << std::endl;
  std::cout << calphaAngle.toString() << std::endl;
  std::cout << carbonAngle.toString() << std::endl;
  std::cout << nitrogenAngle.toString() << std::endl;
  //delete ramachadranPlot; 
  return 0;
}
