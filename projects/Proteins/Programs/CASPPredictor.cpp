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

class TertiaryStructureCASPFileGenerator : public CASPFileGenerator
{
public:
  TertiaryStructureCASPFileGenerator(const File& file, const String& method)
    : CASPFileGenerator(file, method) {}

  virtual String getFormatSpecificationCode() const
    {return T("TS");}

  virtual void printPredictionData(ProteinPtr protein)
    {/* FIXME */}
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
