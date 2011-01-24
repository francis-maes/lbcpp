/*-----------------------------------------.---------------------------------.
| Filename: PDBFileParser.cpp              | PDB File Parser                 |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 18:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "PDBFileParser.h"
using namespace lbcpp;

PDBFileParser::PDBFileParser(ExecutionContext& context, const File& file, bool beTolerant)
  : TextParser(context, file), beTolerant(beTolerant)
  {}

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
    return parseHeaderLine(context, line);
  else if (keyword == T("EXPDTA"))
    return parseExpDataLine(context, line);
  else if (keyword == T("REMARK"))
    return parseRemarkLine(context, line);
  else if (keyword == T("SEQRES"))
    return parseSeqResLine(context, line);
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
    return parseModelLine(context, line);
  else if (keyword == T("ATOM"))
    return currentModelSerialNumber != 1 || parseAtomLine(context, line);
  else if (keyword == T("TER"))
    return currentModelSerialNumber != 1 || parseTerLine(context, line);
  else if (keyword == T("HETATM"))
    return currentModelSerialNumber != 1 || parseHetAtomLine(context, line);
  else if (keyword == T("ANISOU") || keyword == T("CONECT") || keyword == T("MASTER"))
    return true;
  else if (keyword == T("COMPND") || keyword == T("SOURCE") ||
           keyword == T("AUTHOR") || keyword == T("REVDAT") || keyword == T("TITLE") ||
           keyword == T("KEYWDS") || keyword == T("JRNL") || keyword == T("DBREF") ||
           keyword == T("NUMMDL") || keyword == T("MDLTYP") || keyword == T("SPRSDE"))
    return true; // skip line

  context.warningCallback(T("PDBFileParser::parseLine"), T("Unknown keyword ") + keyword);
  return true;
}

bool PDBFileParser::parseHeaderLine(ExecutionContext& context, const String& line)
{
  //proteinName = getSubString(line, 63, 66);
  proteinName = getSubString(line, 63, 70).trim();
  return true;
}

bool PDBFileParser::parseExpDataLine(ExecutionContext& context, const String& line)
{
  experimentData = getSubString(line, 11, 79).trim();
  context.informationCallback(T("PDBFileParser::parseExpDataLine"), T("Protein ") + proteinName + T(" Experiment ") + experimentData);
  return true;
}

bool PDBFileParser::parseRemarkLine(ExecutionContext& context, const String& line)
{
  String remark = getSubString(line, 12, 70).trim();
  if (remark.startsWith(T("RESOLUTION.")))
  {
    int b = (int)strlen("RESOLUTION.");
    int n = remark.indexOf(b, T("ANGSTROMS."));
    if (n >= 0)
    {
      static const double highestTolerableResolution = 2.5;
      double resolution = remark.substring(b, n).trim().getDoubleValue();
      context.informationCallback(T("PDBFileParser::parseExpDataLine"), T("Protein ") + proteinName + T(" Resolution ") + String(resolution));
      if (!beTolerant && resolution > highestTolerableResolution)
      {
        context.errorCallback(T("PDBFileParser::parseRemarkLine"), T("Resolution ") + String(resolution) + T(" is not precise enough"));
        return false;
      }
    }
  }
  return true;
}

bool PDBFileParser::parseSeqResLine(ExecutionContext& context, const String& line)
{
  // parse serial number and chain id
  int serialNumber;
  if (!getInteger(context, line, 8, 10, serialNumber))
    return false;
  char chainID;
  if (!getChainId(context, line, 12, chainID))
    return false;

  // quickly parse amino acids to see if we are well using three letter codes
  enum {numAminoAcidsPerLine = 13};
  int aminoAcidCodesLength = 0;
  for (size_t i = 0; i < numAminoAcidsPerLine; ++i)
  {
    int firstColumn = 20 + (int)i * 4;
    String aminoAcidCode = getSubString(line, firstColumn, firstColumn + 2).trim();
    if (aminoAcidCode.isEmpty())
      break;

    if (i == 0)
      aminoAcidCodesLength = aminoAcidCode.length();
    else if (aminoAcidCodesLength != aminoAcidCode.length())
    {
      context.errorCallback(T("PDBFileParser::parseSeqResLine"), T("Not all the amino acids have the same code length"));
      return false;
    }
  }
  if (aminoAcidCodesLength < 3)
  {
    skippedChains.insert(chainID);
    return true; // skip line
  }

  // parse and check num residues
  int numResidues;
  if (!getInteger(context, line, 14, 17, numResidues))
    return false;
  if (numResidues <= 0)
  {
    context.errorCallback(T("PDBFileParser::parseSeqResLine"), T("Invalid number of residues: ") + String(numResidues));
    return false;
  }
  if (numResidues < 10)
    return true; // skip chains of less than 10 residues

  VectorPtr primaryStructure;

  // create protein if not done yet
  Chain& chain = chains[chainID];
  ProteinPtr protein = chain.protein;
  if (!protein)
  {
    jassert(serialNumber == 1);
    chain.protein = protein = new Protein(proteinName);
    primaryStructure = vector(aminoAcidTypeEnumeration);
    protein->setPrimaryStructure(primaryStructure);
    currentSeqResSerialNumber = serialNumber;
  }
  else
  {
    // check serial number
    ++currentSeqResSerialNumber;
    if (serialNumber != currentSeqResSerialNumber)
    {
      context.errorCallback(T("PDBFileParser::parseSeqResLine"), T("Invalid serial number: ") + String(serialNumber));
      return false;
    }
    primaryStructure = protein->getPrimaryStructure();
  }

  // parse amino acids
  for (size_t i = 0; i < numAminoAcidsPerLine; ++i)
  {
    int firstColumn = 20 + (int)i * 4;
    String aminoAcidCode = getSubString(line, firstColumn, firstColumn + 2);
    if (aminoAcidCode == T("   "))
      break;
    
    
    Variable aminoAcid = AminoAcid::fromThreeLettersCode(aminoAcidCode);
    if (!aminoAcid.exists())
    {
      static const juce::tchar* hetResidueCodes[] = {T("NH2"), T("ACE")};
      bool isHetResidue = true; // TODO: set to "true"
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
        context.errorCallback(T("PDBFileParser::parseSeqResLine"), T("Unreconized amino acid code: ") + aminoAcidCode);
        return false;
      }
    }
    primaryStructure->append(aminoAcid);
  }
  return true;
}

bool PDBFileParser::parseModelLine(ExecutionContext& context, const String& line)
{
  int modelSerialNumber;
  if (!getInteger(context, line, 11, 14, modelSerialNumber))
    return false;
  if (modelSerialNumber <= 0)
  {
    context.errorCallback(T("PDBFileParser::parseModelLine"), T("Invalid model serial number"));
    return false;
  }
  currentModelSerialNumber = modelSerialNumber;
  return true;
}

bool PDBFileParser::parseAtomLine(ExecutionContext& context, const String& line)
{
  if (!parseAndCheckAtomSerialNumber(context, line, 7, 11))
    return false;

  // parse atom name, residue name
  String atomName = getSubString(line, 13, 16).trim();
  String residueName = getSubString(line, 18, 20);

  // retrieve/create protein from chain id
  Chain* chain = getChain(context, line, 22);
  ProteinPtr protein;
  if (chain)
    protein = chain->protein;
  else
  {
    if (!beTolerant)
      return true; // skip all chains that do not have a corresponding SEQRES section
    char chainId;
    if (!getChainId(context, line, 22, chainId))
      return false;
    if (skippedChains.find(chainId) != skippedChains.end())
      return true; // skip this chain

    chain = &chains[chainId]; // create a new chain
    protein = (chain->protein = new Protein(proteinName));
  }

  // parse residue sequence number and insertion code
  int residueSequenceNumber;
  char residueInsertionCode;
  if (!getInteger(context, line, 23, 26, residueSequenceNumber) || !getChar(context, line, 27, residueInsertionCode))
    return false;

  // create a tertiary structure block if not done yet
  if (!chain->tertiaryStructureBlocks.size())
  {
    chain->tertiaryStructureBlocks.push_back(std::vector<ResiduePtr>());
    currentResidueIndex = 0;
    currentResidueSerialNumber = residueSequenceNumber;
    currentResidueInsertionCode = residueInsertionCode;
  }

  // update currentResidueIndex
  if (currentResidueSerialNumber != residueSequenceNumber || currentResidueInsertionCode != residueInsertionCode)
  {
    if (residueSequenceNumber < currentResidueSerialNumber)
    {
      context.errorCallback(T("PDBFileParser::parseAtomLine"), T("Residue sequence number are misordered (previous: ")
                            + String(residueSequenceNumber)
                            + T(", current: ")
                            + String(currentResidueSerialNumber)
                            + T(")"));
      return false;
    }

    if (residueSequenceNumber > currentResidueSerialNumber + 1)
    {
      // non contiguous, start a new tertiaryStructureBlock
      chain->tertiaryStructureBlocks.push_back(std::vector<ResiduePtr>());
      currentResidueIndex = 0;
    }
    else
      ++currentResidueIndex; // contiguous, just increment the current residue index

    currentResidueSerialNumber = residueSequenceNumber;
    currentResidueInsertionCode = residueInsertionCode;
  }

  // check amino acid code
  Variable aminoAcid = AminoAcid::fromThreeLettersCode(residueName);
  if (!aminoAcid.exists() || aminoAcid.getInteger() >= (int)AminoAcid::numStandardAminoAcid)
  {
    context.errorCallback(T("PDBFileParser::parseAtomLine"), T("Invalid residue name: ") + residueName);
    return false;
  }

  // retrieve residue or create a new one
  ResiduePtr residue;
  jassert(chain->tertiaryStructureBlocks.size());
  std::vector<ResiduePtr>& tertiaryStructureBlock = chain->tertiaryStructureBlocks.back();
  if (currentResidueIndex < tertiaryStructureBlock.size())
  {
    residue = tertiaryStructureBlock[currentResidueIndex];
    if (residue->getAminoAcidType() != aminoAcid.getInteger())
    {
      context.errorCallback(T("PDBFileParser::parseAtomLine"), T("Unconsistent residue name: ") + residueName);
      return false;
    }
  }
  else
  {
    if (tertiaryStructureBlock.size() && !checkResidueConsistency(context, tertiaryStructureBlock.back()))
      {} //return false; // TODO: remove this line and repace it by "return false
    residue = new Residue((AminoAcidType)aminoAcid.getInteger());
    tertiaryStructureBlock.push_back(residue);
  }

  // create atom
  String symbolElement = getSubString(line, 77, 78);
  if (symbolElement[0] < 'A' || symbolElement[0] > 'Z')
    symbolElement = String::empty; // in old formats, the symbol element is not specified
  AtomPtr atom = new Atom(atomName, symbolElement);

  // parse atom
  double x, y, z, occupancy, temperatureFactor;
  if (!getDouble(context, line, 31, 38, x) ||
      !getDouble(context, line, 39, 46, y) ||
      !getDouble(context, line, 47, 54, z) ||
      !getDouble(context, line, 55, 60, occupancy) ||
      !getDouble(context, line, 61, 66, temperatureFactor))
    return false;
  Vector3Ptr v = new Vector3();
  v->setX(x);
  v->setY(y);
  v->setZ(z);
  atom->setPosition(v);
  atom->setOccupancy(occupancy);
  atom->setTemperatureFactor(temperatureFactor);
  residue->addAtom(atom);
  return true;
}

bool PDBFileParser::parseTerLine(ExecutionContext& context, const String& line)
{
  if (!parseAndCheckAtomSerialNumber(context, line, 7, 11))
    return false;

  // retrieve protein from chain id
  Chain* chain = getChain(context, line, 22);
  if (!chain)
    return true; // skip this chain
  ProteinPtr protein = chain->protein;
  jassert(protein);

  if (!chain->tertiaryStructureBlocks.size())
  {
    context.errorCallback(T("PDBFileParser::parseTerLine"), T("No tertiary structure"));
    return false;
  }
 
/*
  // parse residue sequence number and insertion code
  int residueSequenceNumber;
  char residueInsertionCode;
  if (!getInteger(line, 23, 26, residueSequenceNumber) || !getChar(line, 27, residueInsertionCode))
    return false;
  if (currentResidueSerialNumber == residueSequenceNumber &&
      currentResidueInsertionCode == residueInsertionCode)
  {
    // remove the last residue if it is not complete
    std::vector<ProteinResidueAtomsPtr>& lastBlock = chain->tertiaryStructureBlocks.back();
    ProteinResidueAtomsPtr lastResidue = lastBlock.size() ? lastBlock.back() : ProteinResidueAtomsPtr();
    if (lastResidue && !lastResidue->getCAlphaAtom())
      lastBlock.pop_back();
  }
*/

  currentResidueSerialNumber = 0;
  currentResidueInsertionCode = (char)0;
  return true;
}

