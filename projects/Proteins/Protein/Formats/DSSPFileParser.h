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

class ProteinBetaStrandBond : public Object
{
public:
  bool isParallel() const
    {return parallel;}

  bool isAntiParallel() const
    {return !parallel;}

private:
  bool parallel;
};

typedef ReferenceCountedObjectPtr<ProteinBetaStrandBond> ProteinBetaStrandBondPtr;

class ProteinBetaStrand : public Object
{
public:
  ProteinBetaStrand(size_t begin, size_t end,
      ProteinBetaStrandBondPtr bondWithPrevious = ProteinBetaStrandBondPtr(), 
      ProteinBetaStrandBondPtr bondWithNext = ProteinBetaStrandBondPtr())
    : begin(begin), end(end), bondWithPrevious(bondWithPrevious), bondWithNext(bondWithNext) {}

private:
  size_t begin, end;
  ProteinBetaStrandBondPtr bondWithPrevious;
  ProteinBetaStrandBondPtr bondWithNext;
};

typedef ReferenceCountedObjectPtr<ProteinBetaStrand> ProteinBetaStrandPtr;

class ProteinBetaBridges : public Object
{
public:
  
private:
  std::vector<ProteinBetaStrandPtr> strands;
};

typedef ReferenceCountedObjectPtr<ProteinBetaBridges> ProteinBetaBridgesPtr;

class ProteinBetaBridgesBuilder
{
public:
  struct LinkPair
  {
    LinkPair() : first(-1), second(-1) {}

    bool insert(int value)
    {
      if (first < 0)
      {
        jassert(second < 0);
        first = value;
        return true;
      }
      else if (value == first)
        return true;

      if (second < 0)
      {
        second = value;
        return true;
      }
      else if (value == second)
        return true;

      Object::error(T("LinkPair::insert"), T("Too many elements in pair"));
      return false;
    }

    size_t getFirst() const
      {jassert(first >= 0); return (size_t)first;}

    size_t getSecond() const
      {jassert(second >= 0); return (size_t)second;}

    size_t size() const
      {return first >= 0 ? (second >= 0 ? 2 : 1) : 0;}

    int first;
    int second;
  };

  bool addBetaLink(size_t first, size_t second)
  {
    jassert(first != second);
    if (first > second)
      {size_t tmp = first; first = second; second = tmp;}
    std::cout << "(" << first << ", " << second << ")\t" << std::flush;
    return betaLinks[first].insert(second);
  }

  struct BridgeInConstruction
  {
    BridgeInConstruction() : sourceStart(0), targetStart(0), order(0), length(0) {}

    bool exists() const
      {return length > 0;}

    bool isConsistentWith(size_t source, size_t target) const
    {
      if (!exists())
        return true;
      if (source != sourceStart + length)
        return false;

      int delta = (int)target - (int)targetStart;
      if (order == 0)
      {
        if (abs(delta) != (int)length)
          return false;
      }
      else if (delta * order != (int)length)
        return false;

      return true;
    }

    void expand(size_t source, size_t target)
    {
      if (exists())
      {
        jassert(isConsistentWith(source, target));
        if (!order)
        {
          jassert(length == 1);
          order = (int)target - (int)targetStart;
          jassert(order == -1 || order == 1);
        }
        ++length;
      }
      else
      {
        sourceStart = source;
        targetStart = target;
        order = 0;
        length = 1;
      }
    }

    String toString() const
    {
      if (!length)
        return T("N/A");
      else if (length == 1)
        return lbcpp::toString(sourceStart) + T(" <--> ") + lbcpp::toString(targetStart);
      jassert(order == 1 || order == -1);
      return T("[") + lbcpp::toString(sourceStart) + T(", ") + lbcpp::toString(sourceStart + length - 1) + T("] <--> [") +
        lbcpp::toString(targetStart) + T(", ") + lbcpp::toString(targetStart + (length - 1) * order) + T("]");
    }

    size_t sourceStart;
    size_t targetStart;
    int order; // 0 = undetermined yet, -1 = antiparallel, +1 = parallel
    size_t length;
  };

  void addLinkToBridge(BridgeInConstruction& currentBridge, size_t first, size_t second, std::vector<BridgeInConstruction>& bridges)
  {
    if (!currentBridge.isConsistentWith(first, second))
    {
      if (currentBridge.exists())
        bridges.push_back(currentBridge);
      currentBridge = BridgeInConstruction();
    }

    currentBridge.expand(first, second);
  }
#if 0
  bool savePNG(juce::Image& image, const File& file)
  {
    if (file.exists())
      file.deleteFile();

    juce::PNGImageFormat format;

    OutputStream* ostr = file.createOutputStream();
    if (!ostr)
      return false;

    format.writeImageToStream(image, *ostr);
    delete ostr;
    return true;
  }

