/*-----------------------------------------.---------------------------------.
| Filename: CASPPredictor.cpp              | CASP Predictor                  |
| Author  : Francis Maes                   |                                 |
| Started : 17/04/2010 14:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../ProteinInference/Protein.h"
#include "../ProteinInference/AminoAcidDictionary.h"
#include "../ProteinInference/SecondaryStructureDictionary.h"
using namespace lbcpp;

extern void declareProteinClasses();

class CASPFileGenerator : public TextObjectPrinter
{
public:
  CASPFileGenerator(const File& file, const String& method)
    : TextObjectPrinter(file), method(method) {}

  virtual String getFormatSpecificationCode() const = 0;
  virtual void printPredictionData(ProteinPtr protein) = 0;

  virtual void consume(ObjectPtr object)
  {
    ProteinPtr protein = object.dynamicCast<Protein>();

    printRecord(T("PFRMAT"), getFormatSpecificationCode());
    printRecord(T("TARGET"), protein->getName());
    printRecord(T("AUTHOR"), T("ULg-GIGA")); // T("9344-7768-6843"));
    printMultiLineRecord(T("REMARK"), T("This is a prediction of server ULg-GIGA made at ") + Time::getCurrentTime().toString(true, true, true, true));
    printMultiLineRecord(T("METHOD"), method);
    printRecord(T("MODEL"), T("1"));
    printPredictionData(protein);
    print("END", true);
  }

protected:
  enum {maxColumns = 80};

  String method;

  void printRecord(const String& keyword, const String& data)
    {print(keyword + T(" ") + data + T("\n"));}

  void printMultiLineRecord(const String& keyword, const String& text)
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

  void printMultiLineRecordBase(const String& keyword, const String& text)
  {
    if (text.isEmpty())
      return;

    String line = keyword + T(" ");
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
};

class PDBFileGenerator : public TextObjectPrinter
{
public:
  static String makeAtomLine(size_t atomNumber, const String& atomName, const String& residueName, const String& chainID,
    size_t residueNumber, double x, double y, double z, double occupancy, double temperatureFactor, const String& segmentIdentifier,
    const String& elementSymbol, const String& atomCharge)
  {
    String line = T("ATOM  ");                                                    jassert(line.length() == 6);
    line += toFixedLengthStringRightJustified(String(atomNumber), 5);             jassert(line.length() == 11);
    line += T(" ");                                                               jassert(line.length() == 12);
    line += toFixedLengthString(atomName, 4);                                     jassert(line.length() == 16);
    line += T(" "); /* alternate location indicator */                            jassert(line.length() == 17);
    line += toFixedLengthString(residueName.toUpperCase(), 3);                    jassert(line.length() == 20);
    line += T(" ");                                                               jassert(line.length() == 21);
    line += toFixedLengthString(chainID, 1);                                      jassert(line.length() == 22);
    line += toFixedLengthStringRightJustified(String(residueNumber), 4);          jassert(line.length() == 26);
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

protected:
  static String toFixedLengthString(size_t i, int length)
    {return toFixedLengthString(String((int)i), length);}

  static String toFixedLengthString(const String& str, int length)
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

  static String toFixedLengthStringRightJustified(const String& str, int length)
  {
    jassert(str.length() <= length);
    String res = str;
    int i = 0;
    while (res.length() < length)
      res = T(" ") + res;
    return res;
  }
  
  static String toFixedLengthStringLeftJustified(const String& str, int length)
  {
    jassert(str.length() <= length);
    String res = str;
    int i = 0;
    while (res.length() < length)
      res += T(" ");
    return res;
  }
};

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

void addDefaultPredictions(ProteinPtr protein)
{
  size_t n = protein->getLength();

  LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
  jassert(aminoAcidSequence);

  /*
  ** Order-disorder region
  */
  ScoreVectorSequencePtr orderDisorderScoreSequence =
    new ScoreVectorSequence(T("OrderDisorderScoreSequence"), OrderDisorderDictionary::getInstance(), n);
  for (size_t i = 0; i < n; ++i)
  {
    orderDisorderScoreSequence->setScore(i, OrderDisorderDictionary::order, 0.5);
    orderDisorderScoreSequence->setScore(i, OrderDisorderDictionary::disorder, 0.5);
  }
  protein->setObject(orderDisorderScoreSequence);

  /*
  ** Residue-residue contact
  */
  ScoreSymmetricMatrixPtr rrContactMatrix = 
    new ScoreSymmetricMatrix(T("ResidueResidueContactProbabilityMatrix"), n, 0.5);
  protein->setObject(rrContactMatrix);

  /*
  ** Tertiary structure
  */
  ProteinTertiaryStructurePtr tertiaryStructure = new ProteinTertiaryStructure(n);
  for (size_t i = 0; i < n; ++i)
  {
    static const double constantLength = 4.32;

    ProteinResiduePtr residue = new ProteinResidue((AminoAcidDictionary::Type)aminoAcidSequence->getIndex(i));
    
    ProteinAtomPtr atom = new ProteinAtom(T("CA"), T("C"));
    atom->setPosition(Vector3(i * constantLength, 0.0, 0.0));
    residue->addAtom(atom);

    tertiaryStructure->setResidue(i, residue);
  }
  protein->setObject(tertiaryStructure);
}

int main(int argc, char* argv[])
{
  declareProteinClasses();

  if (argc < 5)
  {
    std::cerr << "Usage: " << argv[0] << " modelDirectory targetName aminoAcidSequence outputBaseName" << std::endl;
    return 1;
  }
  File cwd = File::getCurrentWorkingDirectory();
  File modelDirectory = cwd.getChildFile(argv[1]);
  std::cout << "Model directory: " << modelDirectory.getFullPathName() << std::endl;
  String targetName = argv[2];
  std::cout << "Target Name: " << targetName << std::endl;
  String aminoAcidSequence = argv[3];
  std::cout << "Amino Acid Sequence: " << aminoAcidSequence << std::endl;
  String outputBaseName = argv[4];
  std::cout << "Output Base Name: " << outputBaseName << std::endl;

  ProteinPtr protein = Protein::createFromAminoAcidSequence(targetName, aminoAcidSequence);
  if (!protein)
    return 2;

  addDefaultPredictions(protein);
  std::cout << "===========================" << std::endl << protein->toString() << std::endl;
  
  String method = T("This files contains a default prediction. No prediction methods are applied yet.\nWe have to quickly develop our code !!!");

  File tertiaryStructureFile = cwd.getChildFile(outputBaseName + T(".TS"));
  ObjectConsumerPtr(new TertiaryStructureCASPFileGenerator(tertiaryStructureFile, method))->consume(protein);

  File residueResidueDistanceFile = cwd.getChildFile(outputBaseName + T(".RR"));
  ObjectConsumerPtr(new ResidueResidueDistanceCASPFileGenerator(residueResidueDistanceFile, method))->consume(protein);

  File orderDisorderRegionFile = cwd.getChildFile(outputBaseName + T(".DR"));
  ObjectConsumerPtr(new OrderDisorderRegionCASPFileGenerator(orderDisorderRegionFile, method))->consume(protein);

  return 0;
}
