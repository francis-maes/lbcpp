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
# include <ml/Objective.h>

# undef T
# include <ReClaM/GaussianProcess.h>
# define T JUCE_T

namespace lbcpp
{

class GaussianProcessExpression : public Expression
{
public:
  GaussianProcessExpression() : Expression(scalarVariableMeanAndVarianceClass), gaussianProcess(new GaussianProcess(new SVM(new NormalizedRBFKernel()))) {}
  virtual ~GaussianProcessExpression()
    {delete gaussianProcess;}
  
  virtual ObjectPtr compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs) const
  {
    Array<double> inputArray = Array<double>(limits.size());
    // TODO: check if limits.size() == inputs.size()
    for (size_t i = 0; i < limits.size(); ++i)
      inputArray(i) = 2 * (inputs[i]->toDouble() - limits[i].first) / (limits[i].second - limits[i].first) - 1; // rescale to [-1,1]
    double mean = (*gaussianProcess)(inputArray);
    
    double kpp = gaussianProcess->getSVM()->getKernel()->eval(inputArray, inputArray);
    Array<double> cinv = gaussianProcess->getCInv();
    Array<double> kp = Array<double>(trainingInputs.size());
    for (size_t i = 0; i < trainingInputs.size(); ++i)
      kp(i) = gaussianProcess->getSVM()->getKernel()->eval(inputArray, trainingInputs[i]);
    
    double var = 0.0;

    for (size_t i = 0; i < trainingInputs.size(); ++i)
    {
      double val = 0.0;
      for (size_t j = 0; j < trainingInputs.size(); ++j)
      {
        val += cinv(i,j) * kp(j);
      }
      var += val * kp(i);
    }
    var = kpp - var;
    return new ScalarVariableConstMeanAndVariance(mean, var);
  }
  
  GaussianProcess* getGaussianProcess()
    {return gaussianProcess;}
  
  void setLimits(std::vector< std::pair<double, double> > limits)
    {this->limits = limits;}
  
  void setTrainingInputs(std::vector< Array<double> > trainingInputs)
    {this->trainingInputs = trainingInputs;}

protected:
  GaussianProcess* gaussianProcess;
  std::vector< std::pair<double, double> > limits;          /*< The domain limits, for normalization */
  std::vector< Array<double> > trainingInputs;              /*< Training inputs                      */
  
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
    // TODO if problemdomain is scalarvectordomain: use those values as min and max
    // otherwise use min and max of training inputs
    SupervisedLearningObjectivePtr objective = problem->getObjective(0).staticCast<SupervisedLearningObjective>();
    TablePtr data = objective->getData();
    // values need to be normalized to [-1,1]
    
    std::vector< std::pair<double, double> > limits(objective->getNumVariables() - 1);
    for (size_t i = 0; i < objective->getNumVariables() -1; ++i)
      limits[i] = std::pair<double, double>(DBL_MAX, -DBL_MAX);
    for (size_t i = 0; i < data->getNumRows(); ++i)
    {
      std::vector<ObjectPtr> row = data->getRow(i);
      for (size_t j = 0; j < row.size() - 1; ++j)
      {
        if (limits[j].first > row[j]->toDouble())
          limits[j].first = row[j]->toDouble();
        if (limits[j].second < row[j]->toDouble())
          limits[j].second = row[j]->toDouble();
      }
    }
      
    Array<double> inputs = Array<double>(data->getNumRows(), data->getNumColumns() - 1);
    std::vector< Array<double> > trainingInputs(data->getNumRows());
    Array<double> supervisions = Array<double>(data->getNumRows(), 1);
    for (size_t i = 0; i < data->getNumRows(); ++i)
    {
      std::vector<ObjectPtr> row = data->getRow(i);
      Array<double> trainingExample(data->getNumColumns() - 1);
      for (size_t j = 0; j < row.size() - 1; ++j)
      {
        inputs(i,j) = 2 * (row[j]->toDouble() - limits[j].first) / (limits[j].second - limits[j].first) - 1; // rescale to [-1,1]
        trainingExample(j) = inputs(i,j);
      }
      trainingInputs[i] = trainingExample;
      supervisions(i,0) = row.back()->toDouble();
    }
    gp->getGaussianProcess()->train(inputs, supervisions);
    gp->setLimits(limits);
    gp->setTrainingInputs(trainingInputs);
    evaluate(context, gp);
  }
};
  
}; /* namespace lbcpp */

#endif // !ML_LEARNER_GAUSSIAN_PROCESS_H_
