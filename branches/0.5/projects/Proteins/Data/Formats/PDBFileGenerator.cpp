/*-----------------------------------------.---------------------------------.
| Filename: PDBFileGenerator.cpp           | PDB File Generator              |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 17:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "PDBFileGenerator.h"
using namespace lbcpp;

PDBFileGenerator::PDBFileGenerator(ExecutionContext& context, const File& file)
  : TextPrinter(context, file) {}

PDBFileGenerator::PDBFileGenerator(ExecutionContext& context)
  : TextPrinter(new juce::MemoryOutputStream()) {}

void PDBFileGenerator::consume(ExecutionContext& context, const Variable& variable)
{
  const ProteinPtr& protein = variable.getObjectAndCast<Protein>(context);
  jassert(protein);
  size_t n = protein->getLength();

  VectorPtr primaryStructure = protein->getPrimaryStructure();
  jassert(primaryStructure);
  TertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
  jassert(tertiaryStructure);

  /*
  ** Header
  */
  String idCode = protein->getName();
  if (idCode.length() > 4)
    idCode = idCode.substring(0, 4);
  print(makeHeaderLine(String::empty, String::empty, idCode), true);

  /*
  ** Primary Structure
  */
  std::vector<String> residueNames(n);
  for (size_t i = 0; i < n; ++i)
    residueNames[i] = AminoAcid::toThreeLettersCode((AminoAcidType)primaryStructure->getElement(i).getInteger()).toUpperCase();
  size_t firstResidueIndex = 0;
  size_t seqResIndex = 1;
  while (firstResidueIndex < n)
    print(makeSeqResLine(seqResIndex++, String::empty, n, residueNames, firstResidueIndex), true);

  /*
  ** Tertiary Structure
  */
  size_t atomNumber = 1;
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue = tertiaryStructure->getResidue(i);
    if (residue)
      for (size_t j = 0; j < residue->getNumAtoms(); ++j)
      {
        AtomPtr atom = residue->getAtom(j);
        jassert(atom->getPosition().exists());
        jassert(isNumberValid(atom->getX()) && isNumberValid(atom->getY()) && isNumberValid(atom->getZ()));
        print(makeAtomLine(atomNumber++, atom->getName(), residue->getThreeLettersCodeName(), String::empty,
          i + 1, atom->getX(), atom->getY(), atom->getZ(), atom->getOccupancy(), atom->getTemperatureFactor(),
          String::empty, atom->getElementSymbol(), String::empty), true);
      }
  }

  print(makeEndLine(), true);
}

String PDBFileGenerator::producePDBString(ExecutionContext& context, ProteinPtr protein)
{
   PDBFileGenerator generator(context);
   generator.consume(context, protein);
   juce::MemoryOutputStream* mos = (juce::MemoryOutputStream* )generator.ostr;
   String pdbString(mos->getData());
   return pdbString;
}

String PDBFileGenerator::makeHeaderLine(const String& classification, const String& date, const String& idCode)
{
  String line = T("HEADER");                                      jassert(line.length() == 6);
  line += T("    ");                                              jassert(line.length() == 10);
  line += toFixedLengthStringLeftJustified(classification, 40);   jassert(line.length() == 50);
  line += toFixedLengthString(date, 9);                           jassert(line.length() == 59);
  line += T("   ");                                               jassert(line.length() == 62);
  line += toFixedLengthString(idCode, 4);                         jassert(line.length() == 66);
  return toFixedLengthStringLeftJustified(line, 80);
}

