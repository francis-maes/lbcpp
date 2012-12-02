/*-----------------------------------------.---------------------------------.
| Filename: ExhaustiveConditionLearner.h   | Exhaustive Condition Learner    |
| Author  : Francis Maes                   |                                 |
| Started : 15/11/2012 16:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_LEARNER_EXHAUSTIVE_CONDITION_H_
# define LBCPP_ML_LEARNER_EXHAUSTIVE_CONDITION_H_

# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/Expression.h>
# include <lbcpp-ml/Sampler.h>
# include <lbcpp-ml/SplittingCriterion.h>
# include <algorithm>

namespace lbcpp
{

class ExhaustiveConditionLearner : public Solver
{
public:
  ExhaustiveConditionLearner(SamplerPtr expressionsSampler)
    : expressionsSampler(expressionsSampler) {}
  ExhaustiveConditionLearner() {}

  virtual void stopBatch(ExecutionContext& context)
    {sortedValuesCache.clear();}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    Solver::startSolver(context, problem, callback, startingSolution);
    expressionsSampler->initialize(context, vectorDomain(problem->getDomain()));
  }

  virtual void runSolver(ExecutionContext& context)
  {
    SplittingCriterionPtr splittingCriterion = problem->getObjective(0).staticCast<SplittingCriterion>();

    IndexSetPtr indices = splittingCriterion->getIndices();

    // flags[i] is true if i is included in indices and false otherwise
    // note that we use std::vector<int> which is faster than std::vector<bool>
    std::vector<int> flags(splittingCriterion->getData()->getNumRows(), 0);
    for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
      flags[*it] = 1;
    
    OVectorPtr expressions = expressionsSampler->sample(context).staticCast<OVector>();
    for (size_t i = 0; i < expressions->getNumElements(); ++i)
    {
      ExpressionPtr expression = expressions->getElement(i).staticCast<Expression>();
      std::pair<double, ExpressionPtr> p = computeCriterionWithEventualStump(context, splittingCriterion, flags, expression);
      if (p.first >= -DBL_MAX)
        addSolution(context, p.second, p.first);
    }
  }

protected:
  friend class ExhaustiveConditionLearnerClass;

  SamplerPtr expressionsSampler;

  std::pair<double, ExpressionPtr> computeCriterionWithEventualStump(ExecutionContext& context, SplittingCriterionPtr splittingCriterion, const std::vector<int>& flags, const ExpressionPtr& booleanOrScalar)
  {
     if (booleanOrScalar->getType() == booleanClass)
      return std::make_pair(splittingCriterion->evaluate(context, booleanOrScalar), booleanOrScalar);
    else
    {
      jassert(booleanOrScalar->getType()->isConvertibleToDouble());
      double value;
      SparseDoubleVectorPtr sortedValues = getSortedValues(context, splittingCriterion, flags, booleanOrScalar);
      double threshold = findBestThreshold(context, splittingCriterion, booleanOrScalar, sortedValues, value);
      return std::make_pair(value, new FunctionExpression(stumpFunction(threshold), booleanOrScalar));
    }
  }
  
  double findBestThreshold(ExecutionContext& context, SplittingCriterionPtr splittingCriterion, const ExpressionPtr& scalar, const SparseDoubleVectorPtr& sortedDoubleValues, double& bestScore)
  {
    splittingCriterion->setPredictions(DataVector::createConstant(splittingCriterion->getIndices(), new Boolean(false)));
    splittingCriterion->ensureIsUpToDate();

    if (sortedDoubleValues->getNumValues() == 0)
    {
      bestScore = splittingCriterion->computeCriterion();
      return 0.0;
    }

    bestScore = -DBL_MAX;
    std::vector<double> bestThresholds;

    if (verbosity >= verbosityDetailed)
      context.enterScope("Find best threshold for node " + scalar->toShortString());

    size_t n = sortedDoubleValues->getNumValues();
    double previousThreshold = sortedDoubleValues->getValue(n - 1).second;
    for (int i = (int)n - 1; i >= 0; --i)
    {
      size_t index = sortedDoubleValues->getValue(i).first;
      double threshold = sortedDoubleValues->getValue(i).second;

      jassert(threshold <= previousThreshold);
      if (threshold < previousThreshold)
      {
        double e = splittingCriterion->computeCriterion();

        if (verbosity >= verbosityAll)
        {
          context.enterScope("Iteration " + string((int)i));
          context.resultCallback("threshold", (threshold + previousThreshold) / 2.0);
          context.resultCallback("splittingCriterion", e);
          context.leaveScope();
        }

        if (e >= bestScore)
        {
          if (e > bestScore)
          {
            bestThresholds.clear();
            bestScore = e;
          }
          bestThresholds.push_back((threshold + previousThreshold) / 2.0);
        }
        previousThreshold = threshold;
      }
      splittingCriterion->flipPrediction(index);
    }

    if (verbosity >= verbosityDetailed)
      context.leaveScope(PairPtr(new Pair(new Double(bestThresholds.size() ? bestThresholds[0] : 0.0), new Double(bestScore))));

    return bestThresholds.size() ? bestThresholds[bestThresholds.size() / 2] : 0; // median value
  }
  
private:
  typedef std::map<ExpressionPtr, SparseDoubleVectorPtr> SortedValuesCacheMap;
  SortedValuesCacheMap sortedValuesCache;
  
  struct SortDoubleValuesOperator
  {
    static double transformIntoValidNumber(double input)
      {return input;}

    bool operator()(const std::pair<size_t, double>& a, const std::pair<size_t, double>& b) const
      {return a.second == b.second ? a.first < b.first : a.second < b.second;}
  };

  static SparseDoubleVectorPtr sortDoubleValues(const DataVectorPtr& data)
  {
    size_t n = data->size();
    SparseDoubleVectorPtr res = new SparseDoubleVector(n);
    std::vector< std::pair<size_t, double> >& v = res->getValuesVector();
  
    bool isDouble = (data->getElementsType() == doubleClass);
    for (DataVector::const_iterator it = data->begin(); it != data->end(); ++it)
    {
      double value = isDouble ? it.getRawDouble() : it.getRawObject()->toDouble();
      if (value != DVector::missingValue)
        v.push_back(std::make_pair(it.getIndex(), value));
    }
    std::sort(v.begin(), v.end(), SortDoubleValuesOperator());
    return res;
  }

  static SparseDoubleVectorPtr computeSortedValuesSubset(const SparseDoubleVectorPtr& allValues, const IndexSetPtr& indices, const std::vector<int>& flags)
  {
    SparseDoubleVectorPtr res = new SparseDoubleVector(indices->size());
    std::vector<std::pair<size_t, double> >& resValues = res->getValuesVector();
    for (size_t i = 0; i < allValues->getNumValues(); ++i)
      if (flags[allValues->getValue(i).first] > 0)
        resValues.push_back(allValues->getValue(i));
    jassert(resValues.size() <= indices->size()); // there may be missing values
    return res;
  }

  SparseDoubleVectorPtr getSortedValues(ExecutionContext& context, SplittingCriterionPtr splittingCriterion, const std::vector<int>& flags, const ExpressionPtr& expression)
  {
    SparseDoubleVectorPtr allValues;
    SortedValuesCacheMap::const_iterator it = sortedValuesCache.find(expression);
    if (it == sortedValuesCache.end())
    {
      DataVectorPtr values = expression->compute(context, splittingCriterion->getData());
      allValues = sortDoubleValues(values); 
      sortedValuesCache[expression] = allValues;
    }
    else
      allValues = it->second;
      
    if (splittingCriterion->getIndices()->size() == splittingCriterion->getData()->getNumRows())
      return allValues;
    else
      return computeSortedValuesSubset(allValues, splittingCriterion->getIndices(), flags);
  }
};

}; /* namespace lbcpp */

#endif // LBCPP_ML_LEARNER_EXHAUSTIVE_CONDITION_H_
