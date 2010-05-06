/*-----------------------------------------.---------------------------------.
| Filename: AminoAcidDictionary.h          | Amino Acid Dictionary           |
| Author  : Francis Maes                   |                                 |
| Started : 26/03/2010 13:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_AMINO_ACID_DICTIONARY_H_
# define LBCPP_PROTEIN_INFERENCE_AMINO_ACID_DICTIONARY_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class AminoAcidDictionary;
typedef ReferenceCountedObjectPtr<AminoAcidDictionary> AminoAcidDictionaryPtr;

class AminoAcidDictionary : public FeatureDictionary
{
public:
  static AminoAcidDictionaryPtr getInstance();

  enum Type
  {
    // 20 amino acids
    alanine = 0,
    arginine,
    asparagine,
    asparticAcid,
    cysteine,
    glutamicAcid,
    glutamine,
    glycine,
    histidine,
    isoleucine,
    leucine,
    lysine,
    methionine,
    phenylalanine,
    proline,
    serine,
    threonine,
    tryptophan,
    tyrosine,
    valine,
    
    // 3 ambiguous amino acids
    asparagineOrAsparticAcid,
    glutamineOrGlutamicAcid,
    lecineOrIsoleucine,

    // unspecified or unknown amino acid
    unknown
  };

  enum
  {
    numAminoAcids = 20,
    numAmbiguousAminoAcids = 4
  };

  static const String oneLetterCodes;
  static const juce::tchar* threeLettersCodes[];

  static Type getTypeFromThreeLettersCode(const String& threeLettersCode);
  static Type getTypeFromOneLetterCode(const juce::juce_wchar oneLetterCode);
  static const String getThreeLettersCode(Type aminoAcidType);

  enum AminoAcidCategory
  {
    nonPolarCategory,
    polarCategory,
    acidicCategory,
    basicCategory,
    unknownCategory
  };

  static AminoAcidCategory getCategory(Type aminoAcidType);

private:
  AminoAcidDictionary();
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_AMINO_ACID_DICTIONARY_H_
