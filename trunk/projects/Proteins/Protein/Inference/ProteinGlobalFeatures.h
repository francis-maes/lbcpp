/*-----------------------------------------.---------------------------------.
| Filename: ProteinGlobalFeatures.h        | A feature function phi :        |
| Author  : Francis Maes                   |    protein -> features          |
| Started : 05/05/2010 16:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_GLOBAL_FEATURES_H_
# define LBCPP_PROTEIN_INFERENCE_GLOBAL_FEATURES_H_

# include "../Protein.h"

namespace lbcpp
{

class ProteinGlobalFeatures : public Object
{
public:
  virtual String getName() const
    {return getClassName();}

  virtual FeatureGeneratorPtr compute(ProteinPtr protein) = 0;
};

typedef ReferenceCountedObjectPtr<ProteinGlobalFeatures> ProteinGlobalFeaturesPtr;

// numberLogFeatures(length)
extern ProteinGlobalFeaturesPtr proteinLengthFeatures(size_t numIntervalsPerLog10 = 3);
extern ProteinGlobalFeaturesPtr proteinGlobalCompositionFeatures(const String& sequenceName);

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_GLOBAL_FEATURES_H_
