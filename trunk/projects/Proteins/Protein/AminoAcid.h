/*-----------------------------------------.---------------------------------.
| Filename: AminoAcid.h                    | Amino Acids                     |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 17:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_AMINO_ACID_H_
# define LBCPP_PROTEINS_AMINO_ACID_H_

# include <lbcpp/Object/Class.h>

namespace lbcpp
{

enum AminoAcidType
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
  
  // 4 ambiguous amino acids
  asparagineOrAsparticAcid, // B
  glutamineOrGlutamicAcid,  // Z
  lecineOrIsoleucine,       // J

  totalNumAminoAcids,
};

enum AminoAcidCategory1
{
  nonPolarCategory,
  polarCategory,
  acidicCategory,
  basicCategory,
};

class AminoAcid : public Object
{
public:
  AminoAcidType getType() const
    {return type;}

  static CollectionPtr getCollection()
    {return Class::get(T("AminoAcid"));}

  juce::tchar getOneLetterCode() const;
  String getThreeLettersCode() const;
  AminoAcidCategory1 getCategory1() const;


  virtual String toString() const
    {String res; res += getOneLetterCode(); return res;}

  virtual void accept(ObjectVisitorPtr visitor);

private:
  friend class AminoAcidClass;

  AminoAcid(AminoAcidType type)
    : type(type) {}

  AminoAcid() {}

  AminoAcidType type;

  static const String oneLetterCodes;
  static const juce::tchar* threeLettersCodes[];
};

typedef ReferenceCountedObjectPtr<AminoAcid> AminoAcidPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_AMINO_ACID_H_
