/*-----------------------------------------.---------------------------------.
| Filename: SecondaryStructureDictionary.h | Secondary Structure Dictionary  |
| Author  : Francis Maes                   |                                 |
| Started : 10/04/2010 14:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_SECONDARY_STRUCTURE_DICTIONARY_H_
# define LBCPP_PROTEIN_INFERENCE_SECONDARY_STRUCTURE_DICTIONARY_H_

# include "../InferenceData/LabelSequence.h"

namespace lbcpp
{

// old
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
  static LabelSequencePtr createSequenceFromDSSPSequence(const String& name, LabelSequencePtr dsspSequence);

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
  
class AminoAcidPropertyDictionary : public FeatureDictionary
{
public:
  static FeatureDictionaryPtr getInstance();
  
private:
  AminoAcidPropertyDictionary();
};

class StructuralAlphabetDictionary : public FeatureDictionary
{
public:
  static FeatureDictionaryPtr getInstance();

private:
  StructuralAlphabetDictionary();
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_SECONDARY_STRUCTURE_DICTIONARY_H_