  void makeImage(ProteinPtr protein, const std::map<int, std::pair<int, int> >& betaBridgePartners)
  {
    std::map<int, int> betaMapping;
    for (std::map<int, std::pair<int, int> >::const_iterator it = betaBridgePartners.begin(); it != betaBridgePartners.end(); ++it)
      betaMapping[it->first] = it->first;//betaMapping.size();

    protein->computeMissingFields();
    ScoreSymmetricMatrixPtr contactMap = protein->getResidueResidueContactMatrix8Cb();
    jassert(contactMap);
    //->saveToPNGFile(File(T("C:\\Projets\\LBC++\\projects\\temp\\SmallPDB\\contact8.png")));

    int size = contactMap->getDimension();
    juce::Image* image = new juce::Image(juce::Image::RGB, size, size, true);

    for (int i = 0; i < size; ++i)
      for (int j = 0; j < size; ++j)
      {
        double p = contactMap->getScore(i, j);
        image->setPixelAt(i, j, p ? juce::Colours::white : juce::Colours::black);
      }
/*    for (std::map<int, std::pair<int, int> >::const_iterator it = betaBridgePartners.begin(); it != betaBridgePartners.end(); ++it)
    {
      if (it->second.first)
        image->setPixelAt(betaMapping[it->first], betaMapping[it->second.first], juce::Colours::red);
      if (it->second.second)
        image->setPixelAt(betaMapping[it->first], betaMapping[it->second.second], juce::Colours::blue);
    }*/

    savePNG(*image, File(T("C:\\Projets\\LBC++\\projects\\temp\\SmallPDB\\pouet.png")));
    delete image;
  }
#endif // 0

  ProteinBetaBridgesPtr build(ProteinPtr protein, const std::map<int, std::pair<int, int> >& betaBridgePartners)
  {
    if (!betaBridgePartners.size())
      return ProteinBetaBridgesPtr();

    //makeImage(protein, betaBridgePartners);

    // build directed beta links
    for (std::map<int, std::pair<int, int> >::const_iterator it = betaBridgePartners.begin(); it != betaBridgePartners.end(); ++it)
    {
      if (it->second.first && !addBetaLink((size_t)it->first, (size_t)it->second.first))
        return ProteinBetaBridgesPtr();
      if (it->second.second && !addBetaLink((size_t)it->first, (size_t)it->second.second))
        return ProteinBetaBridgesPtr();
    }

    std::vector<BridgeInConstruction> bridges;

    BridgeInConstruction bridge1, bridge2;
    for (std::map<size_t, LinkPair>::const_iterator it = betaLinks.begin(); it != betaLinks.end(); ++it)
    {
      if (it->second.size() >= 1)
        addLinkToBridge(bridge1, it->first, it->second.getFirst(), bridges);
      if (it->second.size() == 2)
        addLinkToBridge(bridge2, it->first, it->second.getSecond(), bridges);
    }
    if (bridge1.exists())
      bridges.push_back(bridge1);
    if (bridge2.exists())
      bridges.push_back(bridge2);

    std::cout << "Beta Bridges: " << std::endl;
    for (size_t i = 0; i < bridges.size(); ++i)
      std::cout << bridges[i].toString() << std::endl;

    return new ProteinBetaBridges();
  }

private:
  std::map<size_t, LinkPair> betaLinks;
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
#if 0
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
#endif // 0
    return true;
  }
/*
  bool finalizeBetaBridgePartners()
  {
    ProteinBetaBridgesBuilder builder;
    ProteinBetaBridgesPtr betaBridges = builder.build(protein, betaBridgePartners);

    int betaSheetStart = -1;
    for (size_t i = 0; i < dsspSecondaryStructureSequence->size(); ++i)
    {
      if (dsspSecondaryStructureSequence->getIndex(i) == DSSPSecondaryStructureDictionary::extendedStrandInSheet)
      {
        if (betaSheetStart < 0)
          betaSheetStart = (int)i;
      }
      else
      {
        if (betaSheetStart >= 0)
        {
          std::cout << "BETA SHEET: " << betaSheetStart + 1 << " - " << i << std::endl;
          betaSheetStart = -1;
        }
      }
    }
    return betaBridges;
  }*/

  virtual bool parseEnd()
  {
    if (!serialNumber)
    {
      Object::error(T("DSSPFileParser::parseEnd"), T("No residues in dssp file"));
      return false;
    }
 //   if (!finalizeBetaBridgePartners())
 //     return false;

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
