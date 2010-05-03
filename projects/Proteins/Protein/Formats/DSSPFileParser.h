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
# include "../SecondaryStructureDictionary.h"

namespace lbcpp
{

class ProteinBetaBridges : public Object
{
public:
  // TODO ... 
  
};

class DSSPFileParser : public TextObjectParser
{
public:
  DSSPFileParser(const File& file, ProteinPtr protein)
    : TextObjectParser(file), protein(protein), firstResidueNumber(-1)
  {
    jassert(protein->getAminoAcidSequence());
    //std::cout << "AA: " << protein->getAminoAcidSequence()->toString() << std::endl;
  }

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
        aminoAcidSequence = protein->createEmptyObject(T("AminoAcidSequence"));
        dsspSecondaryStructureSequence = protein->createEmptyObject(T("DSSPSecondaryStructureSequence"));
        solventAccesibilitySequence = protein->createEmptyObject(T("NormalizedSolventAccessibilitySequence"));
        ++serialNumber;
      }
      return true;
    }

    if (serialNumber == 1)
      firstChainID = line.substring(11, 12);

    if (line.length() < 100)
    {
      Object::error(T("DSSPFileParser::parseLine"), T("Line is not long enough"));
      return false;
    }

    int newSerialNumber = line.substring(0, 5).trim().getIntValue();
    if (newSerialNumber != (int)serialNumber)
    {
      Object::error(T("DSSPFileParser::parseLine"), T("Invalid serial number: ") + lbcpp::toString(newSerialNumber));
      return false;
    }
    ++serialNumber;
    
    if (line.substring(11, 12) != firstChainID)
      return true;
    
    String residueNumberString = line.substring(5, 10).trim();
    if (residueNumberString.isEmpty())
      return true; // skip

    int residueNumber = residueNumberString.getIntValue();
    if (firstResidueNumber == -1)
      firstResidueNumber = residueNumber;
    residueNumber -= firstResidueNumber;
    
    if (residueNumber < 0 || residueNumber >= (int)n)
    {
      Object::error(T("DSSPFileParser::parseLine"), T("Invalid residue number: ") + lbcpp::toString(residueNumber));
      return false;
    }

    /*
    ** Amino Acid
    */
    size_t aminoAcidCode = AminoAcidDictionary::getTypeFromOneLetterCode(line.substring(13, 14).trim().getLastCharacter());
    aminoAcidSequence->setIndex((size_t)residueNumber, aminoAcidCode);
    
    /*
    ** 8-state Secondary Structure
    */
    String secondaryStructureCode = line.substring(16, 17);
    if (secondaryStructureCode == T(" "))
      secondaryStructureCode = T("_");
    int secondaryStructureIndex = DSSPSecondaryStructureDictionary::getInstance()->getFeatures()->getIndex(secondaryStructureCode);
    if (secondaryStructureIndex < 0)
    {
      Object::error(T("DSSPFileParser::parseLine"), T("Unrecognized secondary structure code: ") + secondaryStructureCode);
      return false;
    }
    dsspSecondaryStructureSequence->setIndex((size_t)residueNumber, (size_t)secondaryStructureIndex);

    /*
    ** Solvent accesibility
    */
    String solventAccesibilityString = line.substring(34, 38).trim();
    if (!solventAccesibilityString.containsOnly(T("0123456789")))
    {
      Object::error(T("DSSPFileParser::parseLine"), T("Invalid solvent accesibility: ") + solventAccesibilityString);
      return false;
    }
    int absoluteSolventAccesiblity = solventAccesibilityString.getIntValue();

    static const double maximumSolventAccissibilityValue[] = {
      118.1, 256.0, 165.5, 158.7, 146.1,
      186.2, 193.2, 88.1, 202.5, 181.0,
      193.1, 225.8, 203.4, 222.8, 146.8,
      129.8, 152.5, 266.3, 236.8, 164.5,
      162.1, 189.7, 187.05, 256.0
    };

    double normalizedSolventAccessibility = (double)absoluteSolventAccesiblity / maximumSolventAccissibilityValue[aminoAcidCode];
    // jassert(normalizedSolventAccessibility <= 1.0); FIXME: IT FAILS !
    if (normalizedSolventAccessibility > 1.0)
      normalizedSolventAccessibility = 1.0;
    solventAccesibilitySequence->setValue((size_t)residueNumber, normalizedSolventAccessibility);

    /*
    ** Beta bridge partners
    */
    String bp1 = line.substring(25, 29).trim();
    String bp2 = line.substring(29, 33).trim();
    if (secondaryStructureCode != T("E"))
    {
      if (bp1 != T("0") || bp2 != T("0"))
      {
        Object::error(T("DSSPFileParser::parseLine"), T("BP fields should be null"));
        return false;
      }
    }
    else
    {
      int index1 = bp1.getIntValue();
      int index2 = bp2.getIntValue();
      if (index1 || index2)
        betaBridgePartners[newSerialNumber] = std::make_pair(index1, index2);
    }
    return true;
  }

  bool finalizeBetaBridgePartners()
  {
    std::cout << "betaBridgePartners: " << std::endl;
    for (std::map<int, std::pair<int, int> >::const_iterator it = betaBridgePartners.begin(); it != betaBridgePartners.end(); ++it)
    {
      if (it->second.first)
        std::cout << it->first << " => " << it->second.first << std::endl;
      if (it->second.second)
        std::cout << it->first << " => " << it->second.second << std::endl;
    }
    
    exit(1);

    return true;
  }

  virtual bool parseEnd()
  {
    if (!serialNumber)
    {
      Object::error(T("DSSPFileParser::parseEnd"), T("No residues in dssp file"));
      return false;
    }
    if (!finalizeBetaBridgePartners())
      return false;

    size_t nbReadResidues = protein->getLength();
    for (; aminoAcidSequence->getIndex(nbReadResidues-1) < 0; --nbReadResidues);

    LabelSequencePtr proteinAminoAcidSequence = protein->getAminoAcidSequence(); 
    size_t nbMaxCorrectAlignment = 0;
    size_t bestShift = 0;
    for (size_t i = 0; i <= protein->getLength() - nbReadResidues; ++i)
    {
      size_t nbCorrectAlignment = 0;
      for (size_t j = 0; j < nbReadResidues; ++j)
      {
        if (proteinAminoAcidSequence->getString(i + j) == aminoAcidSequence->getString(j))
          ++nbCorrectAlignment;
      }

      if (nbCorrectAlignment > nbMaxCorrectAlignment)
      {
        nbMaxCorrectAlignment = nbCorrectAlignment;
        bestShift = i;
      }
    }

    std::cout << "Gap:";
    for (size_t i = 0; i < bestShift; ++i)
      std::cout << " ";
    std::cout  << aminoAcidSequence->toString() << std::endl;
    /*
    if (nbMaxCorrectAlignment != lastResidueNumber)
    {
      Object::error(T("DSSPFileParser::parseLine"), T("Amino acid does not matches"));
      return false;
    }
    */
    LabelSequencePtr dsspSecondaryStructureSequence = protein->createEmptyObject(T("DSSPSecondaryStructureSequence"));
    ScalarSequencePtr solventAccesibilitySequence = protein->createEmptyObject(T("NormalizedSolventAccessibilitySequence"));
    
    for (size_t i = 0; i < aminoAcidSequence->size() - bestShift; ++i)
    {
      dsspSecondaryStructureSequence->setIndex(bestShift + i, this->dsspSecondaryStructureSequence->getIndex(i));
      solventAccesibilitySequence->setValue(bestShift + i, this->solventAccesibilitySequence->getValue(i));
    }
    std::cout << "SS :" << dsspSecondaryStructureSequence->toString() << std::endl;

    setResult(dsspSecondaryStructureSequence);
    protein->setObject(dsspSecondaryStructureSequence);
    protein->setObject(solventAccesibilitySequence);
    return true;
  }
  
protected:
  ProteinPtr protein;
  LabelSequencePtr aminoAcidSequence;
  LabelSequencePtr dsspSecondaryStructureSequence;
  ScalarSequencePtr solventAccesibilitySequence;
  int serialNumber;
  int firstResidueNumber;
  String firstChainID;
  std::map<int, std::pair<int, int> > betaBridgePartners;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DSSP_FILE_PARSER_H_
