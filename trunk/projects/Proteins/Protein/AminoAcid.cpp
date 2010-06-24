/*-----------------------------------------.---------------------------------.
| Filename: AminoAcid.cpp                  | Amino Acids                     |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "AminoAcid.h"
using namespace lbcpp;

static const juce::tchar* aminoAcidTypeStrings[] = {
  // 20 amino acids
  T("Alanine"), T("Asparagine"), T("Aspartic Acid"), T("Cysteine"),
  T("Glutamic Acid"), T("Glutamine"), T("Glycine"), T("Histidine"), T("Isoleucine"),
  T("Leucine"), T("Lysine"), T("Methionine"), T("Phenylalanine"), T("Proline"),
  T("Serine"), T("Threonine"), T("Tryptophan"), T("Tyrosine"), T("Valine"),

  // 3 ambiguous amino acids
  T("Asparagine or Aspartic Acid"),
  T("Glutamine or Glutamic Acid"),
  T("Lecine or Isoleucine"),

  NULL
};

static const juce::tchar* aminoAcidCategory1Strings[] = {
  T("Non Polar"), T("Polar"), T("Acidic"), T("Basic"), NULL
};

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

void AminoAcid::accept(ObjectVisitorPtr visitor)
{
  visitor->visitVariable(0, (int)type);
  visitor->visitVariable(1, toString());
  visitor->visitVariable(2, getThreeLettersCode());
  visitor->visitVariable(3, (int)getCategory1());
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
    addVariable(T("AminoAcidType"), T("type"));
    addVariable(StringClass::getInstance(), T("oneLetterCode"));
    addVariable(StringClass::getInstance(), T("threeLettersCode"));
    addVariable(T("AminoAcidCategory1"), T("category1"));

    for (size_t i = 0; i < (size_t)totalNumAminoAcids; ++i)
      addElement(new AminoAcid((AminoAcidType)i));
  }
};

}; /* namespace lbcpp */

void declareAminoAcidClasses()
{
  Class::declare(new Enumeration(T("AminoAcidType"), aminoAcidTypeStrings));
  Class::declare(new Enumeration(T("AminoAcidCategory1"), aminoAcidCategory1Strings));
  Class::declare(new AminoAcidClass());
}
