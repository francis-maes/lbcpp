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
  protein = new Protein(T("PDB"));
  currentSeqResSerialNumber = 0;
  currentAtomSerialNumber = 0;
}

bool PDBFileParser::parseLine(const String& line)
{
  if (line.isEmpty())
    return true; // ignore empty lines

  jassert(line.length() >= 6);

  String keyword = getSubString(line, 1, 6);
  if (keyword == T("HEADER"))
    return parseHeaderLine(line);
  else if (keyword == T("SEQRES"))
    return parseSeqResLine(line);
  else if (keyword == T("HELIX") || keyword == T("SHEET") || keyword == T("TURN") || keyword == T("SSBOND"))
    return true; // FIXME: parse secondary structure
  else if (keyword == T("CRYST1") || keyword.substring(0, 5) == T("ORIGX") || keyword.substring(0, 5) == T("SCALE"))
    return true; // FIXME
  else if (keyword == T("ATOM  "))
    return parseAtomLine(line);
  else if (keyword == T("REMARK") || keyword == T("COMPND") || keyword == T("SOURCE") ||
           keyword == T("AUTHOR") || keyword == T("REVDAT"))
    return true; // skip line

  std::cerr << "Warning: unparsed line, keyword = " << keyword << std::endl;
  return true;
}

bool PDBFileParser::parseHeaderLine(const String& line)
{
  String idCode = getSubString(line, 63, 66);
  protein->setName(idCode);
  return true;
}

bool PDBFileParser::parseSeqResLine(const String& line)
{
  int serialNumber;
  if (!getInteger(line, 8, 10, serialNumber))
    return false;
  ++currentSeqResSerialNumber;
  if (serialNumber != currentSeqResSerialNumber)
  {
    Object::error(T("PDBFileParser::parseSeqResLine"), T("Invalid serial number: ") + lbcpp::toString(serialNumber));
    return false;
  }

  int numResidues;
  if (!getInteger(line, 14, 17, numResidues))
    return false;
  if (numResidues <= 0)
  {
    Object::error(T("PDBFileParser::parseSeqResLine"), T("Invalid number of residues: ") + lbcpp::toString(serialNumber));
    return false;
  }

  LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
  if (!aminoAcidSequence)
  {
    jassert(serialNumber == 1);
    aminoAcidSequence = new LabelSequence(T("AminoAcidSequence"), AminoAcidDictionary::getInstance(), (size_t)numResidues);
    protein->setObject(aminoAcidSequence);
  }
  
  enum {numAminoAcidsPerLine = 13};

  size_t aminoAcidPosition = (serialNumber - 1) * numAminoAcidsPerLine;
  for (size_t i = 0; i < numAminoAcidsPerLine; ++i, ++aminoAcidPosition)
  {
    int firstColumn = 20 + i * 4;
    String aminoAcidCode = getSubString(line, firstColumn, firstColumn + 2);
    if (aminoAcidCode == T("   "))
    {
      if (aminoAcidPosition != aminoAcidSequence->size())
      {
        Object::error(T("PDBFileParser::parseSeqResLine"), T("Incorrect count of SEQRES elements"));
        return false;
      }
      break;
    }
    AminoAcidDictionary::Type aminoAcid = AminoAcidDictionary::getTypeFromThreeLettersCode(aminoAcidCode);
    if (aminoAcid == AminoAcidDictionary::unknown)
    {
      Object::error(T("PDBFileParser::parseSeqResLine"), T("Unreconized amino acid code: ") + aminoAcidCode);
      return false;
    }
    aminoAcidSequence->setIndex(aminoAcidPosition, aminoAcid);
  }
  return true;
}

bool PDBFileParser::parseAtomLine(const String& line)
{
  LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
  if (!aminoAcidSequence)
  {
    Object::error(T("PDBFileParser::parseAtomLine"), T("Missing primary structure"));
    return false;
  }

  size_t n = protein->getLength();

  int serialNumber;
  if (!getInteger(line, 7, 11, serialNumber))
    return false;
  ++currentAtomSerialNumber;
  if (serialNumber != currentAtomSerialNumber)
  {
    Object::error(T("PDBFileParser::parseAtomLine"), T("Invalid serial number: ") + lbcpp::toString(serialNumber));
    return false;
  }
  
  String atomName = getSubString(line, 13, 16);
  String residueName = getSubString(line, 18, 20);

  int residueSequenceNumber;
  if (!getInteger(line, 23, 26, residueSequenceNumber))
    return false;
  if (residueSequenceNumber < 1 || residueSequenceNumber > (int)n)
  {
    Object::error(T("PDBFileParser::parseAtomLine"), T("Invalid residue sequence number: ") + lbcpp::toString(residueSequenceNumber));
    return false;
  }

  size_t position = (size_t)(residueSequenceNumber - 1);
  AminoAcidDictionary::Type currentAminoAcid = (AminoAcidDictionary::Type)aminoAcidSequence->getIndex(position);
  if (residueName != AminoAcidDictionary::getThreeLettersCode(currentAminoAcid).toUpperCase())
  {
    Object::error(T("PDBFileParser::parseAtomLine"), T("Invalid residue name: ") + lbcpp::toString(residueSequenceNumber));
    return false;
  }
  
  ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
  if (!tertiaryStructure)
  {
    jassert(serialNumber == 1);
    tertiaryStructure = new ProteinTertiaryStructure(n);
    protein->setObject(tertiaryStructure);
  }

  ProteinResiduePtr residue = tertiaryStructure->getResidue(position);
  if (!residue)
  {
    residue = new ProteinResidue(currentAminoAcid);
    tertiaryStructure->setResidue(position, residue);
  }

  String symbolElement = getSubString(line, 77, 78);
  if (symbolElement[0] < 'A' || symbolElement[0] > 'Z')
    symbolElement = String::empty; // in old formats, the symbol element is not specified
  ProteinAtomPtr atom = new ProteinAtom(atomName, symbolElement);

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

bool PDBFileParser::parseEnd()
{
  setResult(protein);
  return true;
}

String PDBFileParser::getSubString(const String& line, int firstColumn, int lastColumn)
{
  jassert(firstColumn >= 1 && firstColumn <= 80 && lastColumn >= 1 && lastColumn <= 80);
  --firstColumn;
  --lastColumn;

  jassert(lastColumn > firstColumn);
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
