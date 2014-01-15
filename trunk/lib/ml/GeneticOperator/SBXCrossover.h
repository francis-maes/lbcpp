/*-----------------------------------------.---------------------------------.
 | Filename: SBXCrossover.h                | Simulated Binary Crossover      |
 | Author  : Denny Verbeeck                |                                 |
 | Started : 15/01/2014 14:39              |                                 |
 `-----------------------------------------/                                 |
                                |                                            |
                                `-------------------------------------------*/

#ifndef ML_SBX_CROSSOVER_H_
# define ML_SBX_CROSSOVER_H_

# include <ml/GeneticOperator.h>

namespace lbcpp
{

class SBXCrossover : public Crossover
{
public:
  SBXCrossover(double probability = 0.0, double distributionIndex = 20.0) : Crossover(probability), distributionIndex(distributionIndex), EPS(1e-14) {}
  
  virtual std::vector<ObjectPtr> execute(ExecutionContext& context, ProblemPtr problem, const std::vector<ObjectPtr>& parents) const
  {
    jassert(parents.size() == 2);
    std::vector<ObjectPtr> result = std::vector<ObjectPtr>(2);
    DenseDoubleVectorPtr parent0 = parents[0].staticCast<DenseDoubleVector>();
    DenseDoubleVectorPtr parent1 = parents[1].staticCast<DenseDoubleVector>();
    DenseDoubleVectorPtr offspring0 = new DenseDoubleVector(1, 0.0);
    DenseDoubleVectorPtr offspring1 = new DenseDoubleVector(1, 0.0);
    parent0->clone(context, offspring0);
    parent1->clone(context, offspring1);
    
    ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
    
    double rand;
    double y1, y2, yL, yu;
    double c1, c2;
    double alpha, beta, betaq;
    double valueX1,valueX2;
		
		size_t numberOfVariables = parent0->getNumValues();
    
    if (context.getRandomGenerator()->sampleDouble() <= probability)
    {
      for (size_t i = 0; i < numberOfVariables; ++i)
      {
        valueX1 = parent0->getValue(i);
        valueX2 = parent1->getValue(i);
        if (context.getRandomGenerator()->sampleDouble() <= 0.5)
        {
          if (abs(valueX1 - valueX2) > EPS)
          {
            if (valueX1 < valueX2)
            {
              y1 = valueX1;
              y2 = valueX2;
            }
            else
            {
              y1 = valueX2;
              y2 = valueX1;
            }
            
            yL = domain->getLowerLimit(i) ;
            yu = domain->getUpperLimit(i) ;
            rand = context.getRandomGenerator()->sampleDouble();
            beta = 1.0 + (2.0*(y1-yL)/(y2-y1));
            alpha = 2.0 - pow(beta,-(distributionIndex+1.0));
            
            if (rand <= (1.0/alpha))
              betaq = pow((rand*alpha),(1.0/(distributionIndex+1.0)));
            else
              betaq = pow((1.0/(2.0 - rand*alpha)),(1.0/(distributionIndex+1.0)));
            
            c1 = 0.5*((y1+y2)-betaq*(y2-y1));
            beta = 1.0 + (2.0*(yu-y2)/(y2-y1));
            alpha = 2.0 - pow(beta,-(distributionIndex+1.0));
            
            if (rand <= (1.0/alpha))
              betaq = pow((rand*alpha),(1.0/(distributionIndex+1.0)));
            else
              betaq = pow((1.0/(2.0 - rand*alpha)),(1.0/(distributionIndex+1.0)));
            
            c2 = 0.5*((y1+y2)+betaq*(y2-y1));
            
            if (c1<yL) c1=yL;
            if (c2<yL) c2=yL;
            if (c1>yu) c1=yu;
            if (c2>yu) c2=yu;
            
            if (context.getRandomGenerator()->sampleDouble() <= 0.5)
            {
              offspring0->setValue(i, c2) ;
              offspring0->setValue(i, c1) ;
            }
            else
            {
              offspring0->setValue(i, c1) ;
              offspring1->setValue(i, c2) ;
            }
          }
          else
          {
            offspring0->setValue(i, valueX1) ;
            offspring1->setValue(i, valueX2) ;
          }
        }
        else {
          offspring0->setValue(i, valueX2) ;
          offspring1->setValue(i, valueX1) ;
        }
      }
    }
    
    result[0] = offspring0;
    result[1] = offspring1;
    return result;
  }
  
protected:
  friend class SBXCrossoverClass;
  
  double distributionIndex;
  const double EPS;
};
  
} /* namespace lbcpp */

#endif //!ML_SBX_CROSSOVER_H_
