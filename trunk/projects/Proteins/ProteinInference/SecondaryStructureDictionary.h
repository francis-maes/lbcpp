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

class SecondaryStructureElement
{
public:
  enum EightStateType
  {
    // eight states:
    threeTurnHelix = 0,        // G
    alphaHelix,                // H
    piHelix,                   // I
    hydrogenBondedTurn,        // T
    extendedStrandInSheet,     // E
    residueInIsolatedBridge,   // B
    bend,                      // S
    coil,                      // C -
  };

  enum ThreeStateType
  {
    helix = 0, // G (threeTurnHelix) or H (alphaHelix)
    sheet, // B (residueInIsolatedBridge) or E (extendedStrandInSheet )
    other, // I, T, S or C
  };

  SecondaryStructureElement(EightStateType type)
    : type((int)type), eightStates(true) {}

  SecondaryStructureElement(ThreeStateType type)
    : type((int)type), eightStates(false) {}

  SecondaryStructureElement(juce::tchar code, bool eightStates)
    : eightStates(eightStates)
  {
    if (eightStates)
    {
      static const String codes = T("GHITEBS");
      type = codes.indexOfChar(code);
      if (type < 0)
        type = coil;
    }
    else
    {
      jassert(code == 'H' || code == 'E' || code == 'C');
      type = (code == 'H' ? helix : (code == 'E' ? sheet : other));
    }
  }

  bool isEightStates() const
    {return eightStates;}

  SecondaryStructureElement toThreeState() const
  {
    jassert(eightStates);
    return SecondaryStructureElement(getType());
  }

  EightStateType getExtendedType() const
  {
    if (eightStates)
      return (EightStateType)type;
    else
      return (type == helix ? alphaHelix : (type == sheet ? extendedStrandInSheet : coil));
  }

  ThreeStateType getType() const
  {
    if (eightStates)
    {
      return (type == threeTurnHelix || type == alphaHelix
                ? helix : (type == residueInIsolatedBridge || type == extendedStrandInSheet ? sheet : other));
    }
    else
      return (ThreeStateType)type;
  }

  static StringDictionaryPtr getOneLetterCodes(bool eightStates)
  {
    if (eightStates)
    {
      static const juce::tchar* codes[] =
        {T("G"), T("H"), T("I"), T("T"), T("E"), T("B"), T("S"), T("_"), NULL};
      static StringDictionaryPtr dictionary = new StringDictionary(T("SecondaryStructureDSSPOneLetterCode"), codes);
      return dictionary;
    }
    else
    {
      static const juce::tchar* codes[] =
        {T("H"), T("E"), T("C"), NULL};
      static StringDictionaryPtr dictionary = new StringDictionary(T("SecondaryStructureOneLetterCode"), codes);
      return dictionary;
    }
  }
  
  juce::tchar getOneLetterCode() const
  {
    StringDictionaryPtr dictionary = getOneLetterCodes(eightStates);
    jassert(dictionary->exists((size_t)type));
    String res = dictionary->getString((size_t)type);
    jassert(res.length() == 1);
    return res[0];
  }

  String toString() const
  {
    if (eightStates)
    {
      static const juce::tchar* names[] = {
        T("Three Turn Helix"),
        T("Alpha Helix"),
        T("Pi Helix"),
        T("Hydrogen Bonded Turn"),
        T("Extended Strand in Sheet"),
        T("Residue in Isolated Bridge"),
        T("Bend"),
        T("Coil")
      };
      jassert(type >= 0 && type < (int)(sizeof (names) / sizeof (juce::tchar* )));
      return names[type];
    }
    else
    {
      static const juce::tchar* names[] = {T("Helix"), T("Sheet"), T("Other")};
      jassert(type >= 0 && type < (int)(sizeof (names) / sizeof (juce::tchar* )));
      return names[type];
    }
  }

  int getTypeIndex() const
    {return type;}

private:
  int type;
  bool eightStates;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_SECONDARY_STRUCTURE_DICTIONARY_H_
