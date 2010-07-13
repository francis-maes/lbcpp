/*-----------------------------------------.---------------------------------.
| Filename: DSSPFileParser.cpp             | DSSP Parser                     |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 19:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_DSSP_FILE_PARSER_H_
# define LBCPP_PROTEIN_DSSP_FILE_PARSER_H_

# include "../Data/Protein.h"
# include "../SecondaryStructureDictionary.h"

namespace lbcpp
{

class DSSPFileParser : public TextParser
{
public:
  DSSPFileParser(const File& file, ProteinPtr protein)
    : TextParser(file), protein(protein)//, firstResidueNumber(-1)
  {
    jassert(protein->getPrimaryStructure());
    primarySequence = protein->getPrimaryStructure();
    std::cout << "AA: " << primarySequence->toString() << std::endl;
  }
  
  virtual TypePtr getElementsType() const
    {return vectorClass(dsspSecondaryStructureElementEnumeration());}

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
//        aminoAcidSequence = protein->createEmptyObject(T("AminoAcidSequence"));
        
        dsspSecondaryStructureSequence = protein->createEmptyDSSPSecondaryStructure();
        solventAccesibilitySequence = protein->createEmptySolventAccesibility();
        ++serialNumber;
      }
      return true;
    }

//    if (serialNumber == 1)
//      firstChainID = line.substring(11, 12);

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
    
//    if (line.substring(11, 12) != firstChainID)
//      return true;
    
    String residueNumberString = line.substring(5, 10).trim();
    if (residueNumberString.isEmpty())
      return true; // skip

    int residueNumber = residueNumberString.getIntValue() -1;
//    if (firstResidueNumber == -1)
//      firstResidueNumber = residueNumber;
//    residueNumber -= firstResidueNumber;
    
    if (residueNumber < 0 || residueNumber >= (int)n)
    {
      callback.errorMessage(T("DSSPFileParser::parseLine"), T("Invalid residue number: ") + lbcpp::toString(residueNumber));
      return false;
    }

    /*
    ** Amino Acid
    */
    juce::tchar aminoAcidCode = line.substring(13, 14).trim().getLastCharacter();
    juce::tchar expectedAminoAcid = primarySequence->getVariable(residueNumber).getObjectAndCast<AminoAcid>()->getOneLetterCode();
    
    if (aminoAcidCode != expectedAminoAcid)
    {
      callback.errorMessage(T("DSSPFileParser::parseLine"), T("Amino acid does not matches: ") + aminoAcidCode);
      return false;
    }
//    aminoAcidSequence->setIndex((size_t)residueNumber, aminoAcidCode);
    
    /*
    ** 8-state Secondary Structure
    */
    String secondaryStructureCode = line.substring(16, 17);
    if (secondaryStructureCode == T(" "))
      secondaryStructureCode = T("C");
    int secondaryStructureIndex = dsspSecondaryStructureElementEnumeration()->findElement(secondaryStructureCode);
    if (secondaryStructureIndex < 0)
    {
      callback.errorMessage(T("DSSPFileParser::parseLine"), T("Unrecognized secondary structure code: ") + secondaryStructureCode);
      return false;
    }
    dsspSecondaryStructureSequence->setVariable((size_t)residueNumber, Variable(secondaryStructureIndex, dsspSecondaryStructureElementEnumeration()));

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

    double normalizedSolventAccessibility = (double)absoluteSolventAccesiblity / maximumSolventAccissibilityValue[aminoAcidCode];
    // jassert(normalizedSolventAccessibility <= 1.0); FIXME: IT FAILS !
    if (normalizedSolventAccessibility > 1.0)
    {
      std::cout << "Solvent Accessibility Exeeded: " << aminoAcidCode << " > " << absoluteSolventAccesiblity << " of " << maximumSolventAccissibilityValue[aminoAcidCode] << std::endl;
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
    //std::cout << "AA: " << aminoAcidSequence->toString() << std::endl;
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
//  int firstResidueNumber;
//  String firstChainID;
  std::map<int, std::pair<int, int> > betaBridgePartners;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DSSP_FILE_PARSER_H_
