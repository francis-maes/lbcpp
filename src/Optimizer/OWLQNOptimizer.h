/*-----------------------------------------.---------------------------------.
| Filename: OWLQNOptimizer.h               | Orthant-Wise Limited memory     |
| Author  : Francis Maes                   | Quasi Newton optimizer          |
| Started : 29/03/2009 19:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_OPTIMIZER_OWLQN_H_
# define CRALGO_OPTIMIZER_OWLQN_H_

/*
** from:
** "Scalable Training of L1-regularized Log-linear Models", Galen Andrew, ICML 2007
** www.machinelearning.org/proceedings/icml2007/papers/449.pdf
*/

# include <cralgo/Optimizer.h>
# include "QuasiNewtonMemory.h"
# include "BackTrackingLineSearch.h"

namespace cralgo
{

class OWLQNOptimizer : public VectorOptimizer
{
public:
  enum {quasiNewtonMemory = 10};

  OWLQNOptimizer() : memory(quasiNewtonMemory) {}
  
  virtual bool initialize(ScalarVectorFunctionPtr function, FeatureGeneratorPtr parameters)
  {
    
  }
  
  virtual OptimizerState step(ScalarVectorFunctionPtr function, FeatureGeneratorPtr& parameters, double value, FeatureGeneratorPtr gradient)
  {
  }

protected:
  QuasiNewtonMemory memory;

////////////
  virtual bool optimize(DoubleVectorPtr& parameters, EnergyPtr energy, TaskEnvironmentPtr environment)
  {
    DoubleVectorPtr gradient;
    double value = energy->computeEnergy(parameters, &gradient);
    if (!gradient)
      return false;
    
    static const int m = 10;
    QuasiNewtonMemory memory(m);
    BackTrackingLineSearch* lineSearch;
    if (l1Regularizer > 0)
      lineSearch = new L1RegularizedBackTrackingLineSearch(energy, l1Regularizer);
    else
      lineSearch = new BackTrackingLineSearch(energy);

    IterationNumberPtr iter(new IterationNumber());
    while (true)
    {
      if (!environment->setIteration(iter, "Optimizing, f = " + String(value) + 
            " norm = " + String(parameters->l2norm()) + 
            " size = " + String((int)parameters->l0norm())))
      {
        delete lineSearch;
        return false;
      }

      DoubleVectorPtr direction = updateDirection(energy, memory, parameters, gradient, l1Regularizer);
      if (direction->l0norm() == 0)
        break;

      DoubleVectorPtr newParameters, newGradient;
      if (!lineSearch->search(parameters, gradient, direction, value, iter->iteration == 0, newParameters, newGradient))
      {
        delete lineSearch;
        environment->setError("backTrackingLineSearch failed.");
        return false;
      }
        
      if (l1Regularizer > 0)
      {
        newParameters->prune(0.0);
        if (newGradient)
          newGradient->prune(0.0);
      }
      if (!newGradient)
        return false;
      
      memory.shift(parameters, gradient, newParameters, newGradient);
      if (isTerminated(value, parameters, gradient))
        break;
      iter->iteration++;
    }
    
    delete lineSearch;
    return true;
  }

private:
  struct MakeSteepestDescDirAssign : public representation::NaryVectorOperation
  {
    MakeSteepestDescDirAssign(double l1Weight) : l1Weight(l1Weight) {}
    double l1Weight;

    virtual void process(const std::vector<double>& inputValues, std::vector< std::pair<size_t, double> >& outputValues)
    {
      double trueGradient = inputValues[0];
      double parameter = inputValues[1];
      double result = inputValues[2];
      
      result = -trueGradient;
      if (parameter == 0)
      {
        if (trueGradient < -l1Weight)
          result -= l1Weight;
        else if (trueGradient > l1Weight)
          result += l1Weight;
        else
          result = 0;
      }
      outputValues.push_back(std::make_pair(2, result));
    }
  }; 

  DoubleVectorPtr makeSteepestDescentDirection(EnergyPtr energy, const DoubleVectorPtr parameters, const DoubleVectorPtr gradient, double l1Weight)
  {
    if (l1Weight == 0)
      return MultipliedDoubleVector::create(gradient, -1.0);
    else
    {
      DoubleVectorPtr trueGradient = gradient->cloneCast<DoubleVector>();
      trueGradient->addWeightedSigns(parameters, l1Weight);
      DoubleVectorPtr res = parameters->createCompatibleVector();
 
      std::vector<DoubleVectorPtr> arguments(2);
      arguments[0] = parameters;
      arguments[1] = res;
      MakeSteepestDescDirAssign operation(l1Weight);
      trueGradient->naryOperation(operation, arguments);
      
      /* 
      for (size_t i=0; i<dim; i++) {
        if (x[i] < 0) {
          dir[i] = -grad[i] + l1weight;      // - (grad[i] - l1weight)  = - trueGradient[i]
        } else if (x[i] > 0) {
          dir[i] = -grad[i] - l1weight;     // - (grad[i] + l1weight) = - trueGradient[i]
        } else {
          if (grad[i] < -l1weight) {
            dir[i] = -grad[i] - l1weight;
          } else if (grad[i] > l1weight) {
            dir[i] = -grad[i] + l1weight;
          } else {
            dir[i] = 0;
          }
        }
      }*/    
      return res;
    }
  }
  
  struct FixDirSignsOperation : public representation::NaryVectorOperation
  {
    virtual void process(const std::vector<double>& inputValues, std::vector< std::pair<size_t, double> >& outputValues)
    {
      double direction = inputValues[0];
      double steepestDescentDirection = inputValues[1];
      if (direction * steepestDescentDirection <= 0)
        outputValues.push_back(std::make_pair(0, 0.0));
    }
  };

  DoubleVectorPtr updateDirection(EnergyPtr energy, QuasiNewtonMemory& memory, const DoubleVectorPtr parameters, const DoubleVectorPtr gradient, double l1Weight)
  {
    DoubleVectorPtr direction = makeSteepestDescentDirection(energy, parameters, gradient, l1Weight);
    //std::cout << "makeSteepestDescentDirection: gradient = " << (const char* )gradient->getShortDescription() << " direction =" << (const char* )direction->getShortDescription() << std::endl;
    if (l1Weight > 0)
    {
      DoubleVectorPtr steepestDescentDirection = direction->cloneCast<DoubleVector>();
      memory.mapDirectionByInverseHessian(direction);

      std::vector<DoubleVectorPtr> arguments(1, steepestDescentDirection);
      FixDirSignsOperation operation;
      direction->naryOperation(operation, arguments);
      direction->prune(0.0);
      //for (size_t i = 0; i < dim; i++)
      //  if (direction[i] * steepestDescentDirection[i] <= 0)
      //    direction[i] = 0;
    }
    else
    {
      //std::cout << "mapDirectionByInverseHessian: " << (const char* )direction->getShortDescription() << " -> ";
      memory.mapDirectionByInverseHessian(direction);
      //std::cout << (const char* )direction->getShortDescription() << std::endl;
    }
    return direction;
  }  
};


}; /* namespace cralgo */

#endif // !CRALGO_OPTIMIZER_OWLQN_H_
