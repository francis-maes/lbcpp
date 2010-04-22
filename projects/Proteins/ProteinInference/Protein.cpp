/*-----------------------------------------.---------------------------------.
| Filename: Protein.cpp                    | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 17/04/2010 14:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Protein.h"
#include "AminoAcidDictionary.h"
#include "SecondaryStructureDictionary.h"
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

ScoreVectorSequencePtr Protein::getAminoAcidProperty() const
{return getObject(T("AminoAcidProperty"));}

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

void Protein::computeProperties()
{
  static const double kiteDoolittle[24] = {0.7, 0, 0.11, 0.11, 0.78, 0.11, 0.11, 0.46, 0.14, 1, 0.92, 0.07, 0.71, 0.81, 0.32, 0.41, 0.42, 0.4, 0.36, 0.97, 0.11, 0.11, 0.96, 0.5};
  static const double hoppWoods[24] = {0.45, 1, 0.56, 1, 0.38, 1, 0.56, 0.53, 0.45, 0.25, 0.25, 1, 0.33, 0.14, 0.53, 0.58, 0.47, 0, 0.17, 0.3, 0.78, 0.78, 0.25, 0.5};
  static const double cornette[24] = {0.38, 0.51, 0.3, 0, 0.82, 0.15, 0.03, 0.35, 0.41, 0.9, 1, 0, 0.83, 0.85, 0.1, 0.3, 0.14, 0.47, 0.72, 0.89, 0.65, 0.09, 0.95, 0.5};
  static const double eisenberg[24] = {0.81, 0, 0.45, 0.42, 0.72, 0.46, 0.43, 0.77, 0.54, 1, 0.92, 0.26, 0.81, 0.95, 0.68, 0.6, 0.63, 0.85, 0.71, 0.92, 0.435, 0.445, 0.96, 0.5};
  static const double rose[24] = {0.56, 0.31, 0.28, 0.26, 1, 0.26, 0.26, 0.51, 0.67, 0.92, 0.85, 0, 0.85, 0.92, 0.31, 0.36, 0.46, 0.85, 0.62, 0.87, 0.27, 0.26, 0.885, 0.5};
  static const double janin[24] = {0.78, 0.15, 0.48, 0.44, 1, 0.41, 0.41, 0.78, 0.63, 0.93, 0.85, 0, 0.81, 0.85, 0.56, 0.63, 0.59, 0.78, 0.52, 0.89, 0.46, 0.41, 0.89, 0.5};
  static const double engelman[24] = {0.87, 0, 0.47, 0.19, 0.89, 0.26, 0.51, 0.83, 0.58, 0.96, 0.94, 0.22, 0.98, 1, 0.76, 0.81, 0.84, 0.89, 0.73, 0.93, 0.34, 0.385, 0.95, 0.5};
  static const double russellLinding[24] = {0.1656, 0.2532, 0.6723, 0.6699, 0.4337, 0.2242, 0.2418, 0.8829, 0.4340, 0.0000, 0.0869, 0.3322, 0.2024, 0.2027, 1.0000, 0.5826, 0.4444, 0.1844, 0.2213, 0.0371, 0.67, 0.23, 0.04, 0.5};
  static const double deleageRoux[24] = {0.2362, 0.2889, 0.6499, 0.6420, 0.3182, 0.2365, 0.3569, 0.7534, 0.4612, 0.1045, 0.1465, 0.3599, 0.1257, 0.1144, 1.0000, 0.5498, 0.4667, 0.2461, 0.4324, 0.0000, 0.645, 0.285, 0.12};
  
  jassert(getAminoAcidProperty() == ObjectPtr());
  
  ScoreVectorSequencePtr properties = new ScoreVectorSequence(T("AminoAcidProperty"), AminoAcidPropertyDictionary::getInstance(), getLength());
  
  LabelSequencePtr sequence = getAminoAcidSequence();
  
  for (size_t i = 0; i < getLength(); ++i) {
    properties->setScore(i, 0, kiteDoolittle[sequence->getIndex(i)]);
    properties->setScore(i, 1, hoppWoods[sequence->getIndex(i)]);
    properties->setScore(i, 2, cornette[sequence->getIndex(i)]);
    properties->setScore(i, 3, eisenberg[sequence->getIndex(i)]);
    properties->setScore(i, 4, rose[sequence->getIndex(i)]);
    properties->setScore(i, 5, janin[sequence->getIndex(i)]);
    properties->setScore(i, 6, engelman[sequence->getIndex(i)]);
    properties->setScore(i, 7, russellLinding[sequence->getIndex(i)]);
    properties->setScore(i, 8, deleageRoux[sequence->getIndex(i)]);
  }
  
  setObject(properties);
}

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
