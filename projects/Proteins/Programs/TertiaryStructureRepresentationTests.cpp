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

int main()
{
  declareProteinClasses();

  File proteinDatabase(T("C:\\Projets\\LBC++\\projects\\temp\\SmallPDB\\protein"));
  ObjectStreamPtr proteinsStream = directoryObjectStream(proteinDatabase, T("*.protein"));
  ObjectContainerPtr proteins = proteinsStream->load(1)->randomize();
  std::cout << proteins->size() << " proteins." << std::endl;
  if (!proteins)
    return 1;

  File ramachadranPlotFile(T("C:\\Projets\\LBC++\\projects\\temp\\ramac.txt"));
  ramachadranPlotFile.deleteRecursively();
  OutputStream* ramachadranPlot = ramachadranPlotFile.createOutputStream();
  jassert(ramachadranPlot);



  static ScalarVariableStatistics nCalphaLength(T("N--CA length"));
  static ScalarVariableStatistics calphaCLength(T("CA--C length")); 
  static ScalarVariableStatistics cnLength(T("C--N length")); 
  static ScalarVariableStatistics calphaCalphaLength(T("CA--CA length")); 

  static ScalarVariableStatistics calphaAngle(T("N--CA--C angle"));
  static ScalarVariableStatistics carbonAngle(T("CA--C--N' angle"));
  static ScalarVariableStatistics nitrogenAngle(T("C--N'--CA' angle"));

  for (size_t index = 0; index < proteins->size(); ++index)
  {
    ProteinPtr protein = proteins->getAndCast<Protein>(index);
    std::cout << "Protein " << protein->getName() << std::endl;

    ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
    jassert(tertiaryStructure);
    if (tertiaryStructure->hasOnlyCAlphaAtoms())
      continue;

    LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();

    ProteinBackboneBondSequencePtr backbone = tertiaryStructure->createBackbone();
    ProteinTertiaryStructurePtr tertiaryStructure2 = ProteinTertiaryStructure::createFromBackbone(aminoAcidSequence, backbone);
    ProteinBackboneBondSequencePtr backbone2 = tertiaryStructure2->createBackbone();
    
    for (size_t i = protein->getLength() - 20; i < protein->getLength(); ++i)
    {
      std::cout << (i+1) << "Correct: " << backbone->getBond(i)->toString() << std::endl
        << " Reconstructed: " << backbone2->getBond(i)->toString() << std::endl;
    }

    protein->setObject(tertiaryStructure2);
    protein->saveToPDBFile(File(T("C:/Projets/LBC++/projects/temp/pouet.pdb")));
    break;

    for (size_t i = 0; i < tertiaryStructure->size(); ++i)
    {
      ProteinResiduePtr residue = tertiaryStructure->getResidue(i);

      Vector3 nitrogen = residue->getNitrogenAtom()->getPosition();
      Vector3 calpha = residue->getCAlphaAtom()->getPosition();
      Vector3 carbon = residue->getCarbonAtom()->getPosition();

      nCalphaLength.push((calpha - nitrogen).l2norm());
      calphaCLength.push((carbon - calpha).l2norm());
      ProteinResiduePtr nextResidue = i < tertiaryStructure->size() - 1 ? tertiaryStructure->getResidue(i + 1) : ProteinResiduePtr();
      if (nextResidue)
      {
        Vector3 nextNitrogen = nextResidue->getNitrogenAtom()->getPosition();
        cnLength.push((nextNitrogen - carbon).l2norm());

        Vector3 nextCalpha = nextResidue->getCAlphaAtom()->getPosition();
        calphaCalphaLength.push((nextCalpha - calpha).l2norm());

        carbonAngle.push((carbon - calpha).angle(nextNitrogen - carbon));
        nitrogenAngle.push((nextNitrogen - carbon).angle(nextCalpha - nextNitrogen));
      }

      calphaAngle.push((calpha - nitrogen).angle(carbon - calpha));
    }

    //for (size_t i = 1; i < dihedralAngles->size() - 1; ++i)
    //  (*ramachadranPlot) << lbcpp::toString(dihedralAngles->getPhi(i)) << " " << lbcpp::toString(dihedralAngles->getPsi(i)) << "\n";
  }

  std::cout << nCalphaLength.toString() << std::endl;
  std::cout << calphaCLength.toString() << std::endl;
  std::cout << cnLength.toString() << std::endl;
  std::cout << calphaCalphaLength.toString() << std::endl;
  std::cout << calphaAngle.toString() << std::endl;
  std::cout << carbonAngle.toString() << std::endl;
  std::cout << nitrogenAngle.toString() << std::endl;
  delete ramachadranPlot; 
  return 0;
}
