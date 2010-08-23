/*-----------------------------------------.---------------------------------.
| Filename: ConvertPDBToProtein.cpp        | Convert PDB to ProteinObject          |
| Author  : Francis Maes                   |                                 |
| Started : 20/04/2010 11:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../Data/Protein.h"
using namespace lbcpp;

extern void declareProteinClasses();

bool convertPDBToProtein(const File& inputFile, const File& outputFile)
{
  std::cout << inputFile.getFullPathName() << "..." << std::endl;
  ProteinPtr protein = Protein::createFromPDB(inputFile, false);
  if (!protein)
    return false;
 
  File output = outputFile;
  if (output.isDirectory())
  {
    output = output.getChildFile(inputFile.getFileNameWithoutExtension() + T(".xml"));
  }
  //OutputStream* o = output.createOutputStream();
  //*o << "> " << inputFile.getFileNameWithoutExtension() << "\n";
  //*o << protein->getPrimaryStructure()->toString();
  //delete o;
  Variable(protein).saveToFile(output);
  return true;
}

bool convertProteinToPDB(const File& inputFile, const File& outputFile)
{
  std::cout << inputFile.getFullPathName() << "..." << std::endl;
  ProteinPtr protein = Protein::createFromXml(inputFile);
  if (!protein)
    return false;
  
  File output = outputFile;
  if (output.isDirectory())
  {
    output = output.getChildFile(inputFile.getFileNameWithoutExtension() + T(".pdb"));
  }
  protein->saveToPDBFile(output);
  return true;
}


int main(int argc, char* argv[])
{
  lbcpp::initialize();
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
  bool convertToPDB = false;
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
    
    if (!inputFiles.size())
    {
      input.findChildFiles(inputFiles, File::findFiles, false, T("*.xml"));
      convertToPDB = true;
    }
    
  }
  else
  {
    if (input.getFileExtension() == T(".xml"))
      convertToPDB = true;
    inputFiles.add(new File(input));
  }
  std::vector<File> errors;
  for (int i = 0; i < inputFiles.size(); ++i)
  {
    if (inputFiles.size() > 1)
      std::cout << (i+1) << " / " << inputFiles.size() << ", ";

    if (convertToPDB && !convertProteinToPDB(*inputFiles[i], output))
        errors.push_back(*inputFiles[i]);
    else if (!convertToPDB && !convertPDBToProtein(*inputFiles[i], output))
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
