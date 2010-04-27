/*-----------------------------------------.---------------------------------.
| Filename: CommonFeatureGenerators.h      | Common Feature Generators       |
| Author  : Francis Maes                   |                                 |
| Started : 26/04/2010 18:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_COMMON_FEATURE_GENERATORS_H_
# define LBCPP_INFERENCE_COMMON_FEATURE_GENERATORS_H_

# include <lbcpp/lbcpp.h>
# include "../Geometry/DihedralAngle.h"

//namespace lbcpp // FIXME: lbcpp-translator does not support feature generators that are in a namespace
//{

extern lbcpp::FeatureGeneratorPtr positiveNumberFeatures(double number, size_t numIntervalsPerUnit = 1);
extern lbcpp::FeatureGeneratorPtr positiveNumberLogFeatures(double number, size_t numIntervalsPerLog10 = 3);

extern lbcpp::FeatureGeneratorPtr multiScalePositiveNumberFeatures(double number, size_t numIntervalsPerLog10InCoarestScale = 1, size_t intervalRatio = 2, size_t numScales = 4);
extern lbcpp::FeatureGeneratorPtr multiScaleNumberFeatures(double number, size_t numIntervalsPerLog10InCoarestScale = 1, size_t intervalRatio = 2, size_t numScales = 4);

extern lbcpp::FeatureGeneratorPtr percentageFeatures(double percentage, size_t numIntervals = 20, bool cycle = false);
extern lbcpp::FeatureGeneratorPtr multiScalePercentageFeatures(double percentage, size_t numIntervalsInCoarsestScale = 1, size_t intervalRatio = 2, size_t numScales = 7);

inline lbcpp::FeatureGeneratorPtr dihedralAngleFeatures(double dihedralAngle, size_t numIntervals = 16)
  {return percentageFeatures((1.0 + dihedralAngle / M_PI) / 2.0, numIntervals, true);}

extern lbcpp::FeatureGeneratorPtr multiScaleDihedralAngleFeatures(double dihedralAngle, size_t numIntervalsInCoarsestScale = 1, size_t intervalRatio = 2, size_t numScales = 7);

//};

#endif // !LBCPP_INFERENCE_COMMON_FEATURE_GENERATORS_H_
