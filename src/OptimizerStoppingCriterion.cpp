/*-----------------------------------------.---------------------------------.
| Filename: OptimizerStoppingCriterion.cpp | Optimizers Stopping Criterions  |
| Author  : Francis Maes                   |                                 |
| Started : 09/06/2009 16:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Optimizer.h>
#include <deque>
#include <cfloat>
using namespace lbcpp;

class MaxIterationsOptimizerStoppingCriterion : public OptimizerStoppingCriterion
{
public:
  MaxIterationsOptimizerStoppingCriterion(size_t maxIterations = 0)
    : maxIterations(maxIterations) {}

  virtual std::string toString() const
    {return "MaxIterations(" + lbcpp::toString(maxIterations) + ")";}

  virtual void reset()
    {iterations = 0;}

  virtual bool isTerminated(double, double, double)
    {assert(maxIterations); ++iterations; return iterations >= maxIterations;}
    
  virtual bool isTerminated(double, const FeatureGeneratorPtr, const FeatureGeneratorPtr)
    {assert(maxIterations); ++iterations; return iterations >= maxIterations;}

  virtual void save(std::ostream& ostr) const
    {write(ostr, maxIterations);}

  virtual bool load(std::istream& istr)
    {return read(istr, maxIterations);}

private:
  size_t iterations, maxIterations;
};

OptimizerStoppingCriterionPtr lbcpp::maxIterationsStoppingCriterion(size_t maxIterations)
  {return new MaxIterationsOptimizerStoppingCriterion(maxIterations);}

class AverageImprovementOptimizerStoppingCriterion : public OptimizerStoppingCriterion
{
public:
  AverageImprovementOptimizerStoppingCriterion(double tolerance = 0.001)
    : tolerance(tolerance) {}
    
  virtual std::string toString() const
    {return "AvgImprovment(" + lbcpp::toString(tolerance) + ")";}

  virtual void reset()
    {prevs.clear();}

  bool isTerminated(double value)
  {
    if (prevs.size())
    {
      double prevVal = prevs.front();
      /*if (energy > prevVal)
        prevs.clear();
      else */if (prevs.size() >= 5)
      {
        double averageImprovement = (prevVal - value) / prevs.size();
        double relAvgImpr = value ? averageImprovement / fabs(value) : 0;
//        std::cout << "Av-Improvment: " << averageImprovement << " RealImprovment: " << relAvgImpr << " tol = " << tolerance << std::endl;
        if ((averageImprovement >= 0 && averageImprovement < DBL_EPSILON) || (relAvgImpr >= 0 && relAvgImpr <= tolerance))
          return true;
        prevs.pop_front();
      }
    }
    prevs.push_back(value);
    return false;
  }

  virtual bool isTerminated(double value, double, double)
    {return isTerminated(value);}
    
  virtual bool isTerminated(double value, const FeatureGeneratorPtr, const FeatureGeneratorPtr)
    {return isTerminated(value);}

  virtual void save(std::ostream& ostr) const
    {write(ostr, tolerance);}

  virtual bool load(std::istream& istr)
    {return read(istr, tolerance);}

private:
  double tolerance;
  std::deque<double> prevs;
};

OptimizerStoppingCriterionPtr lbcpp::averageImprovementThresholdStoppingCriterion(double tolerance)
  {return new AverageImprovementOptimizerStoppingCriterion(tolerance);}

class OrOptimizerStoppingCriterion : public OptimizerStoppingCriterion
{
public:
  OrOptimizerStoppingCriterion(OptimizerStoppingCriterionPtr criterion1, OptimizerStoppingCriterionPtr criterion2)
    : criterion1(criterion1), criterion2(criterion2) {}
  OrOptimizerStoppingCriterion() {}

  virtual std::string toString() const
    {return criterion1->toString() + " || " + criterion2->toString();}

  virtual void reset()
    {criterion1->reset(); criterion2->reset();}

  virtual bool isTerminated(double value, double parameter, double derivative)
  {
    bool t1 = criterion1->isTerminated(value, parameter, derivative);
    bool t2 = criterion2->isTerminated(value, parameter, derivative);
    return t1 || t2;
  }
  
  virtual bool isTerminated(double value, const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr gradient)
  {
    bool t1 = criterion1->isTerminated(value, parameters, gradient);
    bool t2 = criterion2->isTerminated(value, parameters, gradient);
    return t1 || t2;
  }

  virtual void save(std::ostream& ostr) const
    {write(ostr, criterion1); write(ostr, criterion2);}

  virtual bool load(std::istream& istr)
    {return read(istr, criterion1) && read(istr, criterion2);}

private:
  OptimizerStoppingCriterionPtr criterion1;
  OptimizerStoppingCriterionPtr criterion2;
};

OptimizerStoppingCriterionPtr lbcpp::logicalOr(OptimizerStoppingCriterionPtr criterion1, OptimizerStoppingCriterionPtr criterion2)
  {return new OrOptimizerStoppingCriterion(criterion1, criterion2);}

/*
** Serializable classes declaration
*/
void declareOptimizerStoppingCriterions()
{
  LBCPP_DECLARE_CLASS(MaxIterationsOptimizerStoppingCriterion);
  LBCPP_DECLARE_CLASS(AverageImprovementOptimizerStoppingCriterion);
  LBCPP_DECLARE_CLASS(OrOptimizerStoppingCriterion);
}