bool PDBFileParser::parseHetAtomLine(ExecutionContext& context, const String& line)
{
  if (!parseAndCheckAtomSerialNumber(context, line, 7, 11))
    return false;
  return true;
}

PDBFileParser::Chain* PDBFileParser::getChain(ExecutionContext& context, const String& line, int column)
{
  char chainId;
  if (!getChainId(context, line, 22, chainId))
    return false;
  ChainMap::iterator it = chains.find(chainId);
  return it == chains.end() ? NULL : &it->second;
}

bool PDBFileParser::parseAndCheckAtomSerialNumber(ExecutionContext& context, const String& line, int firstColumn, int lastColumn)
{
  int serialNumber;
  if (!getInteger(context, line, firstColumn, lastColumn, serialNumber))
    return false;
  ++currentAtomSerialNumber;
  currentAtomSerialNumber = serialNumber;
  if (serialNumber != currentAtomSerialNumber)
  {
    context.errorCallback(T("PDBFileParser::parseAtomLine"), T("Invalid serial number: ") + String(serialNumber));
    return false;
  }
  return true;
}

static String tertiaryStructureBlockToAminoAcidString(const std::vector<ResiduePtr>& residues)
{
  String res;
  for (size_t i = 0; i < residues.size(); ++i)
    res += aminoAcidTypeEnumeration->getOneLetterCode((size_t)residues[i]->getAminoAcidType());
  jassert((size_t) res.length() == residues.size());
  return res;
}

