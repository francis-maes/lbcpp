/*-----------------------------------------.---------------------------------.
| Filename: BBOptimization.h               | Bandit Based Optimization       |
| Author  : Francis Maes                   |  Sand Box                       |
| Started : 25/06/2011 15:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_BB_OPTIMIZATION_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_BB_OPTIMIZATION_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Learning/LossFunction.h>
# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

//////////////////////////////////////////////

class BBODatabase : public Object
{
public:
  virtual void addExample(const Variable& x, double y) = 0;
  virtual void getRelatedExamples(const Variable& x, size_t maxCount, std::vector< std::pair<Variable, double> >& res) = 0;

  virtual ContainerPtr getAllExamples() const = 0;
};

typedef ReferenceCountedObjectPtr<BBODatabase> BBODatabasePtr;

class ScalarBBODatabase : public BBODatabase
{
public:
  virtual void addExample(const Variable& x, double y)
    {examples.insert(std::make_pair(x.toDouble(), y));}

  virtual void getRelatedExamples(const Variable& x, size_t maxCount, std::vector< std::pair<Variable, double> >& res)
  {
    jassert(maxCount > 0);
    res.clear();
    res.reserve(maxCount < examples.size() ? maxCount : examples.size());

    if (examples.empty())
      return;

    double xValue = x.toDouble();

    ExamplesMap::iterator it = examples.lower_bound(xValue);
    jassert(it == examples.end() || it->first >= xValue);
    
    ExamplesMap::iterator itnext = it;
    ExamplesMap::iterator itprev;
    if (it == examples.begin())
      itprev = examples.end();
    else
      {itprev = it; --itprev;}
    
    while (res.size() < maxCount)
    {
      double dprev = (itprev == examples.end() ? DBL_MAX : fabs(itprev->first - xValue));
      double dnext = (itnext == examples.end() ? DBL_MAX : fabs(itnext->first - xValue));
      if (dprev == DBL_MAX && dnext == DBL_MAX)
        break; // no more examples

      if (dprev <= dnext)
      {
        jassert(dprev != DBL_MAX && itprev != examples.end());
        res.push_back(std::make_pair(itprev->first, itprev->second));
        if (itprev == examples.begin())
          itprev = examples.end();
        else
          --itprev;
      }
      else
      {
        jassert(dnext != DBL_MAX && itnext != examples.end());
        res.push_back(std::make_pair(itnext->first, itnext->second));
        ++itnext;
      }
    }

    jassert(res.size() == (maxCount < examples.size() ? maxCount : examples.size()));
  }

  virtual ContainerPtr getAllExamples() const
  {
    ClassPtr elementsType = pairClass(doubleType, doubleType);
    ObjectVectorPtr res = new ObjectVector(elementsType, examples.size());
    size_t i = 0;
    for (ExamplesMap::const_iterator it = examples.begin(); it != examples.end(); ++it, ++i)
      res->set(i, new Pair(elementsType, it->first, it->second));
    return res;
  }

private:
  typedef std::multimap<double, double> ExamplesMap;

  ExamplesMap examples;
};

//////////////////////////////////////////////

class NearestNeighborPredictor : public Function
{
public:
  NearestNeighborPredictor(BBODatabasePtr database, size_t numNeighbors)
    : database(database), numNeighbors(numNeighbors) {}
  NearestNeighborPredictor() : numNeighbors(0) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? doubleType : anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

  virtual DenseDoubleVectorPtr computeActivations(ExecutionContext& context, const Variable& input, const std::vector<std::pair<Variable, double> >& neighbors, double* targetOutput = NULL) const = 0;

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const Variable& input = inputs[0];
    bool hasSupervision = !inputs[1].isMissingValue();

    std::vector<std::pair<Variable, double> > neighbors;
    database->getRelatedExamples(input, numNeighbors, neighbors);
    if (hasSupervision && neighbors.size())
      neighbors.erase(neighbors.begin()); // remove correct output (leave-one-out) during training

    size_t n = neighbors.size();
    if (!n)
      return 0.0;
    else if (n == 1)
      return neighbors[0].second;

    double targetOutput;
    if (hasSupervision)
      targetOutput = inputs[1].getDouble();
    DenseDoubleVectorPtr activations = computeActivations(context, input, neighbors, hasSupervision ? &targetOutput : NULL);

    double res = 0.0;
    double weightSum = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      double weight = activations ? activations->getValue(i) : 0.0;
      res += weight * neighbors[i].second;
      weightSum += weight;
    }
    if (weightSum)
      res /= weightSum;
    return res;
  }

protected:
  friend class NearestNeighborPredictorClass;

  BBODatabasePtr database;
  size_t numNeighbors;
};

typedef ReferenceCountedObjectPtr<NearestNeighborPredictor> NearestNeighborPredictorPtr;

class GaussianNearestNeighborPredictor : public NearestNeighborPredictor
{
public:
  GaussianNearestNeighborPredictor(BBODatabasePtr database, size_t numNeighbors, double width)
    : NearestNeighborPredictor(database, numNeighbors), width(width) {}
  GaussianNearestNeighborPredictor() {}

  static double squareDistance(const Variable& v1, const Variable& v2)
  {
    double delta = v1.getDouble() - v2.getDouble();
    return delta * delta;
  }

  virtual DenseDoubleVectorPtr computeActivations(ExecutionContext& context, const Variable& input, const std::vector<std::pair<Variable, double> >& neighbors, double* targetOutput) const
  {
    size_t n = neighbors.size();

    double Z = -1.0 / (2.0 * width * width);
    DenseDoubleVectorPtr res(new DenseDoubleVector(n, 0.0));
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, exp(Z * squareDistance(input, neighbors[i].first)));
    return res;
  }

protected:
  friend class GaussianNearestNeighborPredictorClass;

  double width;
};

//////////////////////////////////////////////////

class BBOSupervisionInfo : public Object
{
public:
  size_t getNumNeighbors() const
    {return neighborValues.size();}

  double getNeighborValue(size_t index) const
    {return neighborValues[index];}

  double getCorrectOutput() const
    {return correctOutput;}

  std::vector<double> neighborValues;
  double correctOutput;
};

typedef ReferenceCountedObjectPtr<BBOSupervisionInfo> BBOSupervisionInfoPtr;

extern ClassPtr bboSupervisionInfoClass;

class BernoulliProbabilityLossFunction : public RegressionLossFunction
{
public:
  virtual bool isDerivable() const
    {return true;}

  virtual void computeRegressionLoss(double p, double target, double* output, double* derivative) const
  {
    jassert(isNumberValid(p));
    bool isActive = target > 0.5;
    if (output)
      //*output = isActive ? -log(p) : -log(1 - p);
      *output = isActive ? 1 - p : p; // probability of missing it
    if (derivative)
      *derivative = isActive ? -1 : 1;//(-1.0 / p) : (-1.0 / (1 - p));
  }
};

class LearnableNearestNeighborPredictorLossFunction : public ScalarVectorFunction
{
public:
  LearnableNearestNeighborPredictorLossFunction(RegressionLossFunctionPtr outputLoss, bool useExponentialScores)
    : outputLoss(outputLoss), useExponentialScores(useExponentialScores) {}

  virtual bool isDerivable() const
    {return outputLoss->isDerivable();}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? bboSupervisionInfoClass : (TypePtr)denseDoubleVectorClass();}

  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& scores,
      const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    BBOSupervisionInfoPtr supervision = otherInputs->getObjectAndCast<BBOSupervisionInfo>();
    jassert(supervision);

    size_t n = supervision->getNumNeighbors();

    double invZ = 0.0;
    double predictedOutput;

    if (useExponentialScores)
    {
      // avoid numerical errors due to too big exponentials
      double highestScore = scores ? scores->getMaximumValue() : 0.0;
      double tmpZ = 0.0;
      double tmpW = 0.0;
      for (size_t i = 0; i < n; ++i)
      {
        double score = scores ? scores->getValue(i) : 0.0;
        double e = exp(score - highestScore);
        tmpZ += e;
        tmpW += e * supervision->getNeighborValue(i);
      }
      predictedOutput = tmpW / tmpZ;
      double Z = tmpZ * exp(highestScore);
      invZ = isNumberValid(Z) && Z ? 1.0 / Z : 1e-12;
      jassert(isNumberValid(predictedOutput) && isNumberValid(invZ));
    }
    else
    {
      double Z = 0.0; // sum of all weights
      double W = 0.0; // sum of all weights * outputValues
      for (size_t i = 0; i < n; ++i)
      {
        double score = scores ? scores->getValue(i) : 0.0;
        Z += score;
        W += score * supervision->getNeighborValue(i);
      }
      jassert(isNumberValid(W) && isNumberValid(Z));
      predictedOutput = W / Z;
      invZ = Z ? 1.0 / Z : 1.0;
    }
   

    double outputLoss;
    double outputLossDerivative;
    Variable correctOutput = supervision->getCorrectOutput();
    this->outputLoss->computeScalarFunction(predictedOutput, &correctOutput, &outputLoss, &outputLossDerivative);
    if (output)
      *output = outputLoss;
    
    if (gradientTarget && invZ)
    {
      for (size_t i = 0; i < n; ++i)
      {
        double derivative = invZ * (supervision->getNeighborValue(i) - predictedOutput);
        if (useExponentialScores)
          derivative *= exp(scores ? scores->getValue(i) : 0.0);
        derivative *= outputLossDerivative;
        derivative *= gradientWeight;
        (*gradientTarget)->incrementValue(i, derivative);
      }
    }
  }

protected:
  RegressionLossFunctionPtr outputLoss;
  bool useExponentialScores;
};

class LearnableNearestNeighborPredictor : public NearestNeighborPredictor
{
public:
  LearnableNearestNeighborPredictor(BBODatabasePtr database, size_t numNeighbors, FunctionPtr relationFeatureGenerator, bool useExponentialScores = false)
    : NearestNeighborPredictor(database, numNeighbors), relationFeatureGenerator(relationFeatureGenerator), useExponentialScores(useExponentialScores)
  {
    ExecutionContext& context = defaultExecutionContext();

    activationPredictor = linearLearnableFunction();
    activationsPredictor = rankingLearnableFunction(activationPredictor);

    StochasticGDParametersPtr parameters = new StochasticGDParameters(constantIterationFunction(10.0), StoppingCriterionPtr());

    RegressionLossFunctionPtr regressionLoss = new BernoulliProbabilityLossFunction();

    parameters->setLossFunction(new LearnableNearestNeighborPredictorLossFunction(regressionLoss, useExponentialScores));
    activationsPredictor->setOnlineLearner(parameters->createOnlineLearner(context));
    setBatchLearner(parameters->createBatchLearner(context));

    setEvaluator(regressionEvaluator());
  }
  LearnableNearestNeighborPredictor() {}
 
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (!relationFeatureGenerator->initialize(context, doubleType, doubleType))
      return TypePtr();
    if (!activationsPredictor->initialize(context, objectVectorClass(relationFeatureGenerator->getOutputType()), bboSupervisionInfoClass))
      return TypePtr();
    hackParameters(0, -1);
    return doubleType;
  }

  virtual DenseDoubleVectorPtr computeActivations(ExecutionContext& context, const Variable& input, const std::vector<std::pair<Variable, double> >& neighbors, double* targetOutput) const
  {
    size_t n = neighbors.size();

    ObjectVectorPtr relationFeatures(new ObjectVector(relationFeatureGenerator->getOutputType(), n));
    for (size_t i = 0; i < n; ++i)
      relationFeatures->set(i, relationFeatureGenerator->compute(context, input, neighbors[i].first).getObjectAndCast<DoubleVector>());

    Variable supervision;
    if (targetOutput)
    {
      BBOSupervisionInfoPtr info = new BBOSupervisionInfo();
      info->correctOutput = *targetOutput;
      info->neighborValues.resize(n);
      for (size_t i = 0; i < n; ++i)
        info->neighborValues[i] = neighbors[i].second;
      supervision = info;
    }

    DenseDoubleVectorPtr activations = activationsPredictor->compute(context, relationFeatures, supervision).getObjectAndCast<DenseDoubleVector>();
    jassert(!activations || activations->getNumValues() == n);
    if (useExponentialScores && activations)
      for (size_t i = 0; i < n; ++i)
        activations->setValue(i, exp(activations->getValue(i)));
    return activations;
  }

  void hackParameters(double value1, double value2)
  {
    DenseDoubleVectorPtr hackedParameters = activationPredictor->createParameters().staticCast<DenseDoubleVector>();
    hackedParameters->setValue(0, value1);
    hackedParameters->setValue(1, value2);
    activationPredictor->setParameters(hackedParameters);
  }

protected:
  friend class LearnableNearestNeighborPredictorClass;

  FunctionPtr relationFeatureGenerator;
  NumericalLearnableFunctionPtr activationPredictor;
  NumericalLearnableFunctionPtr activationsPredictor;
  bool useExponentialScores;
};

typedef ReferenceCountedObjectPtr<LearnableNearestNeighborPredictor> LearnableNearestNeighborPredictorPtr;


//////////////////////////////////////////////

// input, neighbor -> DoubleVector
class ScalarBBORelationFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    return falseOrTrueEnumeration;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    double input = inputs[0].getDouble();
    double neighbor = inputs[1].getDouble();
    double delta = input - neighbor;
    callback.sense(0, fabs(delta) * 10); // stddev = 0.1
    callback.sense(1, delta * delta * 100); 
  }
};

class BBOptimizationSandBox : public WorkUnit
{
public:
  double computeTestFunctionExpectation(double x)
    {return (sin(13.0 * x) * sin(27.0 * x) + 1.0) / 2.0;}

  double sampleTestFunction(RandomGeneratorPtr random, double x)
  {
    double p = computeTestFunctionExpectation(x);
    return random->sampleBool(p) ? 1.0 : 0.0;
  }

  virtual Variable run(ExecutionContext& context)
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    BBODatabasePtr database = new ScalarBBODatabase();
    
    size_t N = 100;
    size_t Nneighbors = 50;
    context.enterScope(T("Training examples"));
    for (size_t i = 0; i < N; ++i)
    {
      context.enterScope(String((int)i));
      double x = random->sampleDouble(0, 1);
      double y = sampleTestFunction(random, x);
      database->addExample(x, y);
      context.resultCallback(T("x"), x);
      context.resultCallback(T("y"), y);
      context.leaveScope();
    }
    context.leaveScope();

    NearestNeighborPredictorPtr predictor1 = new GaussianNearestNeighborPredictor(database, Nneighbors, 0.01);
    LearnableNearestNeighborPredictorPtr predictor2 = new LearnableNearestNeighborPredictor(database, Nneighbors, new ScalarBBORelationFeatureGenerator(), true);

    predictor2->train(context, database->getAllExamples(), ContainerPtr(), T("Learning NN predictor"), true);

    context.enterScope(T("Curve"));
    for (double x = 0.0; x <= 1.0; x += 0.001)
    {
      context.enterScope(T("X = ") + String(x));
      context.resultCallback(T("x"), x);
      context.resultCallback(T("y*"), computeTestFunctionExpectation(x));

      context.resultCallback(T("y1"), predictor1->compute(context, x, Variable::missingValue(doubleType)).getDouble());
      context.resultCallback(T("y2"), predictor2->compute(context, x, Variable::missingValue(doubleType)).getDouble());

      context.leaveScope();
    }
    context.leaveScope();
    return true;
  }
};


}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_BB_OPTIMIZATION_H_
