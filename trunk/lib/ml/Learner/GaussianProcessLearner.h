/*-----------------------------------------.---------------------------------.
 | Filename: GaussianProcessLearner.h      | Gaussian Process Learner        |
 | Author  : Denny Verbeeck                |                                 |
 | Started : 27/03/2013 12:44              |                                 |
 `-----------------------------------------/                                 |
                                |                                            |
                                `-------------------------------------------*/

#ifndef ML_LEARNER_GAUSSIAN_PROCESS_H_
# define ML_LEARNER_GAUSSIAN_PROCESS_H_

# include <ml/Solver.h>
# include <ml/Expression.h>
# include <ReClaM/GaussianProcess.h>
# include <ReClaM/KernelFunction.h>
# include <ReClaM/Svm.h>
# include <Array/Array.h>

namespace lbcpp
{
  
class GaussianProcessExpression : public Expression
{
public:
  GaussianProcessExpression() : Expression(doubleClass), gaussianProcess(new GaussianProcess(new SVM(new NormalizedRBFKernel()))) {}
  virtual ~GaussianProcessExpression()
    {delete gaussianProcess;}
  
  
  virtual ObjectPtr compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs) const
  {
    Array<double> inputArray = Array<double>(inputs.size());
    for (size_t i = 0; i < inputs.size(); ++i)
      inputArray(i) = inputs[i]->toDouble();
    double mean = (*gaussianProcess)(inputArray);
    
    double kpp = gaussianProcess->getSVM()->getKernel()->eval(inputArray, inputArray);
    Array<double> cinv = gaussianProcess->getCInv();
    Array<double> kp = Array<double>(1, trainingInputs.dim(0));
    double var = 0.0;
    for (size_t i = 0; i < trainingInputs.dim(0); ++i)
    {
      double val = 0.0;
      for (size_t j = 0; j < trainingInputs.dim(0); ++j)
      {
        val += cinv(i,j) * trainingInputs(0,j);
      }
      var += val * trainingInputs(0,i);
    }
    var = kpp - var;
    return new ScalarVariableConstMeanAndVariance(mean, var);
  }
  
  GaussianProcess& getGaussianProcess()
    {return *gaussianProcess;}
  
  void setFactors(std::vector<double> factors)
    {this->factors = factors;}
  
  void setTrainingInputs(Array<double> trainingInputs)
    {this->trainingInputs = trainingInputs;}
  
protected:
  GaussianProcess* gaussianProcess;
  std::vector<double> factors;          /*< The normalization factors */
  Array<double> trainingInputs;         /*< Training inputs           */
                      
  
  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const
  {
    if (getType() == doubleClass)
    {
      DVectorPtr vector = new DVector(indices->size());
      size_t i = 0;
      for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
        vector->set(i++, Double::get(compute(context, data->getRow(*it))));
      return new DataVector(indices, vector);
    }
    else
    {
      OVectorPtr vector = new OVector(indices->size());
      size_t i = 0;
      for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
        vector->set(i++, compute(context, data->getRow(*it)));
      return new DataVector(indices, vector);
    }
  }
};

typedef ReferenceCountedObjectPtr<GaussianProcessExpression> GaussianProcessExpressionPtr;
  
class GaussianProcessLearner : public Solver
{
public:
  virtual void runSolver(ExecutionContext& context)
  {
    GaussianProcessExpressionPtr gp = new GaussianProcessExpression();
    
    SupervisedLearningObjectivePtr objective = problem->getObjective(0).staticCast<SupervisedLearningObjective>();
    TablePtr data = objective->getData();
    // values need to be normalized to [-1,1]
    
    std::vector<double> minValues(objective->getNumVariables() - 1,  DBL_MAX);
    std::vector<double> maxValues(objective->getNumVariables() - 1, -DBL_MAX);
    for (size_t i = 0; i < data->getNumRows(); ++i)
    {
      std::vector<ObjectPtr> row = data->getRow(i);
      for (size_t j = 0; j < row.size() - 1; ++j)
      {
        if (minValues[j] > row[j]->toDouble())
          minValues[j] = row[j]->toDouble();
        if (maxValues[j] < row[j]->toDouble())
          maxValues[j] = row[j]->toDouble();
      }
    }
    std::vector<double> factors(objective->getNumVariables() - 1);
    for (size_t i = 0; i < objective->getNumVariables() - 1; ++i)
      factors[i] = 2 / (maxValues[i] - minValues[i]);
      
    Array<double> inputs = Array<double>(data->getNumRows(), data->getNumColumns() - 1);
    Array<double> supervisions = Array<double>(data->getNumRows(), 1);
    for (size_t i = 0; i < data->getNumRows(); ++i)
    {
      std::vector<ObjectPtr> row = data->getRow(i);
      for (size_t j = 0; j < row.size() - 1; ++j)
        //inputs(i,j) = (row[j]->toDouble() - minValues[j]) * factors[j] - 1; // rescale to [-1,1]
        inputs(i,j) = row[j]->toDouble();
      supervisions(i,0) = row.back()->toDouble();
    }
    gp->getGaussianProcess().train(inputs, supervisions);
    gp->setTrainingInputs(inputs);
    //gp->setFactors(factors);
    evaluate(context, gp);
  }
};
  
}; /* namespace lbcpp */

#endif // !ML_LEARNER_GAUSSIAN_PROCESS_H_
