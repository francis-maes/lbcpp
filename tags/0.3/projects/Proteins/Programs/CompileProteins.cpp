/*-----------------------------------------.---------------------------------.
| Filename: CompileProteins.cpp            | Merge proteins with pssms and   |
| Author  : Francis Maes                   |  dssp files                     |
| Started : 22/04/2010 17:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../Data/Protein.h"
#include "../Data/Formats/PSSMFileParser.h"
#include "../Data/Formats/DSSPFileParser.h"
using namespace lbcpp;

extern void declareProteinClasses();

using juce::OwnedArray;

File findMatchingFileInOtherDirectory(const File& inputFile, const File& otherDirectory, ErrorHandler& callback)
{
  OwnedArray<File> res;
  otherDirectory.findChildFiles(res, File::findFiles, false, inputFile.getFileNameWithoutExtension() + T(".*"));
  if (res.size() == 0)
  {
    callback.errorMessage(T("findMatchingFileInOtherDirectory"),
      T("Could not find matching of ") + inputFile.getFileNameWithoutExtension() + T(" in ") + otherDirectory.getFullPathName());
    return File::nonexistent;
  }
  if (res.size() > 1)
  {
    callback.errorMessage(T("findMatchingFileInOtherDirectory"),
      T("More of one matching files of ") + inputFile.getFileNameWithoutExtension() + T(" in ") + otherDirectory.getFullPathName());
    return File::nonexistent;
  }
  return *res[0];
}

bool loadPSSMFile(ProteinPtr protein, const File& pssmFile, ErrorHandler& callback)
{
  VectorPtr primaryStructure = protein->getPrimaryStructure();
  jassert(primaryStructure);
  VectorPtr pssm = StreamPtr(new PSSMFileParser(pssmFile, primaryStructure, callback))->next().getObjectAndCast<Vector>(); 
  if (!pssm)
    return false;
  protein->setPositionSpecificScoringMatrix(pssm);
  return true;
}

bool loadDSSPFile(ProteinPtr protein, const File& dsspFile, ErrorHandler& callback)
  {StreamPtr(new DSSPFileParser(dsspFile, protein, callback))->next(); return true;}

bool loadProteinRelatedFile(ProteinPtr protein, const File& file, ErrorHandler& callback)
{
  String ext = file.getFileExtension();
  if (ext == T(".pssm"))
    return loadPSSMFile(protein, file, callback);
  else if (ext == T(".dssp"))
    return loadDSSPFile(protein, file, callback);
  else
  {
    callback.errorMessage(T("loadProteinRelatedFile"), T("Unrecognized extension: ") + ext);
    return false;
  }
}

bool compileProtein(const std::vector<File>& inputFiles, const File& outputFile, ErrorHandler& callback)
{
  std::cout << "Compile " << outputFile.getFileNameWithoutExtension() << "..." << std::endl;

  int proteinIndex = -1;
  for (size_t i = 0; i < inputFiles.size(); ++i)
    if (inputFiles[i].getFileName().endsWith(T(".xml")))
      proteinIndex = (int)i;

  if (proteinIndex < 0)
  {
    callback.errorMessage(T("compileProtein"), T("No input protein"));
    return false;
  }

  ProteinPtr protein = Protein::createFromXml(inputFiles[proteinIndex], callback);
  if (!protein)
  {
    callback.errorMessage(T("compileProtein"), T("Could not load protein"));
    return false;
  }
  
  for (size_t i = 0; i < inputFiles.size(); ++i)
    if (i != (size_t)proteinIndex)
      if (!loadProteinRelatedFile(protein, inputFiles[i], callback))
        return false;

  protein->saveToXmlFile(outputFile);
  return true;
}

bool compileProtein(const File& inputFile, const std::vector<File>& otherInputDirectories, const File& outputDirectory, ErrorHandler& callback)
{
  std::vector<File> inputFiles;
  inputFiles.push_back(inputFile);
  for (size_t i = 0; i < otherInputDirectories.size(); ++i)
  {
    File f = findMatchingFileInOtherDirectory(inputFile, otherInputDirectories[i], callback);
    if (!f.exists())
      return false;
    inputFiles.push_back(f);
  }
  File outputFile = outputDirectory.getChildFile(inputFile.getFileNameWithoutExtension() + T(".xml"));
  return compileProtein(inputFiles, outputFile, callback);
}

void compileProteins(const File& mainInputDirectory, const std::vector<File>& otherInputDirectories, const File& outputDirectory, ErrorHandler& callback)
{
  OwnedArray<File> inputs;
  mainInputDirectory.findChildFiles(inputs, File::findFiles, false, T("*.xml"));
  size_t numSuccess = 0;
  for (int i = 0; i < inputs.size(); ++i)
    if (compileProtein(*inputs[i], otherInputDirectories, outputDirectory, callback))
      ++numSuccess;
  std::cout << "Succeed to compile " << numSuccess << " / " << inputs.size() << " proteins." << std::endl;
}

int main(int argc, char* argv[])
{
  lbcpp::initialize();
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
 
  compileProteins(mainInputDirectory, otherInputDirectories, outputDirectory, ErrorHandler::getInstance());
  return 0;
}