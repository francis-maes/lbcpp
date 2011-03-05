/*-----------------------------------------.---------------------------------.
| Filename: AminoAcid.cpp                  | Amino Acids                     |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "AminoAcid.h"
using namespace lbcpp;

/*
** AminoAcid
*/
const size_t AminoAcid::numStandardAminoAcid = 20;

// todo: remove and use Enumeration generic functions
const String AminoAcid::oneLetterCodes
  = T("ARNDCEQGHILKMFPSTWYVBZJ"); // X
const juce::tchar* AminoAcid::threeLettersCodes[] =
{
  T("Ala"), T("Arg"), T("Asn"), T("Asp"), T("Cys"),
  T("Glu"), T("Gln"), T("Gly"), T("His"), T("Ile"),
  T("Leu"), T("Lys"), T("Met"), T("Phe"), T("Pro"),
  T("Ser"), T("Thr"), T("Trp"), T("Tyr"), T("Val"),

  T("Asx"), T("Glx"), T("Xle"), // T("Xaa"),

  NULL
};
/*
Variable AminoAcid::getVariable(size_t index) const
{
  switch (index)
  {
  case 0: return Variable((int)type, aminoAcidTypeEnumeration);
  case 1: return toString();
  case 2: return getThreeLettersCode();
  case 3: return Variable((int)getCategory1(), aminoAcidCategory1Enumeration());
  };
  return Variable();
}
*/
Variable AminoAcid::fromOneLetterCode(juce::tchar code)
{
  int index = oneLetterCodes.indexOfChar(code);
  return index >= 0 ? Variable(index, aminoAcidTypeEnumeration) 
    : Variable::missingValue(aminoAcidTypeEnumeration);
}

Variable AminoAcid::fromThreeLettersCode(const String& code)
{
  for (size_t i = 0; i < 23; ++i)
    if (code.toUpperCase() == String(threeLettersCodes[i]).toUpperCase())
      return Variable(i, aminoAcidTypeEnumeration);

  // ligands
  if (code == T("MSE") || code == T("MHO"))
    return Variable((int)methionine, aminoAcidTypeEnumeration);
  else if (code == T("NEP") || code == T("MHS"))
    return Variable((int)histidine, aminoAcidTypeEnumeration);
  else if (code == T("ALY") || code == T("KCX"))
    return Variable((int)lysine, aminoAcidTypeEnumeration);
  else if (code == T("AGM"))
    return Variable((int)arginine, aminoAcidTypeEnumeration);
  else if (code == T("SMC") || code == T("OCS") || code == T("CSD") || code == T("CME"))
    return Variable((int)cysteine, aminoAcidTypeEnumeration);
  else if (code == T("GL3"))
    return Variable((int)glycine, aminoAcidTypeEnumeration);
  else if (code == T("GLH"))
    return Variable((int)glutamicAcid, aminoAcidTypeEnumeration);
  else if (code == T("PCA"))
    return Variable((int)glutamine, aminoAcidTypeEnumeration);
  else if (code == T("DBY"))
    return Variable((int)tyrosine, aminoAcidTypeEnumeration);
  else if (code == T("NLN"))
    return Variable((int)leucine, aminoAcidTypeEnumeration);
  else if (code == T("MEN"))
    return Variable((int)asparagine, aminoAcidTypeEnumeration);
  else if (code == T("HYP"))
    return Variable((int)proline, aminoAcidTypeEnumeration);

  return Variable::missingValue(aminoAcidTypeEnumeration);
}

juce::tchar AminoAcid::getOneLetterCode() const
{
  jassert((int)type < oneLetterCodes.length());
  return oneLetterCodes[type];
}

juce::tchar AminoAcid::toOneLetterCode(AminoAcidType type)
{
  jassert((int)type < oneLetterCodes.length());
  return oneLetterCodes[type];
}

String AminoAcid::getThreeLettersCode() const
{
  jassert((size_t)type < sizeof (threeLettersCodes) / sizeof (const juce::tchar* ));
  return threeLettersCodes[type];
}

String AminoAcid::toThreeLettersCode(AminoAcidType type)
{
  jassert((size_t)type < sizeof (threeLettersCodes) / sizeof (const juce::tchar* ));
  return String(threeLettersCodes[type]);
}

AminoAcidCategory1 AminoAcid::getCategory1() const
{
  juce::tchar c = oneLetterCodes[type];
  if (String(T("GAVLIPMFW")).indexOfChar(c) >= 0)
    return nonPolarCategory;
  if (String(T("STNQCY")).indexOfChar(c) >= 0)
    return polarCategory;
  if (c == 'D' || c == 'E')
    return acidicCategory;
  if (c == 'K' || c == 'R' || c == 'H')
    return basicCategory;
  jassert(false);
  return nonPolarCategory;
}
