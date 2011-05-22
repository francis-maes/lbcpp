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

const juce::tchar* AminoAcidDictionary::threeLettersCodes[] =
{
  T("Ala"), T("Arg"), T("Asn"), T("Asp"), T("Cys"),
  T("Glu"), T("Gln"), T("Gly"), T("His"), T("Ile"),
  T("Leu"), T("Lys"), T("Met"), T("Phe"), T("Pro"),
  T("Ser"), T("Thr"), T("Trp"), T("Tyr"), T("Val"),

  T("Asx"), T("Glx"), T("Xle"), T("Xaa"),

  NULL
};

const String AminoAcidDictionary::getThreeLettersCode(Type aminoAcidType)
{
  jassert((size_t)aminoAcidType < sizeof (threeLettersCodes) / sizeof (const juce::tchar* ));
  return threeLettersCodes[aminoAcidType];
}

AminoAcidDictionary::Type AminoAcidDictionary::getTypeFromThreeLettersCode(const String& threeLettersCode)
{
  jassert(threeLettersCode.length() == 3);
  String code = threeLettersCode.toUpperCase();

  // ligands
  if (code == T("MSE") || code == T("MHO"))
    return methionine;
  else if (code == T("NEP") || code == T("MHS"))
    return histidine;
  else if (code == T("ALY") || code == T("KCX"))
    return lysine;
  else if (code == T("AGM"))
    return arginine;
  else if (code == T("SMC") || code == T("OCS") || code == T("CSD") || code == T("CME"))
    return cysteine;
  else if (code == T("GL3"))
    return glycine;
  else if (code == T("GLH"))
    return glutamicAcid;
  else if (code == T("PCA"))
    return glutamine;
  else if (code == T("DBY"))
    return tyrosine;
  else if (code == T("NLN"))
    return leucine;
  else if (code == T("MEN"))
    return asparagine;
  else if (code == T("HYP"))
    return proline;

  for (size_t i = 0; i < sizeof (threeLettersCodes) / sizeof (const juce::tchar* ); ++i)
    if (String(threeLettersCodes[i]).toUpperCase() == code)
      return (Type)i;
  return unknown;
}

AminoAcidDictionary::Type AminoAcidDictionary::getTypeFromOneLetterCode(const juce::juce_wchar oneLetterCode)
{
  for (size_t i = 0; i < (size_t) oneLetterCodes.length(); ++i) {
    if (oneLetterCode == oneLetterCodes[i])
      return (Type) i;
  }
  
  jassert(false);
  return unknown;
}

AminoAcidDictionary::AminoAcidCategory AminoAcidDictionary::getCategory(Type aminoAcidType)
{
  juce::tchar c = oneLetterCodes[aminoAcidType];
  if (String(T("GAVLIPMFW")).indexOfChar(c) >= 0)
    return nonPolarCategory;
  if (String(T("STNQCY")).indexOfChar(c) >= 0)
    return polarCategory;
  if (c == 'D' || c == 'E')
    return acidicCategory;
  if (c == 'K' || c == 'R' || c == 'H')
    return basicCategory;
  return unknownCategory;
}