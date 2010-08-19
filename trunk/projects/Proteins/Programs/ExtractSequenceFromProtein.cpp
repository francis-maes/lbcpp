
#include <lbcpp/lbcpp.h>
#include "../Programs/ArgumentSet.h"
#include "Data/Protein.h"

using namespace lbcpp;

extern void declareProteinClasses();

//#define SEQUENCE
//#define FEATURES
//#define SHOW_PSSM
#define LENGTH_SELECTION

#ifdef SEQUENCE
int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();
  
  File proteinDirectory;
  File outputDirectory;
  
  ArgumentSet arguments;
  arguments.insert(new FileArgument(T("proteinDirectory"), proteinDirectory, true, true), true);
  arguments.insert(new FileArgument(T("outputDirectory"), outputDirectory, true, true), true);
  
  if (!arguments.parse(argv, 1, argc-1)) {
    std::cout << "Usage: " << argv[0] << " " << arguments.toString() << std::endl;
    return 1;
  }
  
  juce::OwnedArray<File> proteinFiles;
  proteinDirectory.findChildFiles(proteinFiles, File::findFiles, false, T("*.xml"));
  
  for (size_t i = 0; i < (size_t)proteinFiles.size(); ++i)
  {
    ProteinPtr protein = Protein::createFromXml(*proteinFiles[i]);
    File outputFile = outputDirectory.getChildFile(proteinFiles[i]->getFileNameWithoutExtension() + T(".seq"));
    
    std::cout << "Extracting primary structure of " << proteinFiles[i]->getFileNameWithoutExtension() << " ... ";

    OutputStream* o = outputFile.createOutputStream();
    *o << protein->getPrimaryStructure()->toString();
    delete o;
    
    std::cout << "OK" << std::endl;
  }
  
  return 0;
}
#endif

#ifdef LENGTH_SELECTION
int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();
  
  File proteinDirectory;
  File outputDirectory;
  int minimumLength = 0;
  int maximumLength = 50;
  
  ArgumentSet arguments;
  arguments.insert(new FileArgument(T("proteinDirectory"), proteinDirectory, true, true), true);
  arguments.insert(new FileArgument(T("outputDirectory"), outputDirectory, true, true), true);
  arguments.insert(new IntegerArgument(T("minimumLength"), minimumLength));
  arguments.insert(new IntegerArgument(T("maximumLength"), maximumLength));
  
  if (!arguments.parse(argv, 1, argc-1)) {
    std::cout << "Usage: " << argv[0] << " " << arguments.toString() << std::endl;
    return 1;
  }
  
  juce::OwnedArray<File> proteinFiles;
  proteinDirectory.findChildFiles(proteinFiles, File::findFiles, false, T("*.xml"));
  
  File outputFile = outputDirectory.getChildFile(T("L") + lbcpp::toString(minimumLength) + T("-") + lbcpp::toString(maximumLength) + T("DB"));
  outputFile.deleteFile();
  OutputStream* o = outputFile.createOutputStream();
  
  for (size_t i = 0; i < (size_t)proteinFiles.size(); ++i)
  {
    ProteinPtr protein = Protein::createFromXml(*proteinFiles[i]);

    if (minimumLength > (int)protein->getLength() ||  maximumLength < (int)protein->getLength())
      continue;

    
    std::cout << "Selecting " << proteinFiles[i]->getFileNameWithoutExtension() << " ... ";
    *o << protein->getName() << '\t' << lbcpp::toString(protein->getLength()) << '\n';
    std::cout << "OK" << std::endl;
  }
  delete o;
  
  return 0;
}
#endif

#ifdef SHOW_PSSM
int main(int argc, char** argv)
{
  declareProteinClasses();

  File inputFile;

  ArgumentSet arguments;
  arguments.insert(new FileArgument(T("input"), inputFile), true);

  if (!arguments.parse(argv, 1, argc-1)) {
    std::cout << "Usage: " << argv[0] << " " << arguments.toString() << std::endl;
    return 1;
  }

  ProteinPtr protein = Protein::createFromXml(inputFile);
  if (!protein)
  {
    std::cout << "Invalid protein !" << std::endl;
    return 1;
  }

  std::cout << protein->getPositionSpecificScoringMatrix()->toString() << std::endl;

  return 0;
}
#endif