bool PDBFileParser::checkResidueConsistency(ExecutionContext& context, ResiduePtr residue)
{
  if (!residue->getCAlphaAtom())
  {
    context.errorCallback(T("PDBFileParser::checkResidueConsistency"), T("Missing C-alpha atom in residue ") + residue->getName());
    return false;
  }
  bool hasOnlyCAlpha = residue->getNumAtoms() == 1;
  if (!hasOnlyCAlpha && (!residue->getCarbonAtom() || !residue->getNitrogenAtom()))
  {
    context.errorCallback(T("PDBFileParser::checkResidueConsistency"), T("Missing carbon or nitrogen atom in residue ") + residue->getName());
    return false;
  }
  return true;
}

TertiaryStructurePtr PDBFileParser::finalizeChain(ExecutionContext& context, char chainId, ProteinPtr protein, const std::vector< std::vector<ResiduePtr> >& tertiaryStructureBlocks)
{
  if (!tertiaryStructureBlocks.size())
  {
    String str; str += chainId;
    context.errorCallback(T("PDBFileParser::finalizeChain"), T("No tertiary structure for chain ") + str);
    return TertiaryStructurePtr();
  }
  
  TertiaryStructurePtr tertiaryStructure;
  VectorPtr primaryStructure = protein->getPrimaryStructure();
  if (primaryStructure)
  {
    size_t n = primaryStructure->getNumElements();
    tertiaryStructure = new TertiaryStructure(n);

    String primaryAminoAcids = primaryStructure->toString();
    jassert((size_t)primaryAminoAcids.length() == n);

    // align each tertiary structure block with the primary sequence and fill the corresponding part of the tertiary structure
    int lastIndex = 0;
    for (size_t i = 0; i < tertiaryStructureBlocks.size(); ++i)
    {
      String blockAminoAcids = tertiaryStructureBlockToAminoAcidString(tertiaryStructureBlocks[i]);
      int index = primaryAminoAcids.indexOf(lastIndex, blockAminoAcids);
      if (index < 0)
      {
        context.errorCallback(T("PDBFileParser::finalizeChain"), T("Could not align tertiary structure block with primary structure"));
        return TertiaryStructurePtr();
      }
      const std::vector<ResiduePtr>& residues = tertiaryStructureBlocks[i];
      jassert(index <= (int)(tertiaryStructure->getNumResidues() - residues.size()));
      for (size_t j = 0; j < residues.size(); ++j)
        tertiaryStructure->setResidue((size_t)(index + j), residues[j]);
      lastIndex = index + blockAminoAcids.length();
    }
  }
  else
  {
    // no specified primary sequence => create the primary sequence from the tertiary structure
    if (tertiaryStructureBlocks.size() > 1)
    {
      context.errorCallback(T("PDBFileParser::finalizeChain"), T("Non contiguous tertiary structure blocks without primary structure"));
      return TertiaryStructurePtr();
    }
    size_t n = tertiaryStructureBlocks[0].size();
    tertiaryStructure = new TertiaryStructure(n);
    for (size_t i = 0; i < n; ++i)
      tertiaryStructure->setResidue(i, tertiaryStructureBlocks[0][i]);
    primaryStructure = tertiaryStructure->makePrimaryStructure();
    protein->setPrimaryStructure(primaryStructure);
  }
  protein->setTertiaryStructure(tertiaryStructure);

  if (!beTolerant)
    tertiaryStructure->pruneResiduesThatDoNotHaveCompleteBackbone();
  if (!tertiaryStructure->getNumSpecifiedResidues())
  {
    context.errorCallback(T("PDBFileParser::finalizeChain"), T("Not any complete residue in tertiary structure"));
    return TertiaryStructurePtr();
  }

  String failureReason;
  if (!tertiaryStructure->isConsistent(failureReason))
  {
    if (beTolerant || true) // TODO: remove the "true" condition
      context.warningCallback(T("PDBFileParser::finalizeChain"), T("Tertiary structure is not consistent: ") + failureReason);
    else
    {
      context.errorCallback(T("PDBFileParser::finalizeChain"), T("Tertiary structure is not consistent: ") + failureReason);
      return TertiaryStructurePtr();
    }
  }
  jassert(tertiaryStructure);
  return tertiaryStructure;
}

