/*-----------------------------------------.---------------------------------.
| Filename: AminoAcidDictionary.cpp        | Amino Acid Dictionary           |
| Author  : Francis Maes                   |                                 |
| Started : 26/03/2010 17:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "AminoAcidDictionary.h"
using namespace lbcpp;

AminoAcidDictionaryPtr AminoAcidDictionary::getInstance()
{
  static AminoAcidDictionaryPtr instance;
  if (!instance)
    FeatureDictionaryManager::getInstance().addDictionary(instance = new AminoAcidDictionary());
  return instance;
}

const String AminoAcidDictionary::oneLetterCodes
  = T("ARNDCEQGHILKMFPSTWYVBZJX");

AminoAcidDictionary::AminoAcidDictionary()
  : FeatureDictionary(T("AminoAcid"), new StringDictionary(T("AminoAcid features")), StringDictionaryPtr())
{
  for (int i = 0; i < oneLetterCodes.length(); ++i)
  {
    String oneLetterCode;
    oneLetterCode += oneLetterCodes[i];
    addFeature(oneLetterCode);
  }
}

#if 0
AminoAcidDictionary::AminoAcidDictionary(const String& code)
{
  if (code.length() == 1)
    type = getTypeFromOneLetterCode(code[0]);
  else
  {
    type = unknown;
    jassert(false); // todo: three-letter code
  }
}

juce::tchar AminoAcidDictionary::getOneLetterCode() const
{
  jassert(type >= 0 && type <= unknown);
  jassert(oneLetterCodes.length() == (int)unknown + 1);
  return oneLetterCodes[type];
}

AminoAcidDictionary::Type AminoAcidDictionary::getTypeFromOneLetterCode(juce::tchar c)
{
  int i = oneLetterCodes.indexOfChar(c);
  jassert(i >= 0);
  return i >= 0 ? (Type)i : unknown;
}

StringDictionaryPtr AminoAcidDictionary::getThreeLettersCodes()
{
  static const juce::tchar* threeLettersCodes[] = {
    T("Ala"), T("Arg"), T("Asn"), T("Asp"), T("Cys"),
    T("Glu"), T("Gln"), T("Gly"), T("His"), T("Ile"),
    T("Leu"), T("Lys"), T("Met"), T("Phe"), T("Pro"),
    T("Ser"), T("Thr"), T("Trp"), T("Tyr"), T("Val"),

    T("Asx"), T("Glx"), T("Xle"), T("Xaa"),

    NULL
  };

  static StringDictionaryPtr dictionary = new StringDictionary(T("AminoAcidThreeLetterCode"), threeLettersCodes);
  return dictionary;
}

String AminoAcidDictionary::getThreeLettersCode() const
{
  StringDictionaryPtr dictionary = getThreeLettersCodes();
  jassert(dictionary->exists((size_t)type));
  return dictionary->getString((size_t)type);
}

#endif