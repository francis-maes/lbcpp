/*-----------------------------------------.---------------------------------.
| Filename: Protein.cpp                    | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 17/04/2010 14:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Protein.h"
#include "AminoAcidDictionary.h"
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
