
#include <lbcpp/lbcpp.h>
#include "../Data/Formats/FASTAFileParser.h"
#include "../Programs/ArgumentSet.h"

using namespace lbcpp;

extern void declareProteinClasses();

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();
  
  File pdbDirectoryFile;
  File fastaFile;
  File outputDirectoryFile;
  
  ArgumentSet arguments;
  arguments.insert(new FileArgument(T("pdbDirectory"), pdbDirectoryFile, true, true), true);
  arguments.insert(new FileArgument(T("fastaFile"), fastaFile, true), true);
  arguments.insert(new FileArgument(T("outputDirectory"), outputDirectoryFile, true, true), true);
  
  if (!arguments.parse(argv, 1, argc-1)) {
    std::cout << "Usage: " << argv[0] << " " << arguments.toString() << std::endl;
    return 1;
  }
  
  FASTAFileParser fastaParser(fastaFile);
  while (true)
  {
    Variable variable = fastaParser.next();
    if (!variable)
      break;

    ProteinPtr protein = variable.getObjectAndCast<Protein>();
    
    String name = protein->getName().substring(0, 5);
    VectorPtr primaryStructure = protein->getPrimaryStructure();

    std::cout << name << " > " << primaryStructure->toString() << std::endl;
    File pdbFile = pdbDirectoryFile.getChildFile(name + T(".pdb"));
    if (!pdbFile.exists())
    {
      std::cout << "File not found: " << pdbFile.getFullPathName() << std::endl;
      continue;
    }

    File outputFile = outputDirectoryFile.getChildFile(name + T(".pdb"));
    OutputStream* o = outputFile.createOutputStream();
    
    // Write SEQRES record
    *o << "HEADER                                                       " << name << "\n";

    enum {numResiduesPerLine = 13};
    size_t numResidues = 0;
    size_t numLine = 0;
    for (size_t i = 0; i < primaryStructure->size(); ++i)
    {
      if (!numResidues)
      {
        *o << "SEQRES ";
        String id = lbcpp::toString(++numLine);
        for (size_t i = id.length(); i < 3; ++i)
          *o << " ";
        *o << id << "    128  ";
      }
      *o << AminoAcid::toThreeLettersCode((AminoAcidType)primaryStructure->getVariable(i).getInteger()).toUpperCase() << " ";
      
      ++numResidues;
      numResidues %= numResiduesPerLine;
      
      if (!numResidues)
        *o << "\n";
    }
    
    if (numResidues)
      *o << "\n";
    
    // Copy ATOM record from PDB
    InputStream* in = pdbFile.createInputStream();
    
    while (!in->isExhausted())
    {
      String line = in->readNextLine();
      if (!line.startsWith(T("ATOM")))
        continue;
      *o << line << "\n";
    }
    
    *o << "END";
    
    delete in;
    delete o;
  }
  
  return 0;
}