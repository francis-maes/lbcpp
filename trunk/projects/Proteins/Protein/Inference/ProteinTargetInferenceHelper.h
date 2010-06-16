/*-----------------------------------------.---------------------------------.
| Filename: ProteinTargetInferenceHelper.h   | A base class for simplying      |
| Author  : Francis Maes                   |  protein InferenceSteps         |
| Started : 23/04/2010 12:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_STEP_HELPER_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_HELPER_H_

# include "ProteinResidueFeatures.h"
# include "ProteinResiduePairFeatures.h"

namespace lbcpp
{

// helper class for Inference that have Proteins as inputs and as supervision
class ProteinTargetInferenceHelper
{
public:
  ProteinTargetInferenceHelper(const String& targetName, const String& supervisionName = String::empty)
    : targetName(targetName), supervisionName(supervisionName.isEmpty() ? targetName : supervisionName) {}
  ProteinTargetInferenceHelper() {}

  String getTargetName() const
    {return targetName;}

  void setTargetName(const String& targetName)
    {this->targetName = targetName;}

protected:
  String targetName;
  String supervisionName;

  bool load(InputStream& istr)
    {return lbcpp::read(istr, targetName) && lbcpp::read(istr, supervisionName);}

  void save(OutputStream& ostr) const
    {lbcpp::write(ostr, targetName); lbcpp::write(ostr, supervisionName);}
};

class ProteinResidueRelatedInferenceStepHelper : public ProteinTargetInferenceHelper
{
public:
  ProteinResidueRelatedInferenceStepHelper(const String& targetName, ProteinResidueFeaturesPtr features, const String& supervisionName = String::empty)
    : ProteinTargetInferenceHelper(targetName, supervisionName), features(features) {}
  ProteinResidueRelatedInferenceStepHelper() {}

protected:
  ProteinResidueFeaturesPtr features;

  bool load(InputStream& istr)
    {return ProteinTargetInferenceHelper::load(istr) && lbcpp::read(istr, features);}

  void save(OutputStream& ostr) const
    {ProteinTargetInferenceHelper::save(ostr); lbcpp::write(ostr, features);}
};

class ProteinResiduePairRelatedInferenceStepHelper : public ProteinTargetInferenceHelper
{
public:
  ProteinResiduePairRelatedInferenceStepHelper(const String& targetName, ProteinResiduePairFeaturesPtr features, const String& supervisionName = String::empty)
    : ProteinTargetInferenceHelper(targetName, supervisionName), features(features) {}
  ProteinResiduePairRelatedInferenceStepHelper() {}

protected:
  ProteinResiduePairFeaturesPtr features;

  bool load(InputStream& istr)
    {return ProteinTargetInferenceHelper::load(istr) && lbcpp::read(istr, features);}

  void save(OutputStream& ostr) const
    {ProteinTargetInferenceHelper::save(ostr); lbcpp::write(ostr, features);}
};

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_HELPER_H_
