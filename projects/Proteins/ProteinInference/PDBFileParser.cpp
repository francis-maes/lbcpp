/*-----------------------------------------.---------------------------------.
| Filename: PDBFileParser.cpp              | PDB File Parser                 |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 18:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "PDBFileParser.h"
using namespace lbcpp;

PDBFileParser::PDBFileParser(const File& file)
  : TextObjectParser(file)
{
}

void PDBFileParser::parseBegin()
{
  currentSeqResSerialNumber = 0;
  currentModelSerialNumber = 1;
  currentAtomSerialNumber = 0;
  currentResidueSerialNumber = 0;
  currentResidueInsertionCode = (char)0;
}

bool PDBFileParser::parseLine(const String& line)
{
  if (line.isEmpty())
    return true; // ignore empty lines
  if (line.startsWith(T("END")))
    return true; // last line

  jassert(line.length() >= 6);

  String keyword = getSubString(line, 1, 6).trim();
  if (keyword == T("HEADER"))
    return parseHeaderLine(line);
  else if (keyword == T("SEQRES"))
    return parseSeqResLine(line);
  else if (keyword == T("MODRES"))
    return true; // FIXME ?
  else if (keyword == T("HET") || keyword == T("HETNAM") || keyword == T("HETSYN") || keyword == T("FORMUL") || keyword == T("SEQADV"))
    return true;
  else if (keyword == T("HELIX") || keyword == T("SHEET") || keyword == T("TURN") ||
           keyword == T("SSBOND") || keyword == T("CISPEP") || keyword == T("LINK") || keyword == T("SITE"))
    return true; // FIXME: parse secondary structure
  else if (keyword == T("CRYST1") || keyword.substring(0, 5) == T("ORIGX") || keyword.substring(0, 5) == T("SCALE") || keyword.substring(0, 5) == T("MTRIX"))
    return true; // FIXME
  else if (keyword == T("MODEL"))
    return parseModelLine(line);
  else if (keyword == T("ATOM"))
    return currentModelSerialNumber != 1 || parseAtomLine(line);
  else if (keyword == T("TER"))
    return currentModelSerialNumber != 1 || parseTerLine(line);
  else if (keyword == T("HETATM"))
    return currentModelSerialNumber != 1 || parseHetAtomLine(line);
  else if (keyword == T("ANISOU") || keyword == T("CONECT") || keyword == T("MASTER"))
    return true;
  else if (keyword == T("REMARK") || keyword == T("COMPND") || keyword == T("SOURCE") ||
           keyword == T("AUTHOR") || keyword == T("REVDAT") || keyword == T("TITLE") ||
           keyword == T("KEYWDS") || keyword == T("JRNL") || keyword == T("DBREF") ||
           keyword == T("NUMMDL") || keyword == T("EXPDTA") || keyword == T("MDLTYP") ||
           keyword == T("SPRSDE"))
    return true; // skip line

  Object::warning(T("PDBFileParser::parseLine"), T("Unknown keyword ") + keyword);
  return true;
}

bool PDBFileParser::parseHeaderLine(const String& line)
{
  proteinName = getSubString(line, 63, 66);
  return true;
}

bool PDBFileParser::parseSeqResLine(const String& line)
{
  // parse serial number and chain id
  int serialNumber;
  if (!getInteger(line, 8, 10, serialNumber))
    return false;
  char chainID;
  if (!getChainId(line, 12, chainID))
    return false;

  // quickly parse amino acids to see if we are well using three letter codes
  enum {numAminoAcidsPerLine = 13};
  int aminoAcidCodesLength = 0;
  for (size_t i = 0; i < numAminoAcidsPerLine; ++i)
  {
    int firstColumn = 20 + i * 4;
    String aminoAcidCode = getSubString(line, firstColumn, firstColumn + 2).trim();
    if (aminoAcidCode.isEmpty())
      break;

    if (i == 0)
      aminoAcidCodesLength = aminoAcidCode.length();
    else if (aminoAcidCodesLength != aminoAcidCode.length())
    {
      Object::error(T("PDBFileParser::parseSeqResLine"), T("Not all the amino acids have the same code length"));
      return false;
    }
  }
  if (aminoAcidCodesLength < 3)
    return true; // skip line

  // parse and check num residues
  int numResidues;
  if (!getInteger(line, 14, 17, numResidues))
    return false;
  if (numResidues <= 0)
  {
    Object::error(T("PDBFileParser::parseSeqResLine"), T("Invalid number of residues: ") + lbcpp::toString(serialNumber));
    return false;
  }
  if (numResidues < 10)
    return true; // skip chains of less than 10 residues

  LabelSequencePtr aminoAcidSequence;

  // create protein if not done yet
  ProteinPtr protein = proteins[chainID];
  if (!protein)
  {
    jassert(serialNumber == 1);
    protein = (proteins[chainID] = new Protein(proteinName));
    aminoAcidSequence = new LabelSequence(T("AminoAcidSequence"), AminoAcidDictionary::getInstance());
    protein->setObject(aminoAcidSequence);
    currentSeqResSerialNumber = serialNumber;
  }
  else
  {
    // check serial number
    ++currentSeqResSerialNumber;
    if (serialNumber != currentSeqResSerialNumber)
    {
      Object::error(T("PDBFileParser::parseSeqResLine"), T("Invalid serial number: ") + lbcpp::toString(serialNumber));
      return false;
    }
    aminoAcidSequence = protein->getAminoAcidSequence();
  }

 

  // parse amino acids
  for (size_t i = 0; i < numAminoAcidsPerLine; ++i)
  {
    int firstColumn = 20 + i * 4;
    String aminoAcidCode = getSubString(line, firstColumn, firstColumn + 2);
    if (aminoAcidCode == T("   "))
      break;
    
    AminoAcidDictionary::Type aminoAcid = AminoAcidDictionary::getTypeFromThreeLettersCode(aminoAcidCode);
    if (aminoAcid == AminoAcidDictionary::unknown)
    {
      static const juce::tchar* hetResidueCodes[] = {T("NH2")};
      bool isHetResidue = false;
      for (size_t j = 0; j < sizeof (hetResidueCodes) / sizeof (const juce::tchar* ); ++j)
        if (aminoAcidCode == hetResidueCodes[j])
        {
          isHetResidue = true;
          break;
        }
      if (isHetResidue)
        continue;
      else
      {
        Object::error(T("PDBFileParser::parseSeqResLine"), T("Unreconized amino acid code: ") + aminoAcidCode);
        return false;
      }
    }
    aminoAcidSequence->append(aminoAcid);
  }
  return true;
}

bool PDBFileParser::parseModelLine(const String& line)
{
  int modelSerialNumber;
  if (!getInteger(line, 11, 14, modelSerialNumber))
    return false;
  if (modelSerialNumber <= 0)
  {
    Object::error(T("PDBFileParser::parseModelLine"), T("Invalid model serial number"));
    return false;
  }
  currentModelSerialNumber = modelSerialNumber;
  return true;
}

bool PDBFileParser::parseAtomLine(const String& line)
{
  if (!parseAndCheckAtomSerialNumber(line, 7, 11))
    return false;

  // parse atom name, residue name
  String atomName = getSubString(line, 13, 16).trim();
  String residueName = getSubString(line, 18, 20);

  // retrieve protein from chain id
  ProteinPtr protein = getProteinFromChainId(line, 22);
  if (!protein)
    return true; // skip this chain

  // parse residue sequence number and insertion code
  int residueSequenceNumber;
  char residueInsertionCode;
  if (!getInteger(line, 23, 26, residueSequenceNumber) || !getChar(line, 27, residueInsertionCode))
    return false;

  // create the tertiary structure if not done yet
  ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
  if (!tertiaryStructure)
  {
    tertiaryStructure = new ProteinTertiaryStructure(0);
    protein->setObject(tertiaryStructure);
    currentResidueIndex = 0;
    currentResidueSerialNumber = residueSequenceNumber;
    currentResidueInsertionCode = residueInsertionCode;
  }

  // update currentResidueIndex
  if (currentResidueSerialNumber != residueSequenceNumber || currentResidueInsertionCode != residueInsertionCode)
  {
    ++currentResidueIndex;
    if (currentResidueIndex >= protein->getLength())
    {
      Object::error(T("PDBFileParser::parseAtomLine"), T("Too many residues"));
      return false;
    }

    if (residueSequenceNumber < currentResidueSerialNumber)
    {
      Object::error(T("PDBFileParser::parseAtomLine"), T("Residue sequence number are misordered"));
      return false;
    }
    currentResidueSerialNumber = residueSequenceNumber;
    currentResidueInsertionCode = residueInsertionCode;
  }

  // check amino acid code
  AminoAcidDictionary::Type aminoAcid = AminoAcidDictionary::getTypeFromThreeLettersCode(residueName);
  if ((int)aminoAcid > (int)AminoAcidDictionary::numAminoAcids)
  {
    Object::error(T("PDBFileParser::parseAtomLine"), T("Invalid residue name: ") + lbcpp::toString(residueName));
    return false;
  }

  // retrieve residue or create a new one
  ProteinResiduePtr residue;
  if (currentResidueIndex < tertiaryStructure->size())
  {
    residue = tertiaryStructure->getResidue(currentResidueIndex);
    if (residue->getAminoAcid() != aminoAcid)
    {
      Object::error(T("PDBFileParser::parseAtomLine"), T("Unconsistent residue name: ") + lbcpp::toString(residueName));
      return false;
    }
  }
  else
  {
    // check previous residue if it exists
    if (tertiaryStructure->size())
    {
      ProteinResiduePtr residue = tertiaryStructure->getLastResidue();
      if (!residue->getCAlphaAtom())
      {
        Object::error(T("PDBFileParser::parseAtomLine"), T("Missing C-alpha atom in residue ") + residue->getName());
        return false;
      }
      bool hasOnlyCAlpha = residue->getNumAtoms() == 1;
      if (!hasOnlyCAlpha && (!residue->getCarbonAtom() || !residue->getNitrogenAtom()))
      {
        Object::error(T("PDBFileParser::parseAtomLine"), T("Missing carbon or nitrogen atom in residue ") + residue->getName());
        return false;
      }
    }
    residue = new ProteinResidue(aminoAcid);
    tertiaryStructure->append(residue);
  }

  // create atom
  String symbolElement = getSubString(line, 77, 78);
  if (symbolElement[0] < 'A' || symbolElement[0] > 'Z')
    symbolElement = String::empty; // in old formats, the symbol element is not specified
  ProteinAtomPtr atom = new ProteinAtom(atomName, symbolElement);

  // parse atom
  double x, y, z, occupancy, temperatureFactor;
  if (!getDouble(line, 31, 38, x) ||
      !getDouble(line, 39, 46, y) ||
      !getDouble(line, 47, 54, z) ||
      !getDouble(line, 55, 60, occupancy) ||
      !getDouble(line, 61, 66, temperatureFactor))
    return false;
  atom->setPosition(Vector3(x, y, z));
  atom->setOccupancy(occupancy);
  atom->setTemperatureFactor(temperatureFactor);
  residue->addAtom(atom);
  return true;
}

bool PDBFileParser::parseTerLine(const String& line)
{
  if (!parseAndCheckAtomSerialNumber(line, 7, 11))
    return false;

  // retrieve protein from chain id
  ProteinPtr protein = getProteinFromChainId(line, 22);
  if (!protein)
    return true; // skip this chain

  ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
  if (!tertiaryStructure || !tertiaryStructure->size())
  {
    Object::error(T("PDBFileParser::parseTerLine"), T("No tertiary structure"));
    return false;
  }
  
  // parse residue sequence number and insertion code
  int residueSequenceNumber;
  char residueInsertionCode;
  if (!getInteger(line, 23, 26, residueSequenceNumber) || !getChar(line, 27, residueInsertionCode))
    return false;
  if (currentResidueSerialNumber == residueSequenceNumber &&
      currentResidueInsertionCode == residueInsertionCode)
  {
    // remove the last residue if it is not complete
    ProteinResiduePtr lastResidue = tertiaryStructure->getLastResidue();
    if (!lastResidue->getCAlphaAtom())
      tertiaryStructure->resize(tertiaryStructure->size() - 1);
  }

  currentResidueSerialNumber = 0;
  currentResidueInsertionCode = (char)0;
  return true;
}

bool PDBFileParser::parseHetAtomLine(const String& line)
{
  if (!parseAndCheckAtomSerialNumber(line, 7, 11))
    return false;
  return true;
}

ProteinPtr PDBFileParser::getProteinFromChainId(const String& line, int column)
{
  char chainId;
  if (!getChainId(line, 22, chainId))
    return false;
  ProteinMap::const_iterator it = proteins.find(chainId);
  return it == proteins.end() ? ProteinPtr() : it->second;
}

bool PDBFileParser::parseAndCheckAtomSerialNumber(const String& line, int firstColumn, int lastColumn)
{
  int serialNumber;
  if (!getInteger(line, firstColumn, lastColumn, serialNumber))
    return false;
  ++currentAtomSerialNumber;
  if (serialNumber != currentAtomSerialNumber)
  {
    Object::error(T("PDBFileParser::parseAtomLine"), T("Invalid serial number: ") + lbcpp::toString(serialNumber));
    return false;
  }
  return true;
}

bool PDBFileParser::parseEnd()
{
  if (!proteins.size())
  {
    Object::error(T("PDBFileParser::parseEnd"), T("No proteins PDB file"));
    return false;
  }

  for (ProteinMap::const_iterator it = proteins.begin(); it != proteins.end(); ++it)
  {
    ProteinPtr protein = it->second;
    ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
    if (!tertiaryStructure || !tertiaryStructure->size())
    {
      Object::error(T("PDBFileParser::parseEnd"), T("No tertiary structure for chain ") + lbcpp::toString(it->first));
      return false;
    }
    String failureReason;
    if (!tertiaryStructure->isConsistent(failureReason))
    {
      Object::error(T("PDBFileParser::parseEnd"), T("Tertiary structure is not consistent: ") + failureReason);
      return false;
    }
  }

  setResult(proteins.begin()->second);
  return true;
}

String PDBFileParser::getSubString(const String& line, int firstColumn, int lastColumn)
{
  jassert(firstColumn >= 1 && firstColumn <= 80 && lastColumn >= 1 && lastColumn <= 80);
  --firstColumn;
  --lastColumn;

  jassert(lastColumn >= firstColumn);
  String str;
  if (firstColumn < line.length())
  {
    int end = juce::jmin(line.length(), lastColumn + 1);
    str = line.substring(firstColumn, end);
  }
  int l = lastColumn - firstColumn + 1;
  while (str.length() < l)
    str += T(" ");
  return str;
}

bool PDBFileParser::getChar(const String& line, int column, char& result)
{
  String str = getSubString(line, column, column);
  if (str[0] > 127)
  {
    Object::error(T("PDBFileParser::getChar"), str + T(" is not a char"));
    return false;
  }
  result = (char)str[0];
  return true;
}

bool PDBFileParser::getInteger(const String& line, int firstColumn, int lastColumn, int& result)
{
  String str = getSubString(line, firstColumn, lastColumn).trim();
  if (!str.containsOnly(T("-0123456789")))
  {
    Object::error(T("PDBFileParser::getInteger"), str + T(" is not an integer"));
    return false;
  }
  result = str.getIntValue();
  return true;
}

bool PDBFileParser::getDouble(const String& line, int firstColumn, int lastColumn, double& result)
{
  String str = getSubString(line, firstColumn, lastColumn).trim();
  if (!str.containsOnly(T("e.-0123456789")))
  {
    Object::error(T("PDBFileParser::getDouble"), str + T(" is not an double"));
    return false;
  }
  result = str.getDoubleValue();
  return true;
}

bool PDBFileParser::getChainId(const String& line, int column, char& res) const
{
  if (!getChar(line, column, res))
    return false;
  if (res == ' ' && proteins.size() > 1)
  {
    Object::error(T("PDBFileParser::getChainId"), T("Empty chain ID"));
    return false;
  }
  if (res < 'A' && res > 'Z')
  {
    Object::error(T("PDBFileParser::getChainId"), T("Invalid chain ID: ") + lbcpp::toString(res));
    return false;
  }
  return true;
}
