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

/**
 * Reference counted object wrapper for Shark's Gaussian Process
 */
class SharkGaussianProcess : public Object
{
public:
  SharkGaussianProcess(GaussianProcess* gaussianProcess = new GaussianProcess(new SVM(new NormalizedRBFKernel())))
    : gaussianProcess(gaussianProcess) {}

  ~SharkGaussianProcess()
    {delete gaussianProcess;}

  double predict(Array<double> input)
    {return (*gaussianProcess)(input);}

  SVM* getSVM()
    {return gaussianProcess->getSVM();}

  Array<double> getCInv()
    {return gaussianProcess->getCInv();}

  void train(Array<double> train, Array<double> supervisions)
    {gaussianProcess->train(train, supervisions);}

  virtual ObjectPtr clone(ExecutionContext& context)
    {return ReferenceCountedObjectPtr<SharkGaussianProcess>(new SharkGaussianProcess(gaussianProcess));}

protected:
  GaussianProcess* gaussianProcess;
};

typedef ReferenceCountedObjectPtr<SharkGaussianProcess> SharkGaussianProcessPtr;

class GaussianProcessExpression : public Expression
{
public:
  GaussianProcessExpression(SharkGaussianProcessPtr gaussianProcess = new SharkGaussianProcess())
    : Expression(scalarVariableMeanAndVarianceClass), gaussianProcess(gaussianProcess) {}
  
  virtual ObjectPtr compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs) const
  {
    Array<double> inputArray = Array<double>(limits.size());
    // TODO: check if limits.size() == inputs.size()
    for (size_t i = 0; i < limits.size(); ++i)
      inputArray(i) = 2 * (inputs[i]->toDouble() - limits[i].first) / (limits[i].second - limits[i].first) - 1; // rescale to [-1,1]
      //inputArray(i) = inputs[i]->toDouble();
    double mean = gaussianProcess->predict(inputArray);
    
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
  
  SharkGaussianProcessPtr getGaussianProcess() const
    {return gaussianProcess;}
  
  void setLimits(std::vector< std::pair<double, double> > limits)
    {this->limits = limits;}
  
  void setTrainingInputs(std::vector< Array<double> > trainingInputs)
    {this->trainingInputs = trainingInputs;}

  virtual ObjectPtr clone(ExecutionContext& context) const
  {
    // we need to override this because of the std::vector< Array<double> > can not be in the introspection xml
    GaussianProcessExpression* res = new GaussianProcessExpression(gaussianProcess);
    res->limits = limits;
    res->trainingInputs = trainingInputs;
    return ReferenceCountedObjectPtr<GaussianProcessExpression>(res);
  }

protected:
  SharkGaussianProcessPtr gaussianProcess;
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
    
    // values need to be normalized to [-1,1]
    SupervisedLearningObjectivePtr objective = problem->getObjective(0).staticCast<SupervisedLearningObjective>();
    TablePtr data = objective->getData();
    std::vector< std::pair<double, double> > limits(objective->getNumVariables());
    ScalarVectorDomainPtr domain = problem->getDomain().dynamicCast<ScalarVectorDomain>();
    if (domain) // if problemdomain is scalarvectordomain: use those values as min and max
    {
      for (size_t i = 0; i < domain->getNumDimensions(); ++i)
        limits[i] = std::pair<double, double>(domain->getLowerLimit(i), domain->getUpperLimit(i));
    }
    else        // otherwise use min and max of training inputs
    {
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
        jassert(limits[j].second > limits[j].first)
        inputs(i,j) = 2 * (row[j]->toDouble() - limits[j].first) / (limits[j].second - limits[j].first) - 1; // rescale to [-1,1]
        //inputs(i,j) = row[j]->toDouble();
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
