/*-----------------------------------------.---------------------------------.
 | Filename: GaussianProcessLearner.h      | Gaussian Process Learner        |
 | Author  : Denny Verbeeck                | Shark Wrapper                   |
 | Started : 27/03/2013 12:44              |                                 |
 `-----------------------------------------/                                 |
                                |                                            |
                                `-------------------------------------------*/

#ifndef ML_LEARNER_SHARK_GAUSSIAN_PROCESS_H_
# define ML_LEARNER_SHARK_GAUSSIAN_PROCESS_H_

# include <ml/Solver.h>
# include <ml/Expression.h>
# include <ml/Objective.h>

# undef T
# include <ReClaM/GaussianProcess.h>
# include <ReClaM/AdpBP.h>
# include <ReClaM/CG.h>
# include <ReClaM/BFGS.h>
# define T JUCE_T

namespace lbcpp
{

/**
 * Reference counted object wrapper for Shark's Gaussian Process
 */
class SharkGaussianProcess : public Object
{
public:
  /**
   * Trains the given GaussianProcess with the training data
   */
  SharkGaussianProcess(const Array<double>& train, const Array<double>& supervisions, GaussianProcess* gaussianProcess = new GaussianProcess(new SVM(new NormalizedRBFKernel())))
    : gaussianProcess(gaussianProcess) 
  {
    gaussianProcess->setBetaInv(1.0);
    gaussianProcess->setSigma(1.0);
    gaussianProcess->train(train, supervisions);
    AdpBP90a optimizer;
    //CG optimizer;
    //BFGS optimizer;
    GaussianProcessEvidence error;
    optimizer.init(*gaussianProcess);
    for (size_t i = 0; i < 5; ++i)
      optimizer.optimize(*gaussianProcess, error, train, supervisions);
  }

  /**
   * Merely stores the given GaussianProcess
   */
  SharkGaussianProcess(GaussianProcess* gaussianProcess = new GaussianProcess(new SVM(new RBFKernel(1.0)))) : gaussianProcess(gaussianProcess) {}

  ~SharkGaussianProcess()
    {delete gaussianProcess;}

  double getBetaInv() const
    {return gaussianProcess->getParameter(0);}

  double getSigma() const
    {return gaussianProcess->getParameter(1);}

  ScalarVariableMeanAndVariancePtr compute(const std::vector<ObjectPtr>& inputs)
  {
    Array<double> inputArray = Array<double>(inputs.size());
    for (size_t i = 0; i < inputs.size(); ++i)
      inputArray(i) = inputs[i]->toDouble();
    GaussianProcessVariance var;
    return new ScalarVariableConstMeanAndVariance((*gaussianProcess)(inputArray), var.error(*gaussianProcess, inputArray, Array<double>()));
  }

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
    {return gaussianProcess->compute(inputs);}
  
  SharkGaussianProcessPtr getGaussianProcess() const
    {return gaussianProcess;}

protected:
  friend class GaussianProcessExpressionClass;
  SharkGaussianProcessPtr gaussianProcess;
  
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

class SharkGaussianProcessLearner : public Solver
{
public:
  virtual void runSolver(ExecutionContext& context)
  {
    SupervisedLearningObjectivePtr objective = problem->getObjective(0).staticCast<SupervisedLearningObjective>();
    TablePtr data = objective->getData();
    std::pair<Array<double>, Array<double> > transformed = transform(data);
    SharkGaussianProcessPtr gaussianProcess = new SharkGaussianProcess(transformed.first, transformed.second);
    GaussianProcessExpressionPtr gp = new GaussianProcessExpression(gaussianProcess);
    evaluate(context, gp);
    context.resultCallback("Regularization parameter", gaussianProcess->getBetaInv());
    context.resultCallback("Sigma", gaussianProcess->getSigma());
  }

protected:
  std::vector<std::pair<double, double> > computeLimits(TablePtr data)
  {
    std::vector< std::pair<double, double> > limits(data->getNumColumns());
    ScalarVectorDomainPtr domain = problem->getDomain().dynamicCast<ScalarVectorDomain>();
    for (size_t i = 0; i < data->getNumColumns(); ++i)
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
    return limits;
  }

  /**
   * Transform the lbcpp::Table to a Shark Array<double>
   * The table has \f$n\f$ rows and \f$d + 1\f$ columns, where \f$d\f$ is the dimensionality of the input space.
   * \return The first element of the pair are the training inputs, i.e. an \f$n \times d\f$ Array
   *         The second element of the pair are the training outputs, i.e. an \f$n \times 1\f$ Array
   */
  std::pair<Array<double>, Array<double> > transform(TablePtr data)
  {
    Array<double> inputs(data->getNumRows(), data->getNumColumns() - 1);
    Array<double> outputs(data->getNumRows(), 1);
    for (size_t r = 0; r < data->getNumRows(); ++r)
    {
      for (size_t c = 0; c < data->getNumColumns() - 1; ++c)
        inputs(r, c) = data->getElement(r, c)->toDouble();
      outputs(r) = data->getElement(r, data->getNumColumns() - 1)->toDouble();
    }
    return std::pair<Array<double>, Array<double> >(inputs, outputs);
  }
};
  
}; /* namespace lbcpp */

#endif // !ML_LEARNER_SHARK_GAUSSIAN_PROCESS_H_
