/*-----------------------------------------.---------------------------------.
| Filename: Protein.cpp                    | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 17/04/2010 14:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Protein.h"
#include "AminoAcidDictionary.h"
#include "PDBFileParser.h"
#include "PDBFileGenerator.h"
using namespace lbcpp;

ProteinPtr Protein::createFromAminoAcidSequence(const String& name, const String& aminoAcidString)
{
  ProteinPtr res = new Protein(name);
  FeatureDictionaryPtr aminoAcidDictionary = AminoAcidDictionary::getInstance();
  LabelSequencePtr aminoAcidSequence = new LabelSequence(T("AminoAcidSequence"), aminoAcidDictionary, aminoAcidString.length());
  for (size_t i = 0; i < aminoAcidSequence->size(); ++i)
  {
    String aa;
    aa += aminoAcidString[i];
    int index = aminoAcidDictionary->getFeatures()->getIndex(aa);
    if (index < 0)
    {
      Object::error(T("Protein::createFromAminoAcidSequence"), T("Unknown amino acid: ") + aa);
      return ProteinPtr();
    }
    aminoAcidSequence->setIndex(i, (size_t)index);
  }
  res->setObject(aminoAcidSequence);
  return res;
}

class FASTAFileParser : public TextObjectParser
{
public:
  FASTAFileParser(const File& file)
    : TextObjectParser(file) {}
  
  virtual void parseBegin()
    {}

  virtual bool parseLine(const String& line)
  {
    String str = line.trim();
    if (str.isEmpty())
      return true;
    if (str[0] == '>')
    {
      flush();
      currentName = str.substring(1);
    }
    else
      currentAminoAcidSequence = str;
    return true;
  }
  
  virtual bool parseEnd()
  {
    flush();
    return true;
  }
  
private:
  String currentName;
  String currentAminoAcidSequence;
  
  void flush()
  {
    if (currentName.isNotEmpty() && currentAminoAcidSequence.isNotEmpty())
      setResult(Protein::createFromAminoAcidSequence(currentName, currentAminoAcidSequence));
    currentName = String::empty;
    currentAminoAcidSequence = String::empty;
  }
};

ProteinPtr Protein::createFromFASTA(const File& fastaFile)
{
  ObjectStreamPtr parser(new FASTAFileParser(fastaFile));
  return parser->nextAndCast<Protein>();
}

ProteinPtr Protein::createFromPDB(const File& pdbFile, bool beTolerant)
{
  ObjectStreamPtr parser(new PDBFileParser(pdbFile, beTolerant));
  ProteinPtr res = parser->nextAndCast<Protein>();
  if (!res)
    return ProteinPtr();

  LabelSequencePtr aminoAcidSequence = res->getAminoAcidSequence();
  ProteinTertiaryStructurePtr tertiaryStructure = res->getTertiaryStructure();
  jassert(tertiaryStructure);
  
  // the tertiary structure may not be defined on the whole primary sequence
  // therefore, in general length(tertiary structure) <= length(primary sequence)
  // here, we reduce the primary sequence to the portion for which the tertiary structure is defined
  if (!aminoAcidSequence || tertiaryStructure->size() < aminoAcidSequence->size())
    res->setObject(tertiaryStructure->createAminoAcidSequence()); 

  res->setObject(tertiaryStructure->createCAlphaTrace());
  if (!tertiaryStructure->hasOnlyCAlphaAtoms())
  {
    res->setObject(tertiaryStructure->createCBetaTrace());
    res->setObject(ProteinDihedralAngles::createDihedralAngles(tertiaryStructure));
  }
  return res;
}

void Protein::saveToPDBFile(const File& pdbFile)
{
  ObjectConsumerPtr(new PDBFileGenerator(pdbFile))->consume(ProteinPtr(this));
}

size_t Protein::getLength() const
  {return getAminoAcidSequence()->size();}

LabelSequencePtr Protein::getAminoAcidSequence() const
  {return getObject(T("AminoAcidSequence"));}

ScoreVectorSequencePtr Protein::getPositionSpecificScoringMatrix() const
  {return getObject(T("PositionSpecificScoringMatrix"));}

LabelSequencePtr Protein::getSecondaryStructureSequence() const
  {return getObject(T("SecondaryStructureSequence"));}

LabelSequencePtr Protein::getDSSPSecondaryStructureSequence() const
  {return getObject(T("DSSPSecondaryStructureSequence"));}

LabelSequencePtr Protein::getSolventAccessibilitySequence() const
  {return getObject(T("SolventAccessibilitySequence"));}

LabelSequencePtr Protein::getOrderDisorderSequence() const
  {return getObject(T("OrderDisorderSequence"));}

ScoreVectorSequencePtr Protein::getOrderDisorderScoreSequence() const
  {return getObject(T("OrderDisorderScoreSequence"));}

ScoreSymmetricMatrixPtr Protein::getResidueResidueContactProbabilityMatrix() const
  {return getObject(T("ResidueResidueContactProbabilityMatrix"));}

ProteinDihedralAnglesPtr Protein::getDihedralAngles() const
  {return getObject(T("DihedralAngles"));}

CartesianCoordinatesSequencePtr Protein::getCAlphaTrace() const
  {return getObject(T("CAlphaTrace"));}

ProteinTertiaryStructurePtr Protein::getTertiaryStructure() const
  {return getObject(T("TertiaryStructure"));}

bool Protein::load(InputStream& istr)
{
  int versionNumber;
  if (!lbcpp::read(istr, versionNumber))
    return false;
  if (versionNumber != 101)
  {
    Object::error(T("Protein::load"), T("Unrecognized version number"));
    return false;
  }
  return StringToObjectMap::load(istr);
}

void Protein::save(OutputStream& ostr) const
{
  int versionNumber = 101;
  lbcpp::write(ostr, versionNumber);
  StringToObjectMap::save(ostr);
}