#ifdef FEATURES
static void generateFeatures(ProteinPtr protein, int position, OutputStream* o)
{
  VectorPtr primaryStructure = protein->getPrimaryStructure();
  if (position < 0)
    *o << Variable::missingValue(aminoAcidTypeEnumeration()).getInteger();
  else if (position >= (int)protein->getLength())
    *o << Variable::missingValue(aminoAcidTypeEnumeration()).getInteger() + 1;
  else
    *o << primaryStructure->getVariable(position).getInteger();
  *o << " ";

  VectorPtr pssm = protein->getPositionSpecificScoringMatrix();
  if (position < 0 || position >= (int)protein->getLength())
    for (size_t i = 0; i < pssm->getNumVariables(); ++i)
      *o << "0 ";
  else
    for (size_t i = 0; i < pssm->getNumVariables(); ++i)
      *o << pssm->getObjectAndCast<DiscreteProbabilityDistribution>(position)->getVariable(i).getDouble() << " ";
}

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();

  size_t nbValidProteins = 0;
  size_t nbProteins = 0;
  size_t nbExamples = 0;
  size_t nbSkippedExamples = 0;
  
  File proteinDirectory;
  File outputFile;
  int windowLength = 5;
  
  ArgumentSet arguments;
  arguments.insert(new FileArgument(T("proteinDirectory"), proteinDirectory, true, true), true);
  arguments.insert(new FileArgument(T("outputFile"), outputFile), true);
  arguments.insert(new IntegerArgument(T("windowLength"), windowLength));
  
  if (!arguments.parse(argv, 1, argc-1)) {
    std::cout << "Usage: " << argv[0] << " " << arguments.toString() << std::endl;
    return 1;
  }
  
  juce::OwnedArray<File> proteinFiles;
  proteinDirectory.findChildFiles(proteinFiles, File::findFiles, false, T("*.xml"));
  nbProteins = proteinFiles.size();

  if (outputFile.exists())
    outputFile.deleteFile();
  OutputStream* o = outputFile.createOutputStream();

  for (size_t i = 0; i < (size_t)proteinFiles.size(); ++i)
  {
    std::cout << "Extracting " << proteinFiles[i]->getFileNameWithoutExtension() << " ... ";
    std::cout.flush();

    ProteinPtr protein = Protein::createFromXml(*proteinFiles[i]);
    protein->computeMissingFields();

    VectorPtr ss3 = protein->getSecondaryStructure();
    if (!ss3)
    {
      std::cout << "Skipped - No SS3 available" << std::endl;
      continue;
    }
    ++nbValidProteins;
    for (int j = 0; j < (int)protein->getLength(); ++j)
    {
      int ss3Value = ss3->getVariable(j).getInteger();
      if (ss3Value == -1)
      {
        ++nbSkippedExamples;
        continue;
      }

      switch (ss3Value)
      {
      case 0: *o << "1 0 0 "; break;
      case 1: *o << "0 1 0 "; break;
      case 2: *o << "0 0 1 "; break;
      }
      
      /* Extract a window */
      for (int current = j - windowLength; current <= j + windowLength; ++current)
      {
        generateFeatures(protein, current, o);
      }
      *o << "\n";
      ++nbExamples;
    }
    
    std::cout << "OK" << std::endl;
  }
  
  delete o;

  std::cout << "----- STATS ----" << std::endl;
  std::cout << "  Successfully extracted proteins: " << nbValidProteins << "/" << nbProteins << std::endl;
  std::cout << "  Num. examples: " << nbExamples << std::endl;
  std::cout << "  Num. unsupervised examples (skipped): " << nbSkippedExamples << std::endl;
  
  return 0;
}
#endif

