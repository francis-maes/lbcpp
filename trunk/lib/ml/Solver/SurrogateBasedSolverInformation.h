/*--------------------------------------------.-------------------------------------.
| Filename: SurrogateBasedSolverInformation.h | Surrogate Based Solver Information  |
| Author  : Francis Maes                      |									               	    |
| Started : 26/03/2013 19:13                  |										                  |
`---------------------------------------------/                                     |
                               |                                                    |
                               `---------------------------------------------------*/

#ifndef ML_SOLVER_SURROGATE_BASED_INFORMATION_H_
# define ML_SOLVER_SURROGATE_BASED_INFORMATION_H_

# include <ml/Solver.h>
# include <ml/Problem.h>
# include <ml/SelectionCriterion.h>

namespace lbcpp
{

class SurrogateBasedSolverInformationLayer : public Object
{
public:
  SurrogateBasedSolverInformationLayer(string name) : name(name) {}

  string getName()
    {return name;}

  /** Compute the value of this layer at the point defined by vector **/
  virtual double computeValue(ExecutionContext& context, DenseDoubleVectorPtr vector) = 0;

protected:
  string name;

  std::vector<ObjectPtr> doubleVectorToStdVector(DenseDoubleVectorPtr vector)
  {
    std::vector<ObjectPtr> objectVector(vector->getNumElements());
    for (size_t i = 0; i < vector->getNumElements(); ++i)
      objectVector[i] = vector->getElement(i);
    return objectVector;
  }
};

typedef ReferenceCountedObjectPtr<SurrogateBasedSolverInformationLayer> SurrogateBasedSolverInformationLayerPtr;

/** Class for representing an objective layer **/
class ObjectiveLayer : public SurrogateBasedSolverInformationLayer
{
public:
  ObjectiveLayer(ObjectivePtr objective, string name=T("Objective")) : SurrogateBasedSolverInformationLayer(name), objective(objective) {}
  virtual double computeValue(ExecutionContext& context, DenseDoubleVectorPtr vector)
    {return objective->evaluate(context, vector);}
protected:
  ObjectivePtr objective;
};

/** Class for representing a surrogate model layer **/
class ModelLayer : public SurrogateBasedSolverInformationLayer
{
public:
  ModelLayer(ExpressionPtr surrogateModel, string name=T("Model")) : SurrogateBasedSolverInformationLayer(name), expression(surrogateModel) {}
  virtual double computeValue(ExecutionContext& context, DenseDoubleVectorPtr vector)
  {
    std::vector<ObjectPtr> objectVector = doubleVectorToStdVector(vector);
    ScalarVariableMeanAndVariancePtr prediction = expression->compute(context, objectVector).staticCast<ScalarVariableMeanAndVariance>();
    return prediction->getMean();
  }
protected:
  ExpressionPtr expression;
};

/** Class for representing a stadard deviation of the surrogate model prediction layer **/
class StdDevLayer : public SurrogateBasedSolverInformationLayer
{
public:
  StdDevLayer(ExpressionPtr surrogateModel, string name=T("Standard Deviation")) : SurrogateBasedSolverInformationLayer(name), expression(surrogateModel) {}
  virtual double computeValue(ExecutionContext& context, DenseDoubleVectorPtr vector)
  {
    std::vector<ObjectPtr> objectVector = doubleVectorToStdVector(vector);
    ScalarVariableMeanAndVariancePtr prediction = expression->compute(context, objectVector).staticCast<ScalarVariableMeanAndVariance>();
    return prediction->getStandardDeviation();
  }
protected:
  ExpressionPtr expression;
};

/** Class for representing an expected improvement layer **/
class ExpectedImprovementLayer : public SurrogateBasedSolverInformationLayer
{
public:
  ExpectedImprovementLayer(ExpressionPtr surrogateModel, SolutionVectorPtr solutionSet, 
      ProblemPtr problem, string name=T("Expected Improvement")) : SurrogateBasedSolverInformationLayer(name), expression(surrogateModel)
  {
    FitnessPtr bestFitness;
    bestFitness = solutionSet->getFitness(0);
    for (size_t i = 1; i < solutionSet->getNumSolutions(); ++i)
    {
      FitnessPtr fitness = solutionSet->getFitness(i);
      if (fitness->strictlyDominates(bestFitness))
        bestFitness = fitness;
    }
    criterion = expectedImprovementSelectionCriterion(bestFitness);
    criterion->initialize(problem);
  }

  virtual double computeValue(ExecutionContext& context, DenseDoubleVectorPtr vector)
  {
    std::vector<ObjectPtr> objectVector = doubleVectorToStdVector(vector);
    ScalarVariableMeanAndVariancePtr prediction = expression->compute(context, objectVector).staticCast<ScalarVariableMeanAndVariance>();
    return criterion->evaluate(context, prediction);
  }
protected:
  ExpressionPtr expression;
  SelectionCriterionPtr criterion;
};

/** Class for representing a probability of improvement layer **/
class ProbabilityOfImprovementLayer : public SurrogateBasedSolverInformationLayer
{
public:
  ProbabilityOfImprovementLayer(ExpressionPtr surrogateModel, SolutionVectorPtr solutionSet, 
      ProblemPtr problem, string name=T("Probability Of Improvement 10%")) : SurrogateBasedSolverInformationLayer(name), expression(surrogateModel)
  {
    FitnessPtr bestFitness;
    bestFitness = solutionSet->getFitness(0);
    for (size_t i = 1; i < solutionSet->getNumSolutions(); ++i)
    {
      FitnessPtr fitness = solutionSet->getFitness(i);
      if (fitness->strictlyDominates(bestFitness))
        bestFitness = fitness;
    }
    criterion = probabilityOfImprovementSelectionCriterion(bestFitness);
    criterion->initialize(problem);
  }