VectorPtr PDBFileParser::finalizeDisorderSequence(ProteinPtr protein)
{
  if (experimentData != T("X-RAY DIFFRACTION"))
    return VectorPtr(); // for the moment, disorder regions are only determined for X-ray diffraction models

  TertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
  if (!tertiaryStructure)
    return VectorPtr();
  
  size_t n = tertiaryStructure->getNumResidues();

  // an element is in disorder if the associated residue is not defined
  VectorPtr res = protein->createEmptyProbabilitySequence();
  for (size_t i = 0; i < n; ++i)
    res->setElement(i, Variable(tertiaryStructure->getResidue(i) == ResiduePtr() ? 1.0 : 0.0, probabilityType));

  // remove disorder segments whose length is less than 4
  static const int minimumDisorderLength = 4;
  for (size_t i = 0; i < n; )
  {
    if (res->getElement(i).getDouble() == 1.0)
    {
      size_t j = i + 1;
      while (j < n && res->getElement(j).getDouble() == 1.0) ++j;
      if ((j - i) < (size_t)minimumDisorderLength)
        for (size_t ii = i; ii < j; ++ii)
          res->setElement(ii, Variable(0.0, probabilityType));
      i = j;
    }
    else
      ++i;
  }

  //std::cout << "Disorder sequence: " << res->toString() << std::endl;
  return res;
}

