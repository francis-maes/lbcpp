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
  DSSPFileParser(const File& file, ProteinPtr protein, ErrorHandler& callback = ErrorHandler::getInstance())
    : TextParser(file, callback), protein(protein)
  {
    jassert(protein->getPrimaryStructure());
    primarySequence = protein->getPrimaryStructure();
    std::cout << "AA: " << primarySequence->toString() << std::endl;
  }
  
  virtual TypePtr getElementsType() const
    {return proteinClass();}

  virtual void parseBegin()
    {serialNumber = 0;}

  virtual bool parseLine(const String& line)
  {
    if (line.isEmpty())
      return true; // skip empty lines

    size_t n = protein->getLength();

    if (serialNumber == 0)
    {
      if (line.startsWith(T("  #  RESIDUE AA STRUCTURE BP1 BP2  ACC")))
      {
        dsspSecondaryStructureSequence = protein->createEmptyDSSPSecondaryStructure();
        solventAccesibilitySequence = protein->createEmptyProbabilitySequence();
        ++serialNumber;
      }
      return true;
    }

    if (line.length() < 100)
    {
      callback.errorMessage(T("DSSPFileParser::parseLine"), T("Line is not long enough"));
      return false;
    }

    int newSerialNumber = line.substring(0, 5).trim().getIntValue();
    if (newSerialNumber != (int)serialNumber)
    {
      callback.errorMessage(T("DSSPFileParser::parseLine"), T("Invalid serial number: ") + lbcpp::toString(newSerialNumber));
      return false;
    }
    ++serialNumber;
    
    String residueNumberString = line.substring(5, 10).trim();
    if (residueNumberString.isEmpty())
      return true; // skip

    int residueNumber = residueNumberString.getIntValue() -1;    
    if (residueNumber < 0 || residueNumber >= (int)n)
    {
      callback.errorMessage(T("DSSPFileParser::parseLine"), T("Invalid residue number: ") + lbcpp::toString(residueNumber));
      return false;
    }

    /*
    ** Amino Acid
    */
    juce::tchar aminoAcidCode = line.substring(13, 14).trim().getLastCharacter();
    AminoAcidType aminoAcidType = (AminoAcidType)primarySequence->getVariable(residueNumber).getInteger();
    juce::tchar expectedAminoAcid = AminoAcid::toOneLetterCode(aminoAcidType);

    if (aminoAcidCode != expectedAminoAcid)
    {
      callback.errorMessage(T("DSSPFileParser::parseLine"), T("Amino acid does not matches: ") + aminoAcidCode);
      return false;
    }
    
    /*
    ** 8-state Secondary Structure
    */
    String secondaryStructureCode = line.substring(16, 17);
    if (secondaryStructureCode == T(" "))
      secondaryStructureCode = T("C");
    int secondaryStructureIndex = dsspSecondaryStructureEnumeration()->getOneLetterCodes().indexOf(secondaryStructureCode);
    if (secondaryStructureIndex < 0)
    {
      callback.errorMessage(T("DSSPFileParser::parseLine"), T("Unrecognized secondary structure code: '") + secondaryStructureCode + T("'"));
      return false;
    }
    dsspSecondaryStructureSequence->setVariable((size_t)residueNumber, Variable(secondaryStructureIndex, dsspSecondaryStructureEnumeration()));

    /*
    ** Solvent accesibility
    */
    String solventAccesibilityString = line.substring(34, 38).trim();
    if (!solventAccesibilityString.containsOnly(T("0123456789")))
    {
      callback.errorMessage(T("DSSPFileParser::parseLine"), T("Invalid solvent accesibility: ") + solventAccesibilityString);
      return false;
    }
    int absoluteSolventAccesiblity = solventAccesibilityString.getIntValue();

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
      std::cout << "Solvent Accessibility Exeeded: " << lbcpp::toString(aminoAcidCode) << " > " << absoluteSolventAccesiblity << " of " << maximumSolventAccissibilityValue[aminoAcidType] << std::endl;
      normalizedSolventAccessibility = 1.0;
    }
    solventAccesibilitySequence->setVariable((size_t)residueNumber, Variable(normalizedSolventAccessibility, probabilityType()));
    return true;
  }

  virtual bool parseEnd()
  {
    if (!serialNumber)
    {
      callback.errorMessage(T("DSSPFileParser::parseEnd"), T("No residues in dssp file"));
      return false;
    }

    std::cout << "SS: " << dsspSecondaryStructureSequence->toString() << std::endl;

    setResult(dsspSecondaryStructureSequence);
    protein->setDSSPSecondaryStructure(dsspSecondaryStructureSequence);
    protein->setSolventAccessibility(solventAccesibilitySequence);
    return true;
  }
  
protected:
  ProteinPtr protein;
  VectorPtr primarySequence;
  VectorPtr dsspSecondaryStructureSequence;
  VectorPtr solventAccesibilitySequence;
  int serialNumber;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DSSP_FILE_PARSER_H_
