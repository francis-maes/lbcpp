/*-----------------------------------------.---------------------------------.
| Filename: Protein.h                      | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 27/03/2010 12:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_PROTEIN_H_
# define LBCPP_PROTEIN_INFERENCE_PROTEIN_H_

# include "../InferenceData/LabelSequence.h"
# include "../InferenceData/ScoreVectorSequence.h"

namespace lbcpp
{

class Protein;
typedef ReferenceCountedObjectPtr<Protein> ProteinPtr;

class Protein : public StringToObjectMap
{
public:
  Protein(const String& name)
    : StringToObjectMap(name) {}
  Protein() {}

  size_t getLength() const
    {return getAminoAcidSequence()->size();}

  /*
  ** Primary Structure and Position Specific Scoring Matrix
  */
  void setAminoAcidSequence(LabelSequencePtr sequence)
    {setObject(T("AminoAcidSequence"), sequence);}

  LabelSequencePtr getAminoAcidSequence() const
    {return getObject(T("AminoAcidSequence"));}

  void setPositionSpecificScoringMatrix(ScoreVectorSequencePtr pssm)
    {setObject(T("PositionSpecificScoringMatrix"), pssm);}

  ScoreVectorSequencePtr getPositionSpecificScoringMatrix() const
    {return getObject(T("PositionSpecificScoringMatrix"));}

  /*
  ** Secondary Structure
  */
  void setSecondaryStructureSequence(LabelSequencePtr sequence)
    {setObject(T("SecondaryStructureSequence"), sequence);}

  LabelSequencePtr getSecondaryStructureSequence() const
    {return getObject(T("SecondaryStructureSequence"));}

  void setDSSPSecondaryStructureSequence(LabelSequencePtr sequence)
    {setObject(T("DSSPSecondaryStructureSequence"), sequence);}

  LabelSequencePtr getDSSPSecondaryStructureSequence() const
    {return getObject(T("DSSPSecondaryStructureSequence"));}

  /*
  ** Solvent Accesibility
  */
  void setSolventAccessibilitySequence(LabelSequencePtr solventAccessibility)
    {setObject(T("SolventAccessibilitySequence"), solventAccessibility);}

  LabelSequencePtr getSolventAccesibilitySequence() const
    {return getObject(T("SolventAccessibilitySequence"));}

protected:
  virtual bool load(InputStream& istr)
  {
    int versionNumber;
    if (!lbcpp::read(istr, versionNumber))
      return false;
    if (versionNumber != 100)
    {
      Object::error(T("Protein::load"), T("Unrecognized version number"));
      return false;
    }
    return StringToObjectMap::load(istr);
  }

  virtual void save(OutputStream& ostr) const
  {
    int versionNumber = 100;
    lbcpp::write(ostr, versionNumber);
    StringToObjectMap::save(ostr);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_PROTEIN_H_
