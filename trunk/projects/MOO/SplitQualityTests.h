/*---------------------------------------------.-----------------------------------------.
 | Filename: SplitQualityTests.h               | Experiments for testing split quality   |
 | Author  : Denny Verbeeck                    | measures                                |
 | Started : 19/07/2014 16:56                  |                                         |
 `---------------------------------------------/                                         |
                                  |                                                      |
                                  `-----------------------------------------------------*/

#ifndef MOO_SPLIT_QUALITY_TESTS_H_
# define MOO_SPLIT_QUALITY_TESTS_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Solver.h>
# include <ml/IncrementalLearner.h>
# include <ml/Sampler.h>
# include <math.h>
# include "SharkProblems.h"
# include "../../lib/ml/Learner/IncrementalLearnerBasedLearner.h"
# include "../../lib/ml/Loader/ArffLoader.h"

namespace lbcpp
{

class SplitQualityTests : public WorkUnit
{
public:
  SplitQualityTests() {}

  virtual ObjectPtr run(ExecutionContext& context)
  {
    IncrementalSplittingCriterionPtr extMauve = hoeffdingBoundExtendedMauveIncrementalSplittingCriterion(1, 0.05, 0.1);
    IncrementalSplittingCriterionPtr mauve = hoeffdingBoundMauveIncrementalSplittingCriterion(1, 0.05, 0.1);
    IncrementalSplittingCriterionPtr sdr = hoeffdingBoundStdDevReductionIncrementalSplittingCriterion(1, 0.05, 0.1);

    ScalarVariableMeanAndVariancePtr leftVariance = new ScalarVariableMeanAndVariance();
    ScalarVariableMeanAndVariancePtr rightVariance = new ScalarVariableMeanAndVariance();
    PearsonCorrelationCoefficientPtr leftCorr = new PearsonCorrelationCoefficient();
    PearsonCorrelationCoefficientPtr rightCorr = new PearsonCorrelationCoefficient();
    MultiVariateRegressionStatisticsPtr leftMVRS = new MultiVariateRegressionStatistics();
    MultiVariateRegressionStatisticsPtr rightMVRS = new MultiVariateRegressionStatistics();
    MultiVariateRegressionStatisticsPtr totalMVRS = new MultiVariateRegressionStatistics();
    

    // simuleer functie bestaande uit 2 vlakken, die splitsen op x1 == 0, maar onafhankelijk zijn van x1
    // kijk hoe de split quality van split (x1 < 0.0) evolueert
    SamplerPtr sampler = uniformSampler();
    sampler->initialize(context, new ScalarVectorDomain(std::vector<std::pair<double, double> >(2, std::make_pair(-2.0, 2.0))));
    double x1 = 0.0;
    double x2 = 0.5;
    double y = functionValue(x1, x2);
    leftVariance->push(y);
    leftCorr->push(x1, y);

    context.enterScope("Pushing samples");
    for (size_t i = 0; i < 200; ++i)
    {
      context.enterScope("Sample " + string((int)i));
      context.resultCallback("sample", i);
      DenseDoubleVectorPtr sample = sampler->sample(context).staticCast<DenseDoubleVector>();
      x1 = sample->getValue(0);
      x2 = sample->getValue(1);
      double y = functionValue(x1, x2);
      if (x1 <= 0.0)
      {
        leftVariance->push(y);
        leftCorr->push(x1, y);
        leftMVRS->push(sample, y);
      }
      else
      {
        rightVariance->push(y);
        rightCorr->push(x1, y);
        rightMVRS->push(sample, y);
      }
      totalMVRS->push(sample, y);
      std::pair<size_t, double> extMauveResult = extMauveSplitQuality(extMauve, leftMVRS, rightMVRS);
      if (i > 10)
      {
        context.resultCallback("extMauveAttribute", extMauveResult.first);
        context.resultCallback("extMauveQuality", extMauveResult.second);
        context.resultCallback("Mauve", mauve->splitQuality(leftVariance, leftCorr, rightVariance, rightCorr));
        context.resultCallback("sdr", sdr->splitQuality(leftVariance, leftCorr, rightVariance, rightCorr));
        context.resultCallback("Total left RSD", leftMVRS->getResidualStandardDeviation());
        context.resultCallback("Total right RSD", leftMVRS->getResidualStandardDeviation());
        context.resultCallback("Total RSD", totalMVRS->getResidualStandardDeviation());
      }
      context.leaveScope();
    }
    context.leaveScope();

    return new Boolean(true);
  }

  double functionValue(double x1, double x2)
  {
    if (x1 > 0)
      return 2 * x2 + 1;
    else
      return 4 * x2 - 5;
  }

  std::pair<size_t, double> extMauveSplitQuality(IncrementalSplittingCriterionPtr extMauve, MultiVariateRegressionStatisticsPtr left, MultiVariateRegressionStatisticsPtr right)
  {    
    double quality = -DBL_MAX;
    size_t attr = 0;
    for (size_t i = 0; i < left->getNumAttributes(); ++i)
    {
      double current = extMauve->splitQuality(ScalarVariableMeanAndVariancePtr(), left->getStats(i), ScalarVariableMeanAndVariancePtr(), right->getStats(i));
      if (current > quality)
      {
        quality = current;
        attr = i;
      }
    }
    return std::make_pair(attr, quality);
  }

};


} /* namespace lbcpp */

#endif //!MOO_SPLIT_QUALITY_TESTS_H_