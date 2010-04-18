/*-----------------------------------------.---------------------------------.
| Filename: CASPFileGenerator.cpp          | CASP Prediction Files generator |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 17:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "CASPFileGenerator.h"
#include "PDBFileGenerator.h"
#include "SecondaryStructureDictionary.h"
using namespace lbcpp;

/*
** CASPFileGenerator
*/
CASPFileGenerator::CASPFileGenerator(const File& file, const String& method)
  : TextObjectPrinter(file), method(method) {}

void CASPFileGenerator::consume(ObjectPtr object)
{
  ProteinPtr protein = object.dynamicCast<Protein>();
  jassert(protein);

  printRecord(T("PFRMAT"), getFormatSpecificationCode());
  printRecord(T("TARGET"), protein->getName());
  printRecord(T("AUTHOR"), T("ULg-GIGA")); // T("9344-7768-6843"));
  printMultiLineRecord(T("REMARK"), T("This is a prediction of server ULg-GIGA made at ") + Time::getCurrentTime().toString(true, true, true, true));
  printMultiLineRecord(T("METHOD"), method);
  printRecord(T("MODEL"), T("1"));
  printPredictionData(protein);
  print("END", true);
}

void CASPFileGenerator::printRecord(const String& keyword, const String& data)
{
  String key = keyword;
  while (key.length() < 6)
    key += T(" ");
  print(key + T(" ") + data + T("\n"));
}

void CASPFileGenerator::printMultiLineRecord(const String& keyword, const String& text)
{
  int b = 0;
  int n = text.indexOfChar(0, '\n');
  while (n >= 0)
  {
    printMultiLineRecordBase(keyword, text.substring(b, n));
    b = n + 1;
    n = text.indexOfChar(b, '\n');
  }
  if (b < text.length())
    printMultiLineRecordBase(keyword, text.substring(b));
}

void CASPFileGenerator::printMultiLineRecordBase(const String& keyword, const String& text)
{
  if (text.isEmpty())
    return;

  String line = keyword;
  while (line.length() < 6)
    line += T(" ");
  line += T(" ");

  int remainingCharacters = maxColumns - line.length();
  if (text.length() <= remainingCharacters)
    print(line + text, true);
  else
  {
    int i;
    for (i = remainingCharacters - 1; i >= 0; --i)
      if (text[i] == ' ' || text[i] == '\t')
        break;
    if (i < 0)
    {
      // could not find a space to break the line, force-break
      print(line + text.substring(0, remainingCharacters - 1) + T("-"), true);
      printMultiLineRecordBase(keyword, text.substring(remainingCharacters - 1));
    }
    else
    {
      print(line + text.substring(0, i), true);
      printMultiLineRecordBase(keyword, text.substring(i + 1));
    }
  }
}

/*
** TertiaryStructureCASPFileGenerator
*/
class TertiaryStructureCASPFileGenerator : public CASPFileGenerator
{
public:
  TertiaryStructureCASPFileGenerator(const File& file, const String& method)
    : CASPFileGenerator(file, method) {}

  virtual String getFormatSpecificationCode() const
    {return T("TS");}

  virtual void printPredictionData(ProteinPtr protein)
  {
    printRecord(T("PARENT"), T("N/A"));

    ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
    jassert(tertiaryStructure);
    size_t atomNumber = 1;
    for (size_t i = 0; i < tertiaryStructure->size(); ++i)
    {
      ProteinResiduePtr residue = tertiaryStructure->getResidue(i);
      for (size_t j = 0; j < residue->getNumAtoms(); ++j)
        printAtom(residue, residue->getAtom(j), i + 1, atomNumber++);
    }

    print(T("TER"), true);
  }


  void printAtom(ProteinResiduePtr residue, ProteinAtomPtr atom, size_t residueNumber, size_t atomNumber)
  {
    String line = PDBFileGenerator::makeAtomLine(atomNumber, atom->getName(), residue->getName(),
                  String::empty, residueNumber, atom->getX(), atom->getY(), atom->getZ(),
                      1.0, -1.0, String::empty, String::empty, String::empty);
    print(line, true);
  }
};

ObjectConsumerPtr lbcpp::caspTertiaryStructureFileGenerator(const File& file, const String& method)
  {return new TertiaryStructureCASPFileGenerator(file, method);}

/*
** ResidueResidueDistanceCASPFileGenerator
*/
class ResidueResidueDistanceCASPFileGenerator : public CASPFileGenerator
{
public:
  ResidueResidueDistanceCASPFileGenerator(const File& file, const String& method)
    : CASPFileGenerator(file, method) {}

  virtual String getFormatSpecificationCode() const
    {return T("RR");}

  virtual void printPredictionData(ProteinPtr protein)
  {
    size_t n = protein->getLength();

    LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
    printAminoAcidSequence(aminoAcidSequence);
    
    ScoreSymmetricMatrixPtr residueResidueContactProbabilityMatrix = protein->getResidueResidueContactProbabilityMatrix();
    jassert(residueResidueContactProbabilityMatrix);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 1; j < n; ++j)
      {
        double probability = residueResidueContactProbabilityMatrix->getScore(i, j);
        jassert(probability >= 0.0 && probability <= 1.0);
        print(lbcpp::toString(i + 1) + T(" ") + lbcpp::toString(j + 1) + T(" 0 8 ") + String(probability, 2), true);
      }
  }

  void printAminoAcidSequence(LabelSequencePtr aminoAcidSequence)
  {
    enum {numAminoAcidsPerLine = 50};

    size_t begin = 0;
    size_t length = aminoAcidSequence->size();
    while (begin < length)
    {
      size_t end = begin + numAminoAcidsPerLine;
      if (end > length)
        end = length;

      String line;
      for (size_t i = begin; i < end; ++i)
        line += aminoAcidSequence->getString(i);
      jassert(line.length() <= numAminoAcidsPerLine);
      print(line, true);

      begin = end;
    }
  }
};

ObjectConsumerPtr lbcpp::caspResidueResidueDistanceFileGenerator(const File& file, const String& method)
  {return new ResidueResidueDistanceCASPFileGenerator(file, method);}

/*
** OrderDisorderRegionCASPFileGenerator
*/
class OrderDisorderRegionCASPFileGenerator : public CASPFileGenerator
{
public:
  OrderDisorderRegionCASPFileGenerator(const File& file, const String& method)
    : CASPFileGenerator(file, method) {}

  virtual String getFormatSpecificationCode() const
    {return T("DR");}

  virtual void printPredictionData(ProteinPtr protein)
  {
    size_t n = protein->getLength();

    LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
    ScoreVectorSequencePtr orderDisorderScoreSequence = protein->getOrderDisorderScoreSequence();
    jassert(orderDisorderScoreSequence);
    
    for (size_t i = 0; i < n; ++i)
    {
      double orderProbability = orderDisorderScoreSequence->getScore(i, OrderDisorderDictionary::order); 
      double disorderProbability = orderDisorderScoreSequence->getScore(i, OrderDisorderDictionary::disorder); 
      jassert(fabs(1.0 - (orderProbability + disorderProbability)) < 0.00001);
      print(aminoAcidSequence->getString(i) + T(" ") + 
        (orderProbability >= disorderProbability ? T("O") : T("D")) + T(" ") + 
        String(disorderProbability, 2), true);
    }
  }
};

ObjectConsumerPtr lbcpp::caspOrderDisorderRegionFileGenerator(const File& file, const String& method)
  {return new OrderDisorderRegionCASPFileGenerator(file, method);}
