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

extern void declareProteinClasses(ExecutionContext& context);

using juce::OwnedArray;

File findMatchingFileInOtherDirectory(ExecutionContext& context, const File& inputFile, const File& otherDirectory)
{
  OwnedArray<File> res;
  otherDirectory.findChildFiles(res, File::findFiles, false, inputFile.getFileNameWithoutExtension() + T(".*"));
  if (res.size() == 0)
  {
    context.errorCallback(T("findMatchingFileInOtherDirectory"),
      T("Could not find matching of ") + inputFile.getFileNameWithoutExtension() + T(" in ") + otherDirectory.getFullPathName());
    return File::nonexistent;
  }
  if (res.size() > 1)
  {
    context.errorCallback(T("findMatchingFileInOtherDirectory"),
      T("More of one matching files of ") + inputFile.getFileNameWithoutExtension() + T(" in ") + otherDirectory.getFullPathName());
    return File::nonexistent;
  }
  return *res[0];
}

bool loadPSSMFile(ExecutionContext& context, ProteinPtr protein, const File& pssmFile)
{
  VectorPtr primaryStructure = protein->getPrimaryStructure();
  jassert(primaryStructure);
  VectorPtr pssm = StreamPtr(new PSSMFileParser(context, pssmFile, primaryStructure))->next(context).getObjectAndCast<Vector>(context); 
  if (!pssm)
    return false;
  protein->setPositionSpecificScoringMatrix(pssm);
  return true;
}

bool loadDSSPFile(ExecutionContext& context, ProteinPtr protein, const File& dsspFile)
  {StreamPtr(new DSSPFileParser(context, dsspFile, protein))->next(context); return true;}

bool loadProteinRelatedFile(ExecutionContext& context, ProteinPtr protein, const File& file)
{
  String ext = file.getFileExtension();
  if (ext == T(".pssm"))
    return loadPSSMFile(context, protein, file);
  else if (ext == T(".dssp"))
    return loadDSSPFile(context, protein, file);
  else
  {
    context.errorCallback(T("loadProteinRelatedFile"), T("Unrecognized extension: ") + ext);
    return false;
  }
}

bool compileProtein(ExecutionContext& context, const std::vector<File>& inputFiles, const File& outputFile)
{
  context.informationCallback(T("Compile ") + outputFile.getFileNameWithoutExtension() + T("..."));

  int proteinIndex = -1;
  for (size_t i = 0; i < inputFiles.size(); ++i)
    if (inputFiles[i].getFileName().endsWith(T(".xml")))
      proteinIndex = (int)i;

  if (proteinIndex < 0)
  {
    context.errorCallback(T("compileProtein"), T("No input protein"));
    return false;
  }

  ProteinPtr protein = Protein::createFromXml(context, inputFiles[proteinIndex]);
  if (!protein)
  {
    context.errorCallback(T("compileProtein"), T("Could not load protein"));
    return false;
  }
  
  for (size_t i = 0; i < inputFiles.size(); ++i)
    if (i != (size_t)proteinIndex)
      if (!loadProteinRelatedFile(context, protein, inputFiles[i]))
        return false;

  protein->saveToXmlFile(context, outputFile);
  return true;
}

bool compileProtein(ExecutionContext& context, const File& inputFile, const std::vector<File>& otherInputDirectories, const File& outputDirectory)
{
  std::vector<File> inputFiles;
  inputFiles.push_back(inputFile);
  for (size_t i = 0; i < otherInputDirectories.size(); ++i)
  {
    File f = findMatchingFileInOtherDirectory(context, inputFile, otherInputDirectories[i]);
    if (!f.exists())
      return false;
    inputFiles.push_back(f);
  }
  File outputFile = outputDirectory.getChildFile(inputFile.getFileNameWithoutExtension() + T(".xml"));
  return compileProtein(context, inputFiles, outputFile);
}

void compileProteins(ExecutionContext& context, const File& mainInputDirectory, const std::vector<File>& otherInputDirectories, const File& outputDirectory)
{
  OwnedArray<File> inputs;
  mainInputDirectory.findChildFiles(inputs, File::findFiles, false, T("*.xml"));
  size_t numSuccess = 0;
  for (int i = 0; i < inputs.size(); ++i)
    if (compileProtein(context, *inputs[i], otherInputDirectories, outputDirectory))
      ++numSuccess;
  context.informationCallback(T("Succeed to compile ") + String((int)numSuccess) + T(" / ") + String((int)inputs.size()) + T(" proteins."));
}

int main(int argc, char* argv[])
{
  lbcpp::initialize();
  ExecutionContextPtr context = defaultConsoleExecutionContext();
  declareProteinClasses(*context);

  if (argc < 4)
  {
    std::cout << "Usage: mainInputDirectory [otherInputDirectories]* outputDirectory" << std::endl;
    std::cout << "   or: mainInputFile [inputDirectories]* outputDirectory" << std::endl;
    return 1;
  }


  File cwd = File::getCurrentWorkingDirectory();
  File mainInputDirectory = cwd.getChildFile(argv[1]);

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
 
  if (!mainInputDirectory.isDirectory())
  {
    if (compileProtein(*context, mainInputDirectory, otherInputDirectories, outputDirectory))
      std::cout << "Succeed to compile protein." << std::endl;
    else
      std::cout << "Could not compile protein." << std::endl;
  }
  else
  {
    compileProteins(*context, mainInputDirectory, otherInputDirectories, outputDirectory);
  }
  return 0;
}
