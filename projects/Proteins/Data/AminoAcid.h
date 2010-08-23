/*-----------------------------------------.---------------------------------.
| Filename: AminoAcid.h                    | Amino Acids                     |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 17:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_AMINO_ACID_H_
# define LBCPP_PROTEINS_AMINO_ACID_H_

# include <lbcpp/Data/Variable.h>

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

extern EnumerationPtr aminoAcidTypeEnumeration();

enum AminoAcidCategory1
{
  nonPolarCategory,
  polarCategory,
  acidicCategory,
  basicCategory,
};

extern EnumerationPtr aminoAcidCategory1Enumeration();

class AminoAcid : public Object
{
public:
  static const size_t numStandardAminoAcid;
  
  AminoAcidType getType() const
    {return type;}

  juce::tchar getOneLetterCode() const;
  String getThreeLettersCode() const;
  AminoAcidCategory1 getCategory1() const;

  virtual String getName() const
    {return getThreeLettersCode();}

  virtual String toString() const
    {String res; res += getOneLetterCode(); return res;}

 // virtual Variable getVariable(size_t index) const;
  
  virtual bool isStandard() const
    {return type < 20;}

  static Variable fromOneLetterCode(juce::tchar code);
  static Variable fromThreeLettersCode(const String& code);
  
  static juce::tchar toOneLetterCode(AminoAcidType type);
  static String toThreeLettersCode(AminoAcidType type);
  
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

extern ClassPtr aminoAcidClass();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_AMINO_ACID_H_
