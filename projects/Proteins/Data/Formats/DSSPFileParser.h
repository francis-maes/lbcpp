/*-----------------------------------------.---------------------------------.
| Filename: DSSPFileParser.cpp             | DSSP Parser                     |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 19:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_DSSP_FILE_PARSER_H_
# define LBCPP_PROTEIN_DSSP_FILE_PARSER_H_

# include "../Protein.h"
# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

class DSSPFileParser : public TextParser
{
public:
  DSSPFileParser(ExecutionContext& context, const File& file, ProteinPtr protein)
    : TextParser(context, file), protein(protein)
  {
    jassert(protein->getPrimaryStructure());
    primarySequence = protein->getPrimaryStructure();
    std::cout << "AA: " << primarySequence->toString() << std::endl;
  }
  
  virtual TypePtr getElementsType() const
    {return proteinClass;}

  virtual void parseBegin(ExecutionContext& context)
    {serialNumber = 0;}

  virtual bool parseLine(ExecutionContext& context, const String& line)
  {
    if (line.isEmpty())
      return true; // skip empty lines

    size_t n = protein->getLength();

    if (serialNumber == 0)
    {
      if (line.startsWith(T("  #  RESIDUE AA STRUCTURE BP1 BP2  ACC")))
      {
        dsspSecondaryStructureSequence = protein->createEmptyDSSPSecondaryStructure();
        solventAccessibilitySequence = protein->createEmptyProbabilitySequence();
        ++serialNumber;
      }
      return true;
    }

    if (line.length() < 100)
    {
      context.errorCallback(T("DSSPFileParser::parseLine"), T("Line is not long enough"));
      return false;
    }

    int newSerialNumber = line.substring(0, 5).trim().getIntValue();
    if (newSerialNumber != (int)serialNumber)
    {
      context.errorCallback(T("DSSPFileParser::parseLine"), T("Invalid serial number: ") + String(newSerialNumber));
      return false;
    }
    ++serialNumber;
    
    String residueNumberString = line.substring(5, 10).trim();
    if (residueNumberString.isEmpty())
      return true; // skip

    int residueNumber = residueNumberString.getIntValue() -1;    
    if (residueNumber < 0 || residueNumber >= (int)n)
    {
      context.errorCallback(T("DSSPFileParser::parseLine"), T("Invalid residue number: ") + String(residueNumber));
      return false;
    }

    /*
    ** Amino Acid
    */
    juce::tchar aminoAcidCode = line.substring(13, 14).trim().getLastCharacter();
    AminoAcidType aminoAcidType = (AminoAcidType)primarySequence->getElement(residueNumber).getInteger();
    juce::tchar expectedAminoAcid = AminoAcid::toOneLetterCode(aminoAcidType);

    if (aminoAcidCode != expectedAminoAcid)
    {
      context.errorCallback(T("DSSPFileParser::parseLine"), T("Amino acid does not matches: ") + aminoAcidCode);
      return false;
    }
    
    /*
    ** 8-state Secondary Structure
    */
    String secondaryStructureCode = line.substring(16, 17);
    if (secondaryStructureCode == T(" "))
      secondaryStructureCode = T("C");
    EnumerationPtr dsspEnum = dsspSecondaryStructureElementEnumeration;
    int secondaryStructureIndex = dsspEnum->getOneLetterCodes().indexOf(secondaryStructureCode);
    if (secondaryStructureIndex < 0)
    {
      context.errorCallback(T("DSSPFileParser::parseLine"), T("Unrecognized secondary structure code: '") + secondaryStructureCode + T("'"));
      return false;
    }
    dsspSecondaryStructureSequence->setElement((size_t)residueNumber, Variable(secondaryStructureIndex, dsspEnum));

    /*
    ** Solvent accesibility
    */
    String solventAccessibilityString = line.substring(34, 38).trim();
    if (!solventAccessibilityString.containsOnly(T("0123456789")))
    {
      context.errorCallback(T("DSSPFileParser::parseLine"), T("Invalid solvent accesibility: ") + solventAccessibilityString);
      return false;
    }
    int absoluteSolventAccesiblity = solventAccessibilityString.getIntValue();

    static const double maximumSolventAccissibilityValue[] = {
      204, 341, 271, 258, 198,
      346, 320, 220, 340, 248,
      250, 337, 305, 329, 244,
      207, 223, 376, 341, 223,
      162.1, 189.7, 187.05, 256.0
    };

    double normalizedSolventAccessibility = (double)absoluteSolventAccesiblity / maximumSolventAccissibilityValue[aminoAcidType];
    // jassert(normalizedSolventAccessibility <= 1.0); FIXME: IT FAILS !
    if (normalizedSolventAccessibility > 1.0)
    {
      std::cout << "Solvent Accessibility Exeeded: " << aminoAcidCode << " > " << absoluteSolventAccesiblity << " of " << maximumSolventAccissibilityValue[aminoAcidType] << std::endl;
      normalizedSolventAccessibility = 1.0;
    }
    solventAccessibilitySequence->setElement((size_t)residueNumber, Variable(normalizedSolventAccessibility, probabilityType));
    return true;
  }

  virtual bool parseEnd(ExecutionContext& context)
  {
    if (!serialNumber)
    {
      context.errorCallback(T("DSSPFileParser::parseEnd"), T("No residues in dssp file"));
      return false;
    }

    std::cout << "SS: " << dsspSecondaryStructureSequence->toString() << std::endl;

    setResult(dsspSecondaryStructureSequence);
    protein->setDSSPSecondaryStructure(dsspSecondaryStructureSequence);
    protein->setSolventAccessibility(solventAccessibilitySequence);
    return true;
  }
  
protected:
  ProteinPtr protein;
  VectorPtr primarySequence;
  VectorPtr dsspSecondaryStructureSequence;
  VectorPtr solventAccessibilitySequence;
  int serialNumber;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DSSP_FILE_PARSER_H_
