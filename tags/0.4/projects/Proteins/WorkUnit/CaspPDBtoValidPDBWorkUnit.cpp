/*-----------------------------------------.---------------------------------.
 | Filename: CaspPDBtoValidPDBWorkUnit.cpp  | CaspPDBtoValidPDBWorkUnit       |
 | Author  : Julien Becker                  |                                 |
 | Started : 25/11/2010 11:01               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/
#include "CaspPDBtoValidPDBWorkUnit.h"
#include "../Data/Formats/FASTAFileParser.h"

using namespace lbcpp;

Variable CaspPDBtoValidPDBWorkUnit::run(ExecutionContext& context)
{
  if (pdbDirectory == File::nonexistent || fastaFile == File::nonexistent)
  {
    context.errorCallback(T("CaspPDBtoValidPDBProgram::run"), T("pdbDirectory or fastaFile not specified"));
    return false;
  }

  FASTAFileParser fastaParser(context, fastaFile);
  while (true)
  {
    Variable variable = fastaParser.next();
    if (!variable.exists())
      break;

    const ProteinPtr& protein = variable.getObjectAndCast<Protein>();
    
    String name = protein->getName().substring(0, 5);
    VectorPtr primaryStructure = protein->getPrimaryStructure();

    std::cout << name << " > " << primaryStructure->toString() << std::endl;
    File pdbFile = pdbDirectory.getChildFile(name + T(".pdb"));
    if (!pdbFile.exists())
    {
      std::cout << "File not found: " << pdbFile.getFullPathName() << std::endl;
      continue;
    }

    File outputFile = outputDirectory.getChildFile(name + T(".pdb"));
    OutputStream* o = outputFile.createOutputStream();
    
    // Write SEQRES record
    *o << "HEADER                                                       " << name << "\n";

    enum {numResiduesPerLine = 13};
    size_t numResidues = 0;
    size_t numLine = 0;
    for (size_t i = 0; i < primaryStructure->getNumElements(); ++i)
    {
      if (!numResidues)
      {
        *o << "SEQRES ";
        String id = String((int)(++numLine));
        for (size_t i = id.length(); i < 3; ++i)
          *o << " ";
        *o << id << "    128  ";
      }
      *o << AminoAcid::toThreeLettersCode((AminoAcidType)primaryStructure->getElement(i).getInteger()).toUpperCase() << " ";
      
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

