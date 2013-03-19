/*----------------------------------------------------------.-----------------------------.
 | Filename: ProbabilityOfImprovementSelectionCriterion.h   | Probability Of Improvement  |
 | Author  : Denny Verbeeck                                 | Selection Criterion         |
 | Started : 27/02/2013 14:44                               |                             |
 `----------------------------------------------------------/                             |
                                |                                                         |
                                `--------------------------------------------------------*/

#ifndef ML_PROBABILITY_OF_IMPROVEMENT_SELECTIN_CRITERION_H_
# define ML_PROBABILITY_OF_IMPROVEMENT_SELECTIN_CRITERION_H_

# include <ml/SelectionCriterion.h>
# include <ml/RandomVariable.h>
# include <ml/Sampler.h>
# include <ml/Fitness.h>

namespace lbcpp
{

/** Class for probability of improvement selection.
 *  This class's evaluate() function returns the probability of improving upon
 *  the current best solution, with at least improvementFactor percent.
 */

class ProbabilityOfImprovementSelectionCriterion : public SelectionCriterion
{
public:
  ProbabilityOfImprovementSelectionCriterion(FitnessPtr& bestFitness, double improvementFactor) 
    : bestFitness(bestFitness), improvementFactor(improvementFactor) {}
  ProbabilityOfImprovementSelectionCriterion(FitnessPtr& bestFitness) : bestFitness(bestFitness), improvementFactor(0.1) {}
  ProbabilityOfImprovementSelectionCriterion() : bestFitness(*(FitnessPtr* )0), improvementFactor(0.1) {}
  
  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = 1.0;}
  
  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    ScalarVariableMeanAndVariancePtr pred = object.staticCast<ScalarVariableMeanAndVariance>();
    double mean = pred->getMean();
    double stddev = pred->getStandardDeviation();
    double curBest = bestFitness->toDouble(); // currentBest should be Fitness with 1 value
    curBest = curBest + (originalProblem->getObjective(0)->isMinimization() ? - improvementFactor * fabs(curBest) : improvementFactor * fabs(curBest));
    double cdf = GaussianSampler::cumulativeDensityFunction(curBest, mean, stddev);
    return (originalProblem->getObjective(0)->isMinimization() ? cdf : 1 - cdf);
  }
  
protected:
  friend class ProbabilityOfImprovementSelectionCriterionClass;
  
  FitnessPtr& bestFitness;
  double improvementFactor;
};

}; /* namespace lbcpp */

#endif // !ML_PROBABILITY_OF_IMPROVEMENT_SELECTIN_CRITERION_H_
