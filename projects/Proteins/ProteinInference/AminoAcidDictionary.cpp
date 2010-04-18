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
  static AminoAcidDictionaryPtr instance = new AminoAcidDictionary();
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

const String AminoAcidDictionary::getThreeLettersCode(Type aminoAcidType)
{
  static const juce::tchar* threeLettersCodes[] = {
    T("Ala"), T("Arg"), T("Asn"), T("Asp"), T("Cys"),
    T("Glu"), T("Gln"), T("Gly"), T("His"), T("Ile"),
    T("Leu"), T("Lys"), T("Met"), T("Phe"), T("Pro"),
    T("Ser"), T("Thr"), T("Trp"), T("Tyr"), T("Val"),

    T("Asx"), T("Glx"), T("Xle"), T("Xaa"),

    NULL
  };

  jassert(aminoAcidType < sizeof (threeLettersCodes) / sizeof (const juce::tchar* ));
  return threeLettersCodes[aminoAcidType];
}
