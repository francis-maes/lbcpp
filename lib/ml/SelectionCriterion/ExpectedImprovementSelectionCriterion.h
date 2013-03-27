/*-----------------------------------------------------.---------------------------------.
 | Filename: ExpectedImprovementSelectionCriterion.h   | Expected Improvement Selection  |
 | Author  : Denny Verbeeck                            | Criterion                       |
 | Started : 27/02/2013 15:40                          |                                 |
 `-----------------------------------------------------/                                 |
                                |                                                        |
                                `-------------------------------------------------------*/

#ifndef ML_EXPECTED_IMPROVEMENT_SELECTIN_CRITERION_H_
# define ML_EXPECTED_IMPROVEMENT_SELECTIN_CRITERION_H_

# include <ml/SelectionCriterion.h>
# include <ml/RandomVariable.h>
# include <ml/Sampler.h>
# include <ml/Fitness.h>
# include <ml/Problem.h>

namespace lbcpp
{
  
/** Class for probability of improvement selection.
 *  This class's evaluate() function returns the probability of improving upon
 *  the current best solution.
 */

class ExpectedImprovementSelectionCriterion : public SelectionCriterion
{
public:
  ExpectedImprovementSelectionCriterion(FitnessPtr& bestFitness) : bestFitness(bestFitness) {}
  ExpectedImprovementSelectionCriterion() : bestFitness(*(FitnessPtr* )0) {}
  
  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = DBL_MAX;}
  
  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    jassert(bestFitness);
    ScalarVariableMeanAndVariancePtr pred = object.staticCast<ScalarVariableMeanAndVariance>();
    double mean = pred->getMean();
    double stddev = pred->getStandardDeviation();
    if (stddev < 1e-8) return 0.0;
    double curBest = bestFitness->toDouble(); // currentBest should be Fitness with 1 value
    double delta = (originalProblem->getObjective(0)->isMinimization() ? curBest - mean : mean - curBest);
    double term = delta / stddev;
    double cdf = GaussianSampler::cumulativeDensityFunction(term, 0.0, 1.0);
    double pdf = GaussianSampler::probabilityDensityFunction(term, 0.0, 1.0);
    // expected improvement
    double ei = delta * cdf  + stddev * pdf;
    //return juce::jmax(0.0, ei);
    return ei;
  }

  FitnessPtr& getBestFitness() const
    {return bestFitness;}

protected:
  friend class ExpectedImprovementSelectionCriterionClass;
  
  FitnessPtr& bestFitness;
};

typedef ReferenceCountedObjectPtr<ExpectedImprovementSelectionCriterion> ExpectedImprovementSelectionCriterionPtr;

}; /* namespace lbcpp */

#endif // !ML_EXPECTED_IMPROVEMENT_SELECTIN_CRITERION_H_