bool PDBFileParser::parseEnd()
{
  if (!chains.size())
  {
    context.errorCallback(T("PDBFileParser::parseEnd"), T("No chains in PDB file"));
    return false;
  }

  for (ChainMap::const_iterator it = chains.begin(); it != chains.end(); ++it)
  {
    ProteinPtr protein = it->second.protein;
    TertiaryStructurePtr tertiaryStructure = finalizeChain(context, it->first, protein, it->second.tertiaryStructureBlocks);
    if (!tertiaryStructure)
    {
      context.errorCallback(T("PDBFileParser::parseEnd"), T("No tertiary structure"));
      return false;
    }
    protein->setTertiaryStructure(tertiaryStructure);
    /* Fonction has been transfered into computeMissingVariable
    VectorPtr disorderSequence = finalizeDisorderSequence(protein);
    if (disorderSequence)
      protein->setDisorderRegions(disorderSequence);
    */
  }

  setResult(chains.begin()->second.protein);
  return true;
}

std::vector<ProteinPtr> PDBFileParser::getAllChains() const
{
  std::vector<ProteinPtr> res;
  res.reserve(chains.size());
  for (ChainMap::const_iterator it = chains.begin(); it != chains.end(); ++it)
    res.push_back(it->second.protein);
  return res;
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

bool PDBFileParser::getChar(ExecutionContext& context, const String& line, int column, char& result)
{
  String str = getSubString(line, column, column);
  if (str[0] > 127)
  {
    context.errorCallback(T("PDBFileParser::getChar"), str + T(" is not a char"));
    return false;
  }
  result = (char)str[0];
  return true;
}

bool PDBFileParser::getInteger(ExecutionContext& context, const String& line, int firstColumn, int lastColumn, int& result)
{
  String str = getSubString(line, firstColumn, lastColumn).trim();
  if (!str.containsOnly(T("-0123456789")))
  {
    context.errorCallback(T("PDBFileParser::getInteger"), str + T(" is not an integer"));
    return false;
  }
  result = str.getIntValue();
  return true;
}

bool PDBFileParser::getDouble(ExecutionContext& context, const String& line, int firstColumn, int lastColumn, double& result)
{
  String str = getSubString(line, firstColumn, lastColumn).trim();
  if (!str.containsOnly(T("e.-0123456789")))
  {
    context.errorCallback(T("PDBFileParser::getDouble"), str + T(" is not an double"));
    return false;
  }
  result = str.getDoubleValue();
  return true;
}

bool PDBFileParser::getChainId(ExecutionContext& context, const String& line, int column, char& res) const
{
  if (!getChar(context, line, column, res))
    return false;
  if (res == ' ' && chains.size() > 1)
  {
    context.errorCallback(T("PDBFileParser::getChainId"), T("Empty chain ID"));
    return false;
  }
  if (res < 'A' && res > 'Z')
  {
    String str; str += res;
    context.errorCallback(T("PDBFileParser::getChainId"), T("Invalid chain ID: ") + str);
    return false;
  }
  return true;
}
