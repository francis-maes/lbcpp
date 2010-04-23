/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceStepHelper.h   | A base class for simplying      |
| Author  : Francis Maes                   |  protein InferenceSteps         |
| Started : 23/04/2010 12:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_STEP_HELPER_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_HELPER_H_

# include "ProteinResidueFeatures.h"

namespace lbcpp
{

// helper class for InferenceStep that have Proteins as inputs and as supervision
class ProteinInferenceStepHelper
{
public:
  ProteinInferenceStepHelper(const String& targetName)
    : targetName(targetName) {}
  ProteinInferenceStepHelper() {}

  String getTargetName() const
    {return targetName;}

  void setTargetName(const String& targetName)
    {this->targetName = targetName;}

  ObjectPtr createEmptyOutput(ObjectPtr input) const
    {jassert(targetName.isNotEmpty()); return getProtein(input)->createEmptyObject(targetName);}

  ObjectPtr getTarget(ObjectPtr object) const
    {jassert(targetName.isNotEmpty()); return getProtein(object)->getObject(targetName);}

protected:
  String targetName;

  ProteinPtr getProtein(ObjectPtr object) const
  {
    ProteinPtr protein = object.dynamicCast<Protein>();
    jassert(protein);
    return protein;
  }

  size_t getProteinLength(ObjectPtr object) const
    {return getProtein(object)->getLength();}

  bool load(InputStream& istr)
    {return lbcpp::read(istr, targetName);}

  void save(OutputStream& ostr) const
    {lbcpp::write(ostr, targetName);}
};

class ProteinResidueRelatedInferenceStepHelper : public ProteinInferenceStepHelper
{
public:
  ProteinResidueRelatedInferenceStepHelper(const String& targetName, ProteinResidueFeaturesPtr features)
    : ProteinInferenceStepHelper(targetName), features(features) {}
  ProteinResidueRelatedInferenceStepHelper() {}

protected:
  ProteinResidueFeaturesPtr features;

  bool load(InputStream& istr)
    {return ProteinInferenceStepHelper::load(istr) && lbcpp::read(istr, features);}

  void save(OutputStream& ostr) const
    {ProteinInferenceStepHelper::save(ostr); lbcpp::write(ostr, features);}
};

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_HELPER_H_
