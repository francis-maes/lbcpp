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

class AminoAcidDictionary
{
public:
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

  /*
  ** Constructors
  */
  AminoAcidDictionary(const AminoAcidDictionary& aa)
    : type(aa.type) {}
    
  AminoAcidDictionary(const String& code);
  AminoAcidDictionary(const Type& type = unknown)
    : type(type) {}
  
  /*
  ** Type
  */
  Type getType() const
    {return type;}

  bool isAmbiguous() const
    {return type >= asparagineOrAsparticAcid;}

  bool isNonAmbiguous() const
    {return !isAmbiguous();}

  /*
  ** One letter code
  */
  static const String oneLetterCodes;
  static Type getTypeFromOneLetterCode(juce::tchar c);

  juce::tchar getOneLetterCode() const;

  /*
  ** Three letter code
  */
  static StringDictionaryPtr getThreeLettersCodes();
  String getThreeLettersCode() const;
     
  /*
  ** Comparison operators
  */
  bool operator ==(const AminoAcidDictionary& aa) const
    {return type == aa.type;}
    
  bool operator !=(const AminoAcidDictionary& aa) const
    {return type != aa.type;}
    
  operator Type() const
    {return type;}

private:
  Type type;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_AMINO_ACID_DICTIONARY_H_