  virtual double computeValue(ExecutionContext& context, DenseDoubleVectorPtr vector)
  {
    std::vector<ObjectPtr> objectVector = doubleVectorToStdVector(vector);
    ScalarVariableMeanAndVariancePtr prediction = expression->compute(context, objectVector).staticCast<ScalarVariableMeanAndVariance>();
    return criterion->evaluate(context, prediction);
  }
protected:
  ExpressionPtr expression;
  SelectionCriterionPtr criterion;
};

/** Class for representing an optimistic selection criterion layer **/
class OptimisticSelectionCriterionLayer : public SurrogateBasedSolverInformationLayer
{
public:
  OptimisticSelectionCriterionLayer(ExpressionPtr surrogateModel, ProblemPtr problem, double optimism=1.0, string name=T("Optimistic")) : SurrogateBasedSolverInformationLayer(name), expression(surrogateModel)
  {
    criterion = optimisticSelectionCriterion(optimism);
    name = T("Optimistic " + string(optimism));
    criterion->initialize(problem);
  }

  virtual double computeValue(ExecutionContext& context, DenseDoubleVectorPtr vector)
  {
    std::vector<ObjectPtr> objectVector = doubleVectorToStdVector(vector);
    ScalarVariableMeanAndVariancePtr prediction = expression->compute(context, objectVector).staticCast<ScalarVariableMeanAndVariance>();
    return criterion->evaluate(context, prediction);
  }
protected:
  ExpressionPtr expression;
  SelectionCriterionPtr criterion;
};

/** Class for representing information about a surrogate based solver.
 *  Different types of information are stored in SurrogateBasedSolverInformationLayer objects.
 *  E.g. the problem objectives, the surrogate model prediction, a selection criterion etc.
 *  After constructing an object, you should call setProblem(), setSolutions() and/or setSurrogateModel() to make some default layers available.
 */
class SurrogateBasedSolverInformation : public Object
{
public:
  SurrogateBasedSolverInformation(size_t stepNumber = 0)
    : drawSolutions(true), layerToDraw(0), precision(0.2), colourScaleType(linear), stepNumber(stepNumber) {}

  ProblemPtr getProblem() const
    {return problem;}

  void setProblem(ProblemPtr problem)
  {
    this->problem = problem;
    setUpLayers();  
  }

  SolutionVectorPtr getSolutions() const
    {return solutions;}

  void setSolutions(SolutionVectorPtr solutions)
  {
    this->solutions = solutions;
    setUpLayers();
  }

  ExpressionPtr getSurrogateModel() const
    {return surrogateModel;}

  void setSurrogateModel(ExpressionPtr surrogateModel)
  {
    this->surrogateModel = surrogateModel;
    setUpLayers();
  }

  size_t getStepNumber() const
    {return stepNumber;}

  bool getDrawSolutions() const
    {return drawSolutions;}

  void setDrawSolutions(bool draw)
    {drawSolutions = draw;}

  double getPrecision() const
    {return precision;}

  void setPrecision(double precision)
    {this->precision = precision;}

  /** Set up standard layers for this SolverInformation object
   *  This method clears existing entries in the layers vector, and adds standard layers in the following order:
   *   One objective layer for each problem objective
   *   Model layer
   *   Standard Deviation layer
   *   Selection Criteria layers
   *  Note: this function is implicitly called after calls to setProblem(), setSurrogateModel() and setSolutions().
   */
  void setUpLayers()
  {
    layers.clear();
    if (problem)
      for (size_t i = 0; i < problem->getNumObjectives(); ++i)
        layers.push_back(new ObjectiveLayer(problem->getObjective(i), T("Objective " + string(i))));
    if (surrogateModel)
    {
      layers.push_back(new ModelLayer(surrogateModel));
      layers.push_back(new StdDevLayer(surrogateModel));
      if (solutions && problem)
        layers.push_back(new ExpectedImprovementLayer(surrogateModel, solutions, problem));
      if (problem)
      {
        layers.push_back(new OptimisticSelectionCriterionLayer(surrogateModel, problem, 1.0));
        layers.push_back(new OptimisticSelectionCriterionLayer(surrogateModel, problem, 2.0));
        layers.push_back(new OptimisticSelectionCriterionLayer(surrogateModel, problem, 5.0));
      }
    }
  }

  enum ColourScaleType
  {
    linear = 0,
    percentile
  };

  SurrogateBasedSolverInformationLayerPtr getLayerToDraw() const
    {return layers[layerToDraw];}

  void setLayerToDraw(size_t layerToDraw)
    {this->layerToDraw = layerToDraw;}

  size_t getNumLayers() const
    {return layers.size();}

  SurrogateBasedSolverInformationLayerPtr getLayer(size_t layer) const
    {return layers[layer];}

  ColourScaleType getColourScaleType() const
    {return colourScaleType;}

  void setColourScaleType(ColourScaleType newType)
    {colourScaleType = newType;}

private:
  friend class SurrogateBasedSolverInformationClass;

  bool drawSolutions;
  size_t layerToDraw;
  std::vector<SurrogateBasedSolverInformationLayerPtr> layers;
  double precision;
  ColourScaleType colourScaleType;

  ProblemPtr problem;
  SolutionVectorPtr solutions;
  ExpressionPtr surrogateModel;
  size_t stepNumber;
};

typedef ReferenceCountedObjectPtr<SurrogateBasedSolverInformation> SurrogateBasedSolverInformationPtr;

}; /* namespace lbcpp */

#endif // !ML_SOLVER_SURROGATE_BASED_INFORMATION_H_
