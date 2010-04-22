/*-----------------------------------------.---------------------------------.
| Filename: CompileProteins.cpp            | Merge proteins with pssms and   |
| Author  : Francis Maes                   |  dssp files                     |
| Started : 22/04/2010 17:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../ProteinInference/Protein.h"
using namespace lbcpp;

extern void declareProteinClasses();

using juce::OwnedArray;

File findMatchingFileInOtherDirectory(const File& inputFile, const File& otherDirectory)
{
  OwnedArray<File> res;
  otherDirectory.findChildFiles(res, File::findFiles, false, inputFile.getFileNameWithoutExtension() + T(".*"));
  if (res.size() == 0)
  {
    Object::error(T("findMatchingFileInOtherDirectory"),
      T("Could not find matching of ") + inputFile.getFileNameWithoutExtension() + T(" in ") + otherDirectory.getFullPathName());
    return File::nonexistent;
  }
  if (res.size() > 1)
  {
    Object::error(T("findMatchingFileInOtherDirectory"),
      T("More of one matching files of ") + inputFile.getFileNameWithoutExtension() + T(" in ") + otherDirectory.getFullPathName());
    return File::nonexistent;
  }
  return *res[0];
}

bool loadPSSMFile(ProteinPtr protein, const File& pssmFile)
{
  return true;
}

bool loadDSSPFile(ProteinPtr protein, const File& dsspFile)
{
  return true;
}

bool loadProteinRelatedFile(ProteinPtr protein, const File& file)
{
  String ext = file.getFileExtension();
  if (ext == T(".pssm"))
    return loadPSSMFile(protein, file);
  else if (ext == T(".dssp"))
    return loadDSSPFile(protein, file);
  else
  {
    Object::error(T("loadProteinRelatedFile"), T("Unrecognized extension: ") + ext);
    return false;
  }
}

void compileProtein(const std::vector<File>& inputFiles, const File& outputFile)
{
  int proteinIndex = -1;
  for (size_t i = 0; i < inputFiles.size(); ++i)
    if (inputFiles[i].getFileName().endsWith(T(".protein")))
      proteinIndex = (int)i;

  if (proteinIndex < 0)
  {
    Object::error(T("compileProtein"), T("No input protein"));
    return;
  }

  ProteinPtr protein = Protein::createFromFile(inputFiles[proteinIndex]);
  if (!protein)
  {
    Object::error(T("compileProtein"), T("Could not load protein"));
    return;
  }
  
  for (size_t i = 0; i < inputFiles.size(); ++i)
    if (i != (size_t)proteinIndex)
      if (!loadProteinRelatedFile(protein, inputFiles[i]))
        return;

  protein->saveToFile(outputFile);
}

void compileProtein(const File& inputFile, const std::vector<File>& otherInputDirectories, const File& outputDirectory)
{
  std::vector<File> inputFiles;
  inputFiles.push_back(inputFile);
  for (size_t i = 0; i < otherInputDirectories.size(); ++i)
  {
    File f = findMatchingFileInOtherDirectory(inputFile, otherInputDirectories[i]);
    if (!f.exists())
      return;
    inputFiles.push_back(f);
  }
  File outputFile = outputDirectory.getChildFile(inputFile.getFileNameWithoutExtension() + T(".protein"));
  compileProtein(inputFiles, outputFile);
}

void compileProteins(const File& mainInputDirectory, const std::vector<File>& otherInputDirectories, const File& outputDirectory)
{
  OwnedArray<File> inputs;
  mainInputDirectory.findChildFiles(inputs, File::findFiles, false, T("*.protein"));
  for (int i = 0; i < inputs.size(); ++i)
    compileProtein(*inputs[i], otherInputDirectories, outputDirectory);
}

int main(int argc, char* argv[])
{
  declareProteinClasses();

  if (argc < 4)
  {
    std::cout << "Usage: mainInputDirectory [otherInputDirectories]* outputDirectory" << std::endl;
    return 1;
  }

  File cwd = File::getCurrentWorkingDirectory();
  File mainInputDirectory = cwd.getChildFile(argv[1]);
  if (!mainInputDirectory.isDirectory())
  {
    std::cerr << argv[1] << " is not a directory" << std::endl;
    return 1;
  }

  std::vector<File> otherInputDirectories;
  for (int i = 2; i < argc - 1; ++i)
  {
    File f = cwd.getChildFile(argv[i]);
    if (!f.isDirectory())
    {
      std::cerr << argv[i] << " is not a directory" << std::endl;
      return 1;
    }
    otherInputDirectories.push_back(f);
  }


  File outputDirectory = cwd.getChildFile(argv[argc - 1]);
  if (!outputDirectory.isDirectory())
  {
    std::cerr << argv[argc - 1] << " is not a directory" << std::endl;
    return 1;
  }
 
  compileProteins(mainInputDirectory, otherInputDirectories, outputDirectory);
  return 0;
}
