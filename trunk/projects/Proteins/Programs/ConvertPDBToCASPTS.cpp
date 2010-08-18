/*-----------------------------------------.---------------------------------.
| Filename: ConvertPDBToCASPTS.cpp         | Convert a PDB file into a       |
| Author  : Francis Maes                   |  CASP Tertiary structure        |
| Started : 21/04/2010 15:36               |     prediction file.            |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../Data/Protein.h"
#include "../Data/Formats/CASPFileGenerator.h"
using namespace lbcpp;

extern void declareProteinLibrary();

int main(int argc, char* argv[])
{
  lbcpp::initialize();
  declareProteinLibrary();
  
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " pdbFile" << std::endl;
    return 1;
  }
  File cwd = File::getCurrentWorkingDirectory();
  File pdbFile = cwd.getChildFile(argv[1]);
  std::cout << "Input PDB File: " << pdbFile.getFullPathName() << std::endl;

  ProteinPtr protein = Protein::createFromPDB(pdbFile);
  if (!protein)
  {
    std::cerr << "Could not load pdb file" << std::endl;
    return 1;
  }
  protein->setName(pdbFile.getFileNameWithoutExtension());
  std::cout << "Target Name: " << protein->getName() << std::endl;
  File outputFile = pdbFile.getParentDirectory().getChildFile(pdbFile.getFileNameWithoutExtension() + T(".ts"));
  std::cout << "Output File: " << outputFile.getFullPathName() << std::endl;

  String method = T("This files contains a default prediction. No prediction methods are applied yet.\nWe have to quickly develop our code !!!");
  caspTertiaryStructureFileGenerator(outputFile, method)->consume(protein);
  return 0;
}
