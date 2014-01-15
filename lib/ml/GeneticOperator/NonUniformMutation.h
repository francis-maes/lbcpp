/*-----------------------------------------.---------------------------------.
 | Filename: NonUniformMutation.h          | Non-Uniform Mutation Operator   |
 | Author  : Denny Verbeeck                |                                 |
 | Started : 13/01/2014 15:47              |                                 |
 `-----------------------------------------/                                 |
                                |                                            |
                                `-------------------------------------------*/

#ifndef ML_NON_UNIFORM_MUTATION_H_
# define ML_NON_UNIFORM_MUTATION_H_

# include <ml/GeneticOperator.h>
# include <ml/Domain.h>

namespace lbcpp
{

/**
 * This class implements a uniform mutation operator
 */
class NonUniformMutation : public Mutation
{
public:
  NonUniformMutation() : Mutation(0.0), perturbation(0.0), currentIteration(0), maxIterations(0) {}
  NonUniformMutation(double probability, double perturbation, size_t currentIteration, size_t maxIterations) 
    : Mutation(probability), perturbation(perturbation), currentIteration(currentIteration), maxIterations(maxIterations) {}

  virtual void execute(ExecutionContext& context, ProblemPtr problem, ObjectPtr object) const
  {
    ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
    DenseDoubleVectorPtr solution = object.staticCast<DenseDoubleVector>();
    for (size_t var = 0; var < solution->getNumValues(); ++var)
    {
      if (context.getRandomGenerator()->sampleDouble() < probability) {
        double rand = context.getRandomGenerator()->sampleDouble();
        double tmp;
        if (rand <= 0.5)
        {
          tmp = delta(context, domain->getUpperLimit(var) - solution->getValue(var));
          tmp += solution->getValue(var);
        }
        else {
          tmp = delta(context, domain->getLowerLimit(var) - solution->getValue(var));
          tmp += solution->getValue(var);
        }
        if (tmp < domain->getLowerLimit(var))
          tmp = domain->getLowerLimit(var);
        else if (tmp > domain->getUpperLimit(var))
          tmp = domain->getUpperLimit(var);

        solution->setValue(var, tmp) ;
      }
    }
  }

protected:
  friend class NonUniformMutationClass;

  double perturbation;
  size_t currentIteration;
  size_t maxIterations;

  double delta(ExecutionContext& context, double y) const
  {
    double rand = context.getRandomGenerator()->sampleDouble();
    return (y * (1.0 - pow(rand, pow((1.0 - currentIteration /(double) maxIterations), probability))));
  }
};

} /* namespace lbcpp */

#endif //!ML_NON_UNIFORM_MUTATION_H_