#include "Data/Protein.h"

namespace lbcpp
{

/*
 * Input:
 *  Column
 *  1. Sequential record number
 *  2. Residue letter
 *  3. (Character column 7) Secondary structure letter (Kabsch & Sander)
 *  4. Accessibility (% GGXGG pentapeptide in fully extended conformation)
 *  5. Phi
 *  6. Psi
 *  7. Omega angles
 *  8-22. XYZ coords for N,CA,C,O,CB atoms
 *  23. PDB Residue ID
 *  24-44. PSIBLAST Sequence Profile (Residue order: ARNDCQEGHILKMFPSTWYV)
 * Ouput:
 *  Protein
 */
class TDBFileParser : public TextParser
{
public:
  TDBFileParser(ExecutionContext& context, const File& file)
    : TextParser(context, file),
      fileName(file.getFileNameWithoutExtension()),
      proteinLength((size_t)-1),
      currentResidueIndex(0) {}

  virtual TypePtr getElementsType() const
    {return proteinClass;}
  
  virtual void parseBegin() {}
  
  virtual bool parseLine(const String& srcLine)
  {
    String line = srcLine.trim();
    if (line == String::empty)
      return true;
    /* Parse header */
    // Get the number of residues
    if (line.startsWith(T("#TDB")))
    {
      if (proteinLength != (size_t)-1)
      {
        context.errorCallback(T("TDBFileParser::parseLine"), T("Duplicate header"));
        return false;
      }
      StringArray tokens;
      tokens.addTokens(line, T(" "), NULL);
      tokens.removeEmptyStrings(true);

      int length = tokens[2].getIntValue();
      if (length < 0)
      {
        context.errorCallback(T("TDBFileParser::parseLine"), T("Invalid length: ") + String(length));
        return false;
      }
      proteinLength = (size_t)length;
      return true;
    }
    /* Parse residue */
    // Create protein
    if (currentResidueIndex == 0)
    {
      primaryStructure = vector(aminoAcidTypeEnumeration, proteinLength);
      dsspSecondaryStructureSequence = Protein::createEmptyDSSPSecondaryStructure(proteinLength, true);
    }
    // Replace the coil symbole ' ' by 'C'
    if(line[5] == T(' '))
      line[5] = T('C');

    StringArray tokens;
    tokens.addTokens(line, T(" "), NULL);
    tokens.removeEmptyStrings(true);
    
    if (tokens.size() != 43)
    {
      context.errorCallback(T("TDBFileParser::parseLine"), 
                            T("Invalid number of columns: ")
                            + String(tokens.size()));
      return false;
    }
    // Check the residue index
    int residueIndex = tokens[0].getIntValue();
    if ((size_t)residueIndex != currentResidueIndex + 1)
    {
      context.errorCallback(T("TDBFileParser::parseLine"),
                            T("Invalid residue number: ")
                            + String(residueIndex) + T(" instead of ")
                            + String((int)currentResidueIndex + 1));
      return false;
    }
    // Add residue symbol
    primaryStructure->setElement(currentResidueIndex, AminoAcid::fromOneLetterCode(tokens[1][0]));
    // Add DSSP secondary structure
    juce::tchar secondaryStructureCode = tokens[2][0];
    int secondaryStructureIndex = dsspSecondaryStructureElementEnumeration->findElementByOneLetterCode(secondaryStructureCode);
    if (secondaryStructureIndex < 0)
    {
      context.errorCallback(T("TDBFileParser::parseLine"),
                            T("Unrecognized secondary structure code: '") +
                            String(secondaryStructureCode) + T("'"));
      return false;
    }
    SparseDoubleVectorPtr svDssp = new SparseDoubleVector(dsspSecondaryStructureElementEnumeration, probabilityType);
    svDssp->appendValue(secondaryStructureIndex, 1.0);
    dsspSecondaryStructureSequence->setElement(currentResidueIndex, svDssp);

    ++currentResidueIndex;
    return true;
  }
  
  virtual bool parseEnd()
  {
    if (currentResidueIndex != proteinLength)
    {
      context.errorCallback(T("TDBFileParser::parseEnd"),
                            T("Missing residue: ") + String((int)currentResidueIndex) +
                            T(" instead of ") + String((int)proteinLength));
      return false;      
    }
    
    ProteinPtr protein = new Protein(fileName);
    protein->setPrimaryStructure(primaryStructure);
    protein->setDSSPSecondaryStructure(dsspSecondaryStructureSequence);

    setResult(protein);
    return true;
  }
  
protected:
  String fileName;
  ProteinPtr protein;
  size_t proteinLength;
  size_t currentResidueIndex;
  VectorPtr primaryStructure;
  VectorPtr dsspSecondaryStructureSequence;
};

};
