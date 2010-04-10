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
  static FeatureDictionaryPtr instance;
  if (!instance)
    FeatureDictionaryManager::getInstance().addDictionary(instance = new SecondaryStructureDictionary());
  return instance;
}

SecondaryStructureDictionary::SecondaryStructureDictionary()
  : FeatureDictionary(T("SecondaryStructure"), new StringDictionary(T("SecondaryStructure features")), StringDictionaryPtr())
{
  addFeature(T("H"));
  addFeature(T("E"));
  addFeature(T("C"));
}

/*
** DSSPSecondaryStructure
*/
FeatureDictionaryPtr DSSPSecondaryStructureDictionary::getInstance()
{
  static FeatureDictionaryPtr instance;
  if (!instance)
    FeatureDictionaryManager::getInstance().addDictionary(instance = new DSSPSecondaryStructureDictionary());
  return instance;
}

const String DSSPSecondaryStructureDictionary::oneLetterCodes
  = T("GHITEBS");

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
** SolventAccesibility2StateDictionary
*/
FeatureDictionaryPtr SolventAccesibility2StateDictionary::getInstance()
{
  static FeatureDictionaryPtr instance;
  if (!instance)
    FeatureDictionaryManager::getInstance().addDictionary(instance = new SolventAccesibility2StateDictionary());
  return instance;
}

SolventAccesibility2StateDictionary::SolventAccesibility2StateDictionary()
  : FeatureDictionary(T("SolventAccesibility2State"), new StringDictionary(T("SolventAccesibility2State features")), StringDictionaryPtr())
{
  addFeature(T("E"));
  addFeature(T("B"));
}