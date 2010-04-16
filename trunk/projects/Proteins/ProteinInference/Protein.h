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
  LabelSequencePtr getAminoAcidSequence() const
    {return getObject(T("AminoAcidSequence"));}

  ScoreVectorSequencePtr getPositionSpecificScoringMatrix() const
    {return getObject(T("PositionSpecificScoringMatrix"));}

  /*
  ** Secondary Structure
  */
  LabelSequencePtr getSecondaryStructureSequence() const
    {return getObject(T("SecondaryStructureSequence"));}

  LabelSequencePtr getDSSPSecondaryStructureSequence() const
    {return getObject(T("DSSPSecondaryStructureSequence"));}

  /*
  ** Solvent Accesibility
  */
  LabelSequencePtr getSolventAccessibilitySequence() const
    {return getObject(T("SolventAccessibilitySequence"));}

protected:
  virtual bool load(InputStream& istr)
  {
    int versionNumber;
    if (!lbcpp::read(istr, versionNumber))
      return false;
    if (versionNumber != 101)
    {
      Object::error(T("Protein::load"), T("Unrecognized version number"));
      return false;
    }
    return StringToObjectMap::load(istr);
  }

  virtual void save(OutputStream& ostr) const
  {
    int versionNumber = 101;
    lbcpp::write(ostr, versionNumber);
    StringToObjectMap::save(ostr);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_PROTEIN_H_
