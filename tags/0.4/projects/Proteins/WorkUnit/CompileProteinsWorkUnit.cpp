/*-----------------------------------------.---------------------------------.
| Filename: CompileProteins.cpp            | Merge proteins with pssms and   |
| Author  : Julien Becker                  |  dssp files                     |
| Started : 25/11/2010 11:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "CompileProteinsWorkUnit.h"
#include "../Data/Protein.h"
#include "../Data/Formats/PSSMFileParser.h"
#include "../Data/Formats/DSSPFileParser.h"

using namespace lbcpp;

bool loadPSSMFile(ProteinPtr protein, const File& pssmFile, ExecutionContext& context)
{
  VectorPtr primaryStructure = protein->getPrimaryStructure();
  jassert(primaryStructure);
  VectorPtr pssm = StreamPtr(new PSSMFileParser(context, pssmFile, primaryStructure))->next().getObjectAndCast<Vector>(); 
  if (!pssm)
    return false;
  protein->setPositionSpecificScoringMatrix(pssm);
  return true;
}

bool loadDSSPFile(ProteinPtr protein, const File& dsspFile, ExecutionContext& context)
  {StreamPtr(new DSSPFileParser(context, dsspFile, protein))->next(); return true;}


Variable CompileProteinsWorkUnit::run(ExecutionContext& context)
{
  if (!inputProteinFile.exists())
  {
    context.errorCallback(T("CompileProteinsWorkUnit::run"), T("The input file does not exists"));
    return false;
  }
  
  ProteinPtr protein = Protein::createFromFile(context, inputProteinFile);
  if (!protein)
  {
    context.errorCallback(T("CompileProteinsWorkUnit::run"), T("Invalid input protein"));
    return false;
  }
  
  if (pssmFile != File::nonexistent && !loadPSSMFile(protein, pssmFile, context))
  {
    context.errorCallback(T("CompileProteinsWorkUnit::run"), T("Invalid pssm file: ") + pssmFile.getFullPathName());
    return false;
  }
  
  if (dsspFile != File::nonexistent && !loadDSSPFile(protein, dsspFile, context))
  {
    context.errorCallback(T("CompileProteinsWorkUnit::run"), T("Invalid dssp file: ") + dsspFile.getFullPathName());
    return false;
  }
  
  Variable(protein).saveToFile(context, outputProteinFile);

  return true;
}
