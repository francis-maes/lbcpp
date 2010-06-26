
#include <lbcpp/lbcpp.h>
#include "../Programs/ArgumentSet.h"
#include "Protein/ProteinObject.h"

using namespace lbcpp;

extern void declareProteinClasses();

//#define SEQUENCE
#define FEATURES

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
  proteinDirectory.findChildFiles(proteinFiles, File::findFiles, false, T("*.protein"));
  
  for (size_t i = 0; i < (size_t)proteinFiles.size(); ++i)
  {
    ProteinObjectPtr protein = ProteinObject::createFromFile(*proteinFiles[i]);
    File outputFile = outputDirectory.getChildFile(proteinFiles[i]->getFileNameWithoutExtension() + T(".seq"));
    
    std::cout << "Extracting " << proteinFiles[i]->getFileNameWithoutExtension() << " ... ";

    OutputStream* o = outputFile.createOutputStream();
    *o << protein->getAminoAcidSequence()->toString();
    delete o;
    
    std::cout << "OK" << std::endl;
  }
  
  return 0;
}
#endif

#ifdef FEATURES
static void generateFeatures(ProteinObjectPtr protein, int position, OutputStream* o)
{
  LabelSequencePtr aa = protein->getAminoAcidSequence();
  if (position < 0)
    *o << AminoAcidDictionary::unknown + 1;
  else if (position >= (int)protein->getLength())
    *o << AminoAcidDictionary::unknown + 2;
  else
    *o << aa->getIndex(position);
  *o << " ";

  ScoreVectorSequencePtr pssm = protein->getPositionSpecificScoringMatrix();
  if (position < 0 || position >= (int)protein->getLength())
    for (size_t i = 0; i < pssm->getNumScores(); ++i)
      *o << "0 ";
  else
    for (size_t i = 0; i < pssm->getNumScores(); ++i)
      *o << pssm->getScore(position, i) << " ";
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
  proteinDirectory.findChildFiles(proteinFiles, File::findFiles, false, T("*.protein"));
  nbProteins = proteinFiles.size();

  if (outputFile.exists())
    outputFile.deleteFile();
  OutputStream* o = outputFile.createOutputStream();

  for (size_t i = 0; i < (size_t)proteinFiles.size(); ++i)
  {
    std::cout << "Extracting " << proteinFiles[i]->getFileNameWithoutExtension() << " ... ";
    std::cout.flush();

    ProteinObjectPtr protein = ProteinObject::createFromFile(*proteinFiles[i]);
    protein->computeMissingFields();

    LabelSequencePtr ss3 = protein->getSecondaryStructureSequence();
    if (!ss3)
    {
      std::cout << "Skipped - No SS3 available" << std::endl;
      continue;
    }
    ++nbValidProteins;
    for (int j = 0; j < (int)protein->getLength(); ++j)
    {
      int ss3Value = ss3->getIndex(j);
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

