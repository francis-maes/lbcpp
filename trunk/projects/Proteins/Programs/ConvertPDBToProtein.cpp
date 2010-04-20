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

bool convert(const File& inputFile, const File& outputFile)
{
  std::cout << "Parsing " << inputFile.getFullPathName() << "..." << std::endl;
  ProteinPtr protein = Protein::createFromPDB(inputFile);
  if (!protein)
    return false;
 
  File output = outputFile;
  if (output.isDirectory())
    output = output.getChildFile(outputFile.getFileNameWithoutExtension() + T(".protein"));

  std::cout << "Saving " << output.getFullPathName() << "..." << std::endl;
  protein->saveToFile(output);
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

  for (int i = 0; i < inputFiles.size(); ++i)
    if (!convert(*inputFiles[i], output))
      return 3;

  return 0;
}
