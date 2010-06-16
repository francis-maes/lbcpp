
#include <lbcpp/lbcpp.h>
#include "../Programs/ArgumentSet.h"
#include "Protein/Protein.h"

using namespace lbcpp;

extern void declareProteinClasses();

int main(int argc, char** argv)
{
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
    ProteinPtr protein = Protein::createFromFile(*proteinFiles[i]);
    File outputFile = outputDirectory.getChildFile(proteinFiles[i]->getFileNameWithoutExtension() + T(".seq"));//T(".protein"));//
    
    std::cout << "Extracting " << proteinFiles[i]->getFileNameWithoutExtension() << " ... ";
/*
    if (protein->getLength() < 50)
      protein->saveToFile(outputFile);
*/    
    OutputStream* o = outputFile.createOutputStream();
    *o << protein->getAminoAcidSequence()->toString();
    delete o;
    
    std::cout << "OK" << std::endl;
  }
  
  return 0;
}
