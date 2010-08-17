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
    alanine = 0,      // A  -  Ala
    arginine,         // R  -  Arg
    asparagine,       // N  -  Asn
    asparticAcid,     // D  -  Asp
    cysteine,         // C  -  Cys
    glutamicAcid,     // E  -  Glu
    glutamine,        // Q  -  Gln
    glycine,          // G  -  Gly
    histidine,        // H  -  His
    isoleucine,       // I  -  Ile
    leucine,          // L  -  Leu
    lysine,           // K  -  Lys
    methionine,       // M  -  Met
    phenylalanine,    // F  -  Phe
    proline,          // P  -  Pro
    serine,           // S  -  Ser
    threonine,        // T  -  Thr
    tryptophan,       // W  -  Trp
    tyrosine,         // Y  -  Tyr
    valine,           // V  -  Val
    
    // 3 ambiguous amino acids
    asparagineOrAsparticAcid, // B (gap)
    glutamineOrGlutamicAcid,  // Z (entropy)
    lecineOrIsoleucine,       // J

    // unspecified or unknown amino acid
    unknown                   // X
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
