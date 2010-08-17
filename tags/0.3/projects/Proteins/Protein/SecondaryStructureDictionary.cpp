/*-----------------------------------------.---------------------------------.
| Filename: SecondaryStructureDictionary.cpp| Secondary Structure Dictionary |
| Author  : Francis Maes                   |                                 |
| Started : 10/04/2010 16:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "SecondaryStructureDictionary.h"
using namespace lbcpp;

/*
** SecondaryStructure
*/
FeatureDictionaryPtr SecondaryStructureDictionary::getInstance()
{
  static FeatureDictionaryPtr instance = new SecondaryStructureDictionary();
  return instance;
}

SecondaryStructureDictionary::SecondaryStructureDictionary()
  : FeatureDictionary(T("SecondaryStructure"), new StringDictionary(T("SecondaryStructure features")), StringDictionaryPtr())
{
  addFeature(T("H"));
  addFeature(T("E"));
  addFeature(T("C"));
}

SecondaryStructureDictionary::Type SecondaryStructureDictionary::getIndexFromDSSPElement(const String& dsspElement)
{
  int dsspIndex = DSSPSecondaryStructureDictionary::getInstance()->getFeatures()->getIndex(dsspElement);
  jassert(dsspIndex >= 0);
  DSSPSecondaryStructureDictionary::Type type = (DSSPSecondaryStructureDictionary::Type)dsspIndex;
  if (type == DSSPSecondaryStructureDictionary::threeTurnHelix || type == DSSPSecondaryStructureDictionary::alphaHelix)
    return helix;
  if (type == DSSPSecondaryStructureDictionary::residueInIsolatedBridge || type == DSSPSecondaryStructureDictionary::extendedStrandInSheet)
    return sheet;
  return other;
}

LabelSequencePtr SecondaryStructureDictionary::createSequenceFromDSSPSequence(const String& name, LabelSequencePtr dsspSequence)
{
  jassert(dsspSequence->getDictionary() == DSSPSecondaryStructureDictionary::getInstance());
  size_t n = dsspSequence->size();

  LabelSequencePtr res = new LabelSequence(name, getInstance(), n);
  for (size_t i = 0; i < n; ++i)
    if (dsspSequence->get(i))
      res->setIndex(i, (size_t)getIndexFromDSSPElement(dsspSequence->getString(i)));
  return res;
}

/*
** DSSPSecondaryStructure
*/
FeatureDictionaryPtr DSSPSecondaryStructureDictionary::getInstance()
{
  static FeatureDictionaryPtr instance = new DSSPSecondaryStructureDictionary();
  return instance;
}

const String DSSPSecondaryStructureDictionary::oneLetterCodes
  = T("GHITEBS_");

DSSPSecondaryStructureDictionary::DSSPSecondaryStructureDictionary()
  : FeatureDictionary(T("DSSPSecondaryStructure"), new StringDictionary(T("DSSPSecondaryStructure features")), StringDictionaryPtr())
{
  for (int i = 0; i < oneLetterCodes.length(); ++i)
  {
    String oneLetterCode;
    oneLetterCode += oneLetterCodes[i];
    addFeature(oneLetterCode);
  }
}

/*
** AminoAcidPropertyDictionary
*/
FeatureDictionaryPtr AminoAcidPropertyDictionary::getInstance()
{
  static FeatureDictionaryPtr instance = new AminoAcidPropertyDictionary();
  return instance;
}

AminoAcidPropertyDictionary::AminoAcidPropertyDictionary()
: FeatureDictionary(T("AminoAcidProperty"), new StringDictionary(T("AminoAcidProperty features")), StringDictionaryPtr())
{
  // Hydrophobicity
  addFeature(T("KyteDoolittle"));
  addFeature(T("HoppWoods"));
  addFeature(T("Cornette"));
  addFeature(T("Eisenberg"));
  addFeature(T("Rose"));
  addFeature(T("Janin"));
  addFeature(T("Engelman"));
  
  // Sequence complexity
  //addFeature(T("K2Entropy"));
  
  // IUPPropensity
  addFeature(T("RussellLinding"));
  addFeature(T("DeleageRoux"));
}

/*
** StructuralAlphabetDictionary
*/
FeatureDictionaryPtr StructuralAlphabetDictionary::getInstance()
{
  static FeatureDictionaryPtr instance = new StructuralAlphabetDictionary();
  return instance;
}

StructuralAlphabetDictionary::StructuralAlphabetDictionary()
: FeatureDictionary(T("StructuralAlphabet"), new StringDictionary(T("StructuralAlphabet features")), StringDictionaryPtr())
{
  for (size_t i = 0; i < 26; ++i)
  {
    String str = T("A");
    str[0] += i;
    addFeature(str);
  }
  addFeature(T("a"));
}