String PDBFileGenerator::makeSeqResLine(size_t serialNumber, const String& chainId, size_t numResidues,
                                        const std::vector<String>& residues, size_t& firstResidueIndex)
{
  String line = T("SEQRES");                                                jassert(line.length() == 6);
  line += T(" ");                                                           jassert(line.length() == 7);
  line += toFixedLengthStringRightJustified(String((int)serialNumber), 3);  jassert(line.length() == 10);
  line += T(" ");                                                           jassert(line.length() == 11);
  line += toFixedLengthString(chainId, 1);                                  jassert(line.length() == 12);
  line += T(" ");                                                           jassert(line.length() == 13);
  line += toFixedLengthStringRightJustified(String((int)numResidues), 4);   jassert(line.length() == 17);
  line += T("  ");                                                          jassert(line.length() == 19);

  for (size_t i = 0; i < 13; ++i)
  {
    if (firstResidueIndex < residues.size())
    {
      line += toFixedLengthString(residues[firstResidueIndex], 3);
      ++firstResidueIndex;
    }
    else
      line += T("   ");
    line += T(" ");
  }
  jassert(line.length() == 71);
  return toFixedLengthStringLeftJustified(line, 80);
}

String PDBFileGenerator::makeAtomLine(size_t atomNumber, const String& atomName, const String& residueName, const String& chainID,
  size_t residueNumber, double x, double y, double z, double occupancy, double temperatureFactor, const String& segmentIdentifier,
  const String& elementSymbol, const String& atomCharge)
{
  String line = T("ATOM  ");                                                    jassert(line.length() == 6);
  line += toFixedLengthStringRightJustified(String((int)atomNumber), 5);        jassert(line.length() == 11);
  line += T(" ");                                                               jassert(line.length() == 12);
  line += toFixedLengthString(atomName, 4);                                     jassert(line.length() == 16);
  line += T(" "); /* alternate location indicator */                            jassert(line.length() == 17);
  line += toFixedLengthString(residueName.toUpperCase(), 3);                    jassert(line.length() == 20);
  line += T(" ");                                                               jassert(line.length() == 21);
  line += toFixedLengthString(chainID, 1);                                      jassert(line.length() == 22);
  line += toFixedLengthStringRightJustified(String((int)residueNumber), 4);     jassert(line.length() == 26);
  line += T(" "); /* code for insertion of residues */                          jassert(line.length() == 27);
  line += T("   ");                                                             jassert(line.length() == 30);
  line += toFixedLengthStringRightJustified(String(x, 3), 8);                   jassert(line.length() == 38);
  line += toFixedLengthStringRightJustified(String(y, 3), 8);                   jassert(line.length() == 46);
  line += toFixedLengthStringRightJustified(String(z, 3), 8);                   jassert(line.length() == 54);
  line += toFixedLengthStringRightJustified(String(occupancy, 2), 6);           jassert(line.length() == 60);
  String tempFactor = temperatureFactor >= 0 ? String(temperatureFactor, 2) : String::empty;
  line += toFixedLengthStringRightJustified(tempFactor, 6);                     jassert(line.length() == 66);
  line += T("      ");                                                          jassert(line.length() == 72);
  line += toFixedLengthStringLeftJustified(segmentIdentifier, 4);               jassert(line.length() == 76);
  line += toFixedLengthStringRightJustified(elementSymbol, 2);                  jassert(line.length() == 78);
  line += toFixedLengthString(atomCharge, 2);                                   jassert(line.length() == 80);
  return line;
}

String PDBFileGenerator::makeEndLine()
  {return toFixedLengthStringLeftJustified(T("END"), 80);}

String PDBFileGenerator::toFixedLengthString(size_t i, int length)
  {return toFixedLengthString(String((int)i), length);}

String PDBFileGenerator::toFixedLengthString(const String& str, int length)
{
  jassert(str.length() <= length);
  String res = str;
  int i = 0;
  while (res.length() < length)
  {
    if (i % 2)
      res = T(" ") + res;
    else
      res = res + T(" ");
    ++i;
  }
  return res;
}

String PDBFileGenerator::toFixedLengthStringRightJustified(const String& str, int length)
{
  jassert(str.length() <= length);
  String res = str;
  while (res.length() < length)
    res = T(" ") + res;
  return res;
}

String PDBFileGenerator::toFixedLengthStringLeftJustified(const String& str, int length)
{
  jassert(str.length() <= length);
  String res = str;
  while (res.length() < length)
    res += T(" ");
  return res;
}
