/*-----------------------------------------.---------------------------------.
| Filename: SecondaryStructureDictionary.h | Secondary Structure Dictionary  |
| Author  : Francis Maes                   |                                 |
| Started : 10/04/2010 14:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_SECONDARY_STRUCTURE_DICTIONARY_H_
# define LBCPP_PROTEIN_INFERENCE_SECONDARY_STRUCTURE_DICTIONARY_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class SecondaryStructureDictionary : public FeatureDictionary
{
public:
  static FeatureDictionaryPtr getInstance();

  enum Type
  {
    helix = 0, // G (threeTurnHelix) or H (alphaHelix)
    sheet, // B (residueInIsolatedBridge) or E (extendedStrandInSheet )
    other  // I, T, S or C
  };

  static SecondaryStructureDictionary::Type getIndexFromDSSPElement(const String& dsspElement);

private:
  SecondaryStructureDictionary();
};

class DSSPSecondaryStructureDictionary : public FeatureDictionary
{
public:
  static FeatureDictionaryPtr getInstance();

  enum Type
  {
    // eight states:
    threeTurnHelix = 0,        // G
    alphaHelix,                // H
    piHelix,                   // I
    hydrogenBondedTurn,        // T
    extendedStrandInSheet,     // E
    residueInIsolatedBridge,   // B
    bend,                      // S
    coil                       // C -
  };

  static const String oneLetterCodes;

private:
  DSSPSecondaryStructureDictionary();
};

class SolventAccesibility2StateDictionary : public FeatureDictionary
{
public:
  static FeatureDictionaryPtr getInstance();

  enum Type
  {
    burried = 0,
    exposed,
  };

private:
  SolventAccesibility2StateDictionary();
};

class OrderDisorderDictionary : public FeatureDictionary
{
public:
  static FeatureDictionaryPtr getInstance();

  enum Type
  {
    order = 0,
    disorder,
  };

private:
  OrderDisorderDictionary();
};
  
class AminoAcidPropertyDictionary : public FeatureDictionary
{
public:
  static FeatureDictionaryPtr getInstance();
  
private:
  AminoAcidPropertyDictionary();
};


}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_SECONDARY_STRUCTURE_DICTIONARY_H_
