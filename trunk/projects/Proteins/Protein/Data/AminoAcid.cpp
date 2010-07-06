/*-----------------------------------------.---------------------------------.
| Filename: AminoAcid.cpp                  | Amino Acids                     |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "AminoAcid.h"
using namespace lbcpp;

/*
** AminoAcid
*/
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

Variable AminoAcid::getVariable(size_t index) const
{
  switch (index)
  {
  case 0: return (int)type;
  case 1: return toString();
  case 2: return getThreeLettersCode();
  case 3: return (int)getCategory1();
  };
  return Variable();
}

juce::tchar AminoAcid::getOneLetterCode() const
{
  jassert((int)type < oneLetterCodes.length());
  return oneLetterCodes[type];
}

String AminoAcid::getThreeLettersCode() const
{
  jassert((size_t)type < sizeof (threeLettersCodes) / sizeof (const juce::tchar* ));
  return threeLettersCodes[type];
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
/*
** AminoAcidClass
*/
namespace lbcpp {

class AminoAcidClass : public Collection
{
public:
  AminoAcidClass() : Collection(T("AminoAcid"))
  {
    addVariable(aminoAcidTypeEnumeration(), T("type"));
    addVariable(stringType(), T("oneLetterCode"));
    addVariable(stringType(), T("threeLettersCode"));
    addVariable(aminoAcidCategory1Enumeration(), T("category1"));

    for (size_t i = 0; i < (size_t)totalNumAminoAcids; ++i)
      addElement(new AminoAcid((AminoAcidType)i));
  }
};

}; /* namespace lbcpp */

static const juce::tchar* aminoAcidTypeStrings[] = {
  // 20 amino acids
  T("Alanine"), T("Arginine"), T("Asparagine"), T("Aspartic Acid"), T("Cysteine"),
  T("Glutamic Acid"), T("Glutamine"), T("Glycine"), T("Histidine"), T("Isoleucine"),
  T("Leucine"), T("Lysine"), T("Methionine"), T("Phenylalanine"), T("Proline"),
  T("Serine"), T("Threonine"), T("Tryptophan"), T("Tyrosine"), T("Valine"),

  // 3 ambiguous amino acids
  T("Asparagine or Aspartic Acid"),
  T("Glutamine or Glutamic Acid"),
  T("Lecine or Isoleucine"),

  NULL
};

EnumerationPtr lbcpp::aminoAcidTypeEnumeration()
  {static EnumerationPtr res = Enumeration::get(T("AminoAcidType")); return res;}

static const juce::tchar* aminoAcidCategory1Strings[] = {
  T("Non Polar"), T("Polar"), T("Acidic"), T("Basic"), NULL
};

EnumerationPtr lbcpp::aminoAcidCategory1Enumeration()
  {static EnumerationPtr res = Enumeration::get(T("AminoAcidCategory1")); return res;}

void declareAminoAcidClasses()
{
  Class::declare(new Enumeration(T("AminoAcidType"), aminoAcidTypeStrings));
  Class::declare(new Enumeration(T("AminoAcidCategory1"), aminoAcidCategory1Strings));
  Class::declare(new AminoAcidClass());
}
