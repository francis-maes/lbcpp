/*-----------------------------------------.---------------------------------.
| Filename: ConvertPDBToProtein.cpp        | Convert PDB to Protein          |
| Author  : Francis Maes                   |                                 |
| Started : 20/04/2010 11:20               |                                 |
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

void testUnPeuLeBazar(ProteinPtr protein)
{
  ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
  jassert(tertiaryStructure);
  if (tertiaryStructure->hasOnlyCAlphaAtoms())
    return;

  static ScalarVariableStatistics nCalphaLength(T("N--CA length"));
  static ScalarVariableStatistics calphaCLength(T("CA--C length")); 
  static ScalarVariableStatistics cnLength(T("C--N length")); 

  for (size_t i = 0; i < tertiaryStructure->size(); ++i)
  {
    ProteinResiduePtr residue = tertiaryStructure->getResidue(i);
    nCalphaLength.push(residue->getDistanceBetweenAtoms(T("N"), T("CA")));
    calphaCLength.push(residue->getDistanceBetweenAtoms(T("CA"), T("C")));
    ProteinResiduePtr nextResidue = i < tertiaryStructure->size() - 1 ? tertiaryStructure->getResidue(i + 1) : ProteinResiduePtr();
    if (nextResidue)
    {
      double d = residue->getDistanceBetweenAtoms(T("C"), nextResidue, T("N"));
      jassert(d < 2.0);
      cnLength.push(d);
    }
  }

  std::cout << nCalphaLength.toString() << std::endl;
  std::cout << calphaCLength.toString() << std::endl;
  std::cout << cnLength.toString() << std::endl;
}


bool convert(const File& inputFile, const File& outputFile)
{
  std::cout << inputFile.getFullPathName() << "..." << std::endl;
  ProteinPtr protein = Protein::createFromPDB(inputFile);
  if (!protein)
    return false;
 
  File output = outputFile;
  if (output.isDirectory())
    output = output.getChildFile(inputFile.getFileNameWithoutExtension() + T(".protein"));

  testUnPeuLeBazar(protein);

  //protein->saveToFile(output);
  return true;
}

int main(int argc, char* argv[])
{
  declareProteinClasses();

  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " inputFile outputFile/outputDirectory" << std::endl;
    std::cerr << " or  : " << argv[0] << " inputDirectory outputDirectory" << std::endl;
    return 1;
  }
  File cwd = File::getCurrentWorkingDirectory();
  File input = cwd.getChildFile(argv[1]);
  File output = cwd.getChildFile(argv[2]);

  if (!input.exists())
  {
    std::cerr << "Error: " << input.getFullPathName() << " does not exists." << std::endl;
    return 2;
  }

  juce::OwnedArray<File> inputFiles;
  if (input.isDirectory())
  {
    if (output.exists() && !output.isDirectory())
    {
      std::cerr << "Error: " << output.getFullPathName() << " is not a directory." << std::endl;
      return 2;
    }
    if (!output.createDirectory())
    {
      std::cerr << "Error: could not create directory " << output.getFullPathName() << "." << std::endl;
      return 2;    
    }
    input.findChildFiles(inputFiles, File::findFiles, false, T("*.pdb"));
  }
  else
    inputFiles.add(new File(input));

  std::vector<File> errors;
  for (int i = 0; i < inputFiles.size(); ++i)
  {
    if (inputFiles.size() > 1)
      std::cout << (i+1) << " / " << inputFiles.size() << ", ";
    if (!convert(*inputFiles[i], output))
      errors.push_back(*inputFiles[i]);
  }

  if (errors.size())
  {
    std::cout << "Could not parse " << errors.size() << " / " << inputFiles.size() << " files:" << std::endl;
    for (size_t i = 0; i < errors.size(); ++i)
      std::cout << "\t" << errors[i].getFullPathName() << std::endl;
    return 3;
  }
  return 0;
}
