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
  else if (keyword == T("HET") || keyword == T("HETNAM") || keyword == T("FORMUL"))
    return true;
  else if (keyword == T("HELIX") || keyword == T("SHEET") || keyword == T("TURN") ||
           keyword == T("SSBOND") || keyword == T("CISPEP") || keyword == T("LINK") || keyword == T("SITE"))
    return true; // FIXME: parse secondary structure
  else if (keyword == T("CRYST1") || keyword.substring(0, 5) == T("ORIGX") || keyword.substring(0, 5) == T("SCALE"))
    return true; // FIXME
  else if (keyword == T("MODEL"))
    return parseModelLine(line);
  else if (keyword == T("ATOM"))
    return currentModelSerialNumber != 1 || parseAtomLine(line);
  else if (keyword == T("HETATM") || keyword == T("CONECT") || keyword == T("MASTER"))
    return true;
  else if (keyword == T("TER"))
    return parseTerLine(line);
  else if (keyword == T("REMARK") || keyword == T("COMPND") || keyword == T("SOURCE") ||
           keyword == T("AUTHOR") || keyword == T("REVDAT") || keyword == T("TITLE") ||
           keyword == T("KEYWDS") || keyword == T("JRNL") || keyword == T("DBREF") ||
           keyword == T("NUMMDL") || keyword == T("EXPDTA"))
    return true; // skip line

  std::cerr << "Warning: unparsed line, keyword = " << keyword << std::endl;
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

  // parse and check num residues
  int numResidues;
  if (!getInteger(line, 14, 17, numResidues))
    return false;
  if (numResidues <= 0)
  {
    Object::error(T("PDBFileParser::parseSeqResLine"), T("Invalid number of residues: ") + lbcpp::toString(serialNumber));
    return false;
  }

  // parse amino acids
  enum {numAminoAcidsPerLine = 13};
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
  // parse and check atom serial number
  int serialNumber;
  if (!getInteger(line, 7, 11, serialNumber))
    return false;
  ++currentAtomSerialNumber;
  if (serialNumber != currentAtomSerialNumber)
  {
    Object::error(T("PDBFileParser::parseAtomLine"), T("Invalid serial number: ") + lbcpp::toString(serialNumber));
    return false;
  }

  // parse atom name, residue name
  String atomName = getSubString(line, 13, 16).trim();
  String residueName = getSubString(line, 18, 20);

  // retrieve protein from chain id
  char chainId;
  if (!getChainId(line, 22, chainId))
    return false;
  ProteinPtr protein = proteins[chainId];
  if (!protein)
  {
    Object::error(T("PDBFileParser::parseAtomLine"), T("Invalid chain id: ") + lbcpp::toString(chainId));
    return false;
  }
  LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
  if (!aminoAcidSequence)
  {
    Object::error(T("PDBFileParser::parseAtomLine"), T("Missing primary structure"));
    return false;
  }
  size_t n = aminoAcidSequence->size();

  // parse residue sequence number and insertion code
  int residueSequenceNumber;
  char residueInsertionCode;
  if (!getInteger(line, 23, 26, residueSequenceNumber) || !getChar(line, 27, residueInsertionCode))
    return false;

  // create the tertiary structure if not done yet
  ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
  if (!tertiaryStructure)
  {
    jassert(serialNumber == 1);
    tertiaryStructure = new ProteinTertiaryStructure(n);
    protein->setObject(tertiaryStructure);
    currentResidueIndex = 0;
    currentResidueSerialNumber = residueSequenceNumber;
    currentResidueInsertionCode = residueInsertionCode;
  }

  // update currentResidueIndex
  if (currentResidueSerialNumber != residueSequenceNumber || currentResidueInsertionCode != residueInsertionCode)
  {
    ++currentResidueIndex;
    if (currentResidueIndex >= n)
    {
      Object::error(T("PDBFileParser::parseAtomLine"), T("Too many residues"));
      return false;
    }

    if (residueSequenceNumber < currentResidueSerialNumber ||
      (residueSequenceNumber == currentResidueSerialNumber && residueInsertionCode < currentResidueInsertionCode))
    {
      Object::error(T("PDBFileParser::parseAtomLine"), T("Residue sequence number are misordered"));
      return false;
    }
    currentResidueSerialNumber = residueSequenceNumber;
    currentResidueInsertionCode = residueInsertionCode;
  }

  // check amino acid code
  AminoAcidDictionary::Type currentAminoAcid = (AminoAcidDictionary::Type)aminoAcidSequence->getIndex(currentResidueIndex);
  if (residueName != AminoAcidDictionary::getThreeLettersCode(currentAminoAcid).toUpperCase())
  {
    Object::error(T("PDBFileParser::parseAtomLine"), T("Residue name does not match with primary structure: ") + lbcpp::toString(residueName));
    return false;
  }

  // create residue if not done yet
  ProteinResiduePtr residue = tertiaryStructure->getResidue(currentResidueIndex);
  if (!residue)
  {
    residue = new ProteinResidue(currentAminoAcid);
    tertiaryStructure->setResidue(currentResidueIndex, residue);
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
  currentResidueSerialNumber = 0;
  currentResidueInsertionCode = (char)0;
  currentAtomSerialNumber = 0;
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
    if (!protein->getTertiaryStructure())
    {
      Object::error(T("PDBFileParser::parseEnd"), T("No tertiary structure for chain ") + lbcpp::toString(it->first));
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
  if (res - 'A' > (int)proteins.size())
  {
    Object::error(T("PDBFileParser::getChainId"), T("Non continguous chain IDs"));
    return false;
  }
  return true;
}
