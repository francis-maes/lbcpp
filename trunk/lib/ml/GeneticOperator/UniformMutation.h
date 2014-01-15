/*-----------------------------------------.---------------------------------.
 | Filename: UniformMutation.h             | Uniform Mutation Operator       |
 | Author  : Denny Verbeeck                |                                 |
 | Started : 13/01/2014 15:40              |                                 |
 `-----------------------------------------/                                 |
                                |                                            |
                                `-------------------------------------------*/

#ifndef ML_UNIFORM_MUTATION_H_
# define ML_UNIFORM_MUTATION_H_

# include <ml/GeneticOperator.h>
# include <ml/Domain.h>

namespace lbcpp
{

/**
 * This class implements a uniform mutation operator
 */
class UniformMutation : public Mutation
{
public:
  UniformMutation() : Mutation(0.0), perturbation(0.0) {}
  UniformMutation(double probability, double perturbation) : Mutation(probability), perturbation(perturbation) {}

  virtual void execute(ExecutionContext& context, ProblemPtr problem, ObjectPtr object) const
  {
    ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
    DenseDoubleVectorPtr solution = object.staticCast<DenseDoubleVector>();
    for (size_t var = 0; var < solution->getNumValues(); ++var)
    {
      if (context.getRandomGenerator()->sampleDouble() < probability)
      {
        double rand = context.getRandomGenerator()->sampleDouble();
        double tmp = (rand - 0.5)*perturbation;
        tmp += solution->getValue(var);
        if (tmp < domain->getLowerLimit(var))
          tmp = domain->getLowerLimit(var);
        else if (tmp > domain->getUpperLimit(var))
          tmp = domain->getUpperLimit(var);
        solution->setValue(var, tmp);
      }
    }
  }

protected:
  friend class UniformMutationClass;

  double perturbation;

};

} /* namespace lbcpp */

#endif //!ML_UNIFORM_MUTATION_H_