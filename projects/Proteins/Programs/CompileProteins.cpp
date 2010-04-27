/*-----------------------------------------.---------------------------------.
| Filename: CompileProteins.cpp            | Merge proteins with pssms and   |
| Author  : Francis Maes                   |  dssp files                     |
| Started : 22/04/2010 17:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../Protein/Protein.h"
#include "../Protein/Formats/PSSMFileParser.h"
#include "../Protein/Formats/DSSPFileParser.h"
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
  LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
  jassert(aminoAcidSequence);
  ObjectPtr pssm = ObjectStreamPtr(new PSSMFileParser(pssmFile, aminoAcidSequence))->next(); 
  if (!pssm)
    return false;
  protein->setObject(pssm);
  return true;
}

bool loadDSSPFile(ProteinPtr protein, const File& dsspFile)
  {return ObjectStreamPtr(new DSSPFileParser(dsspFile, protein))->next();}

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

bool compileProtein(const std::vector<File>& inputFiles, const File& outputFile)
{
  std::cout << "Compile " << outputFile.getFileNameWithoutExtension() << "..." << std::endl;

  int proteinIndex = -1;
  for (size_t i = 0; i < inputFiles.size(); ++i)
    if (inputFiles[i].getFileName().endsWith(T(".protein")))
      proteinIndex = (int)i;

  if (proteinIndex < 0)
  {
    Object::error(T("compileProtein"), T("No input protein"));
    return false;
  }

  ProteinPtr protein = Protein::createFromFile(inputFiles[proteinIndex]);
  if (!protein)
  {
    Object::error(T("compileProtein"), T("Could not load protein"));
    return false;
  }
  
  for (size_t i = 0; i < inputFiles.size(); ++i)
    if (i != (size_t)proteinIndex)
      if (!loadProteinRelatedFile(protein, inputFiles[i]))
        return false;

  protein->saveToFile(outputFile);
  return true;
}

bool compileProtein(const File& inputFile, const std::vector<File>& otherInputDirectories, const File& outputDirectory)
{
  std::vector<File> inputFiles;
  inputFiles.push_back(inputFile);
  for (size_t i = 0; i < otherInputDirectories.size(); ++i)
  {
    File f = findMatchingFileInOtherDirectory(inputFile, otherInputDirectories[i]);
    if (!f.exists())
      return false;
    inputFiles.push_back(f);
  }
  File outputFile = outputDirectory.getChildFile(inputFile.getFileNameWithoutExtension() + T(".protein"));
  return compileProtein(inputFiles, outputFile);
}

void compileProteins(const File& mainInputDirectory, const std::vector<File>& otherInputDirectories, const File& outputDirectory)
{
  OwnedArray<File> inputs;
  mainInputDirectory.findChildFiles(inputs, File::findFiles, false, T("*.protein"));
  size_t numSuccess = 0;
  for (int i = 0; i < inputs.size(); ++i)
    if (compileProtein(*inputs[i], otherInputDirectories, outputDirectory))
      ++numSuccess;
  std::cout << "Succeed to compile " << numSuccess << " / " << inputs.size() << " proteins." << std::endl;
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
  if (outputDirectory.exists() && !outputDirectory.isDirectory())
  {
    std::cerr << argv[argc - 1] << " is not a directory" << std::endl;
    return 1;
  }
  if (!outputDirectory.exists() && !outputDirectory.createDirectory())
  {
    std::cerr << "Could not create directory " << argv[argc - 1] << std::endl;
    return 1;
  }
 
  compileProteins(mainInputDirectory, otherInputDirectories, outputDirectory);
  return 0;
}
