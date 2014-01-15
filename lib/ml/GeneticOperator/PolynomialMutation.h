/*-----------------------------------------.---------------------------------.
 | Filename: PolynomialMutation.h          | Polynomial Mutation Operator    |
 | Author  : Denny Verbeeck                |                                 |
 | Started : 10/01/2014 14:28              |                                 |
 `-----------------------------------------/                                 |
                                |                                            |
                                `-------------------------------------------*/

#ifndef ML_POLYNOMIAL_MUTATION_H_
# define ML_POLYNOMIAL_MUTATION_H_

# include <ml/GeneticOperator.h>
# include <ml/Domain.h>

namespace lbcpp
{

/**
 * This class implements a polynomial mutation operator
 */
class PolynomialMutation : public Mutation
{
public:
  PolynomialMutation() : Mutation(), distributionIndex(0.0), eta_m(0.0) {}
  PolynomialMutation(double probability, double distributionIndex = 20.0, double eta_m = 20.0) : Mutation(probability), distributionIndex(distributionIndex), eta_m(eta_m) {}

  virtual void execute(ExecutionContext& context, ProblemPtr problem, ObjectPtr object) const
  {
    double rnd, delta1, delta2, mut_pow, deltaq;
    double y, yl, yu, val, xy;
    DenseDoubleVectorPtr solution = object.staticCast<DenseDoubleVector>();
    ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
    for (size_t var=0; var < solution->getNumValues(); ++var)
    {
      if (context.getRandomGenerator()->sampleDouble() <= probability)
	    {
		    y      = solution->getValue(var);
        yl     = domain->getLowerLimit(var);              
		    yu     = domain->getUpperLimit(var);
		    delta1 = (y - yl) / (yu - yl);
		    delta2 = (yu - y) / (yu - yl);
		    rnd = context.getRandomGenerator()->sampleDouble();
		    mut_pow = 1.0 / (eta_m + 1.0);
		    if (rnd <= 0.5)
		    {
			    xy     = 1.0-delta1;
			    val    = 2.0*rnd+(1.0-2.0*rnd)*(pow(xy,(distributionIndex+1.0)));
			    deltaq =  pow(val,mut_pow) - 1.0;
		    }
		    else
		    {
			    xy = 1.0-delta2;
			    val = 2.0*(1.0-rnd)+2.0*(rnd-0.5)*(pow(xy,(distributionIndex+1.0)));
			    deltaq = 1.0 - (pow(val,mut_pow));
		    }
		    y = y + deltaq*(yu-yl);
		    if (y < yl)
			    y = yl;
		    if (y > yu)
			    y = yu;
		    solution->setValue(var, y);                           
	    }
    }
  }

protected:
  friend class PolynomialMutationClass;

  double distributionIndex;
  double eta_m;
};

} /* namespace lbcpp */

#endif //!ML_POLYNOMIAL_MUTATION_OPERATOR_H_