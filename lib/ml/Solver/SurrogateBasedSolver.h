/*-----------------------------------------.---------------------------------.
| Filename: SurrogateBasedSolver.h         | Surrogate Based Solver          |
| Author  : Francis Maes                   |                                 |
| Started : 27/11/2012 14:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SOLVER_SURROGATE_BASED_H_
# define ML_SOLVER_SURROGATE_BASED_H_

# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/ExpressionDomain.h>
# include <ml/RandomVariable.h>
# include <ml/VariableEncoder.h>
# include <ml/SelectionCriterion.h>
# include <ml/IncrementalLearner.h>
# include <oil/UserInterface/ObjectComponent.h>
# include "../oil/UserInterface/Plot/TwoDimensionalPlotDrawable.h"

namespace lbcpp
{

// TODO: make a SurrogateBasedSolverInformation.h for this
// TODO: add relevant conditions here (num inputs = 2, kind of inputs = DenseDoubleVector, num objectives = 1)
// TODO: add the possibility to draw the selection criterion (prediction, stddev, selection criterions that is used)
// TODO-eventually: better color-scales
// TODO: understand the strange behavior on problem 3
class SurrogateBasedSolverInformation : public Object
{
public:
  SurrogateBasedSolverInformation(size_t stepNumber = 0)
    : stepNumber(stepNumber) {}

  ProblemPtr getProblem() const
    {return problem;}

  void setProblem(ProblemPtr problem)
    {this->problem = problem;}

  SolutionVectorPtr getSolutions() const
    {return solutions;}

  void setSolutions(SolutionVectorPtr solutions)
    {this->solutions = solutions;}

  ExpressionPtr getSurrogateModel() const
    {return surrogateModel;}

  void setSurrogateModel(ExpressionPtr surrogateModel)
    {this->surrogateModel = surrogateModel;}

  size_t getStepNumber() const
    {return stepNumber;}

private:
  friend class SurrogateBasedSolverInformationClass;

  ProblemPtr problem;
  SolutionVectorPtr solutions;
  ExpressionPtr surrogateModel;
  size_t stepNumber;
};

typedef ReferenceCountedObjectPtr<SurrogateBasedSolverInformation> SurrogateBasedSolverInformationPtr;

class SurrogateBasedSolverInformation2DDrawable : public TwoDimensionalPlotDrawable
{
public:
  SurrogateBasedSolverInformation2DDrawable(SurrogateBasedSolverInformationPtr information) : information(information)
  {
    xAxis = makeAxis(information->getSolutions(), 0);
    yAxis = makeAxis(information->getSolutions(), 1);
    computeBounds();
  }

  virtual Drawable* createCopy() const
    {return new SurrogateBasedSolverInformation2DDrawable(information);}

  virtual PlotAxisPtr getXAxis() const
    {return xAxis;}

  virtual PlotAxisPtr getYAxis() const
    {return yAxis;}

  virtual void draw(juce::Graphics& g, const juce::AffineTransform& transform = juce::AffineTransform::identity) const
  {
    if (!areBoundsValid())
      return;

    drawObjective(g, information->getProblem()->getObjective(0), transform);

    TwoDimensionalPlotDrawable::draw(g, transform);

    SolutionVectorPtr solutions = information->getSolutions();
    for (size_t i = 0; i < information->getStepNumber(); ++i)
    {
      DenseDoubleVectorPtr vector = solutions->getSolution(i).staticCast<DenseDoubleVector>();
      int x, y;
      getPixelPosition(vector, x, y, transform);

      juce::uint8 k = (juce::uint8)((255 * i) / information->getStepNumber());
      g.setColour(juce::Colour(k, 0, 255 - k));
      paintPoint(g, x, y);
    }
  }

protected:
  SurrogateBasedSolverInformationPtr information;
  PlotAxisPtr xAxis, yAxis;
  FitnessPtr currentFitness;

  void drawObjective(juce::Graphics& g, ObjectivePtr objective, const juce::AffineTransform& transform) const
  {
    juce::Rectangle rect = getFrameRectangle(transform);
    
    juce::AffineTransform inverseTransform = transform.inverted();

    std::vector< std::vector< double > > values(rect.getHeight() + 1);
    double lowestValue = DBL_MAX;
    double highestValue = -DBL_MAX;

    for (int y = rect.getY(); y <= rect.getBottom(); ++y)
    {
      values[y - rect.getY()].resize(rect.getWidth() + 1);
      for (int x = rect.getX(); x <= rect.getRight(); ++x)
      {
        double vx = (double)x;
        double vy = (double)y;
        inverseTransform.transformPoint(vx, vy);

        DenseDoubleVectorPtr vector(new DenseDoubleVector(2, 0.0));
        vector->setValue(0, vx);
        vector->setValue(1, vy);
        double value = objective->evaluate(defaultExecutionContext(), vector);
        values[y - rect.getY()][x - rect.getX()] = value;
        if (value < lowestValue)
          lowestValue = value;
        if (value > highestValue)
          highestValue = value;
      }
    }

    double invDelta = highestValue > lowestValue ? 1.0 / (highestValue - lowestValue + 1e-6) : 1.0;
    for (int y = rect.getY(); y <= rect.getBottom(); ++y)
      for (int x = rect.getX(); x <= rect.getRight(); ++x)
      {
        double normalizedValue = values[y - rect.getY()][x - rect.getX()] * invDelta;
        jassert(normalizedValue >= 0.0 && normalizedValue <= 1.0);
        juce::uint8 level = (juce::uint8)(255 * normalizedValue);
        g.setColour(juce::Colour(level, level, level));
        g.setPixel(x, y);
      }
  }
  
  void getPixelPosition(DenseDoubleVectorPtr vector, int& x, int& y, const juce::AffineTransform& transform) const
  {
    double dx = vector->getValue(0);
    double dy = vector->getValue(1);
    transform.transformPoint(dx, dy);
    x = (int)dx;
    y = (int)dy;
  }

  void paintPoint(juce::Graphics& g, int x, int y) const
  {
    if (information->getStepNumber() < 200)
    {
      const int pointHalfSize = 4;
      const float lineWidth = 2.f;

      float x1 = (float)(x - pointHalfSize);
      float y1 = (float)(y - pointHalfSize);
      float x2 = (float)(x + pointHalfSize);
      float y2 = (float)(y + pointHalfSize);
      g.drawLine(x1, y1, x2 + 1.f, y2 + 1.f, lineWidth);
      g.drawLine(x1, y2, x2 + 1.f, y1 + 1.f, lineWidth);
    }
    else
      g.setPixel(x, y);
  }
  
  PlotAxisPtr makeAxis(SolutionVectorPtr solutions, size_t inputDimension) const
  {
    double rangeMin = DBL_MAX;
    double rangeMax = -DBL_MAX;

    EnumerationPtr elementsEnumeration;
    for (size_t i = 0; i < solutions->getNumSolutions(); ++i)
    {
      DenseDoubleVectorPtr vector = solutions->getSolution(i).staticCast<DenseDoubleVector>();
      if (i == 0)
        elementsEnumeration = vector->getElementsEnumeration();
      jassert(inputDimension < vector->getNumValues());
      double value = vector->getValue(inputDimension);
      if (value < rangeMin)
        rangeMin = value;
      if (value > rangeMax)
        rangeMax = value;
    }
    return new PlotAxis(rangeMin, rangeMax, elementsEnumeration->getElementName(inputDimension), false);
  }
};

class SurrogateBasedSolverInformationComponent : public juce::Component, public ComponentWithPreferedSize
{
public:
  SurrogateBasedSolverInformationComponent(const SurrogateBasedSolverInformationPtr& information, const string& name)
    : information(information), drawable(NULL)
  {
    // TODO: add relevant conditions here (num inputs = 2, kind of inputs = DenseDoubleVector, num objectives = 1)
    drawable = new SurrogateBasedSolverInformation2DDrawable(information);
  }
  virtual ~SurrogateBasedSolverInformationComponent()
    {if (drawable) delete drawable;}

  virtual void paint(juce::Graphics& g)
  {
    if (information->getSolutions()->isEmpty())
    {
      paintText(g, "Empty Solutions");
      return;
    }
    if (!drawable)
    {
      paintText(g, "This case is not supported yet");
      return;
    }

    if (getWidth() > leftMargin + rightMargin && getHeight() > topMargin + bottomMargin)
      drawable->draw(g, getTransform());
  }

  virtual int getDefaultWidth() const
    {return 600;}

  virtual int getDefaultHeight() const
    {return 600;}

private:
  SurrogateBasedSolverInformationPtr information;

  juce::Drawable* drawable;
  
  enum
  {
    leftMargin = 60,
    rightMargin = 20,
    topMargin = 20,
    bottomMargin = 40
  };

  juce::AffineTransform getTransform() const
  {
    const int destX = leftMargin;
    const int destY = topMargin;
    const int destW = getWidth() - leftMargin - rightMargin;
    const int destH = getHeight() - topMargin - bottomMargin;

    float x, y, w, h;
    drawable->getBounds(x, y, w, h);
    juce::RectanglePlacement placement(juce::RectanglePlacement::stretchToFit);
    return placement.getTransformToFit(x, y, w, h, (float)destX, (float)destY, (float)destW, (float)destH);
  }

  void paintText(juce::Graphics& g, const string& text)
  {
    g.setColour(juce::Colours::grey);
    g.drawText(text, 0, 0, getWidth(), getHeight(), juce::Justification::centred, true);
  }
};

/** Class for surrogate-based optimization.
 *  In this class, the initial sampler should return an OVector with all \f$N\f$ initial samples
 *  as a result of its sample() function. The initial sample will be retrieved upon calling
 *  startSolver(), and will be added to the surrogate data in the first \f$N\f$ calls to iterateSolver.
 **/
class SurrogateBasedSolver : public IterativeSolver
{
public:
  SurrogateBasedSolver(SamplerPtr initialVectorSampler, SolverPtr surrogateSolver,
                       VariableEncoderPtr variableEncoder, SelectionCriterionPtr selectionCriterion, size_t numIterations)
    : IterativeSolver(numIterations), initialVectorSampler(initialVectorSampler), surrogateSolver(surrogateSolver),
      variableEncoder(variableEncoder), selectionCriterion(selectionCriterion) {}
  SurrogateBasedSolver() {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    IterativeSolver::startSolver(context, problem, callback, startingSolution);

    // make fitness class
    if (problem->getNumObjectives() == 1)
      fitnessClass = doubleClass;
    else
    {
      DefaultEnumerationPtr objectivesEnumeration = new DefaultEnumeration("objectives");
      for (size_t i = 0; i < problem->getNumObjectives(); ++i)
        objectivesEnumeration->addElement(context, problem->getObjective(i)->toShortString());
      fitnessClass = denseDoubleVectorClass(objectivesEnumeration, doubleClass);
    }

    // initialize the surrogate data
    initialVectorSampler->initialize(context, new VectorDomain(problem->getDomain()));
    initialSamples = initialVectorSampler->sample(context).staticCast<OVector>();

    // information
    lastInformation = SurrogateBasedSolverInformationPtr();
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    ObjectPtr object;
    
    ExpressionPtr surrogateModel;
    if (iter < initialSamples->getNumElements())
      object = initialSamples->getAndCast<Object>(iter);
    else
    {
      // learn surrogate
      if (verbosity >= verbosityDetailed)
        context.enterScope("Learn surrogate");
      surrogateModel = getSurrogateModel(context);
      if (verbosity >= verbosityDetailed)
      {
        context.resultCallback("surrogateModel", surrogateModel);
        context.leaveScope();
      }
      
      // optimize surrogate
      if (verbosity >= verbosityAll)
        context.enterScope("Optimize surrogate");
      object = optimizeSurrogate(context, surrogateModel);
      if (verbosity >= verbosityAll)
      {
        context.resultCallback("object", object);
        context.leaveScope();
      }
    }
    // evaluate point and add to training data
    FitnessPtr fitness = evaluate(context, object);
    
    if (verbosity == verbosityAll)
    {
      SurrogateBasedSolverInformationPtr information(new SurrogateBasedSolverInformation(iter));
      information->setProblem(problem);
      if (lastInformation)
        information->setSolutions(lastInformation->getSolutions());
      else
        information->setSolutions(new SolutionVector(problem->getFitnessLimits()));
      information->getSolutions()->insertSolution(object, fitness);
      information->setSurrogateModel(surrogateModel ? surrogateModel->cloneAndCast<Expression>() : ExpressionPtr());
      context.resultCallback("information", information);
      lastInformation = information;
    }

    if (verbosity >= verbosityDetailed)
      context.resultCallback("fitness", fitness);
    addFitnessSample(context, object, fitness);
    return true;
  }

protected:
  virtual ExpressionPtr getSurrogateModel(ExecutionContext& context) = 0;
  virtual void addFitnessSample(ExecutionContext& context, ObjectPtr object, FitnessPtr fitness) = 0;

protected:
  friend class SurrogateBasedSolverClass;

  SamplerPtr initialVectorSampler;
  SolverPtr surrogateSolver;
  VariableEncoderPtr variableEncoder;
  SelectionCriterionPtr selectionCriterion;
  
  OVectorPtr initialSamples;
  ClassPtr fitnessClass;

  SurrogateBasedSolverInformationPtr lastInformation;
  
  struct SurrogateBasedSelectionObjective : public Objective
  {
    SurrogateBasedSelectionObjective(VariableEncoderPtr encoder, ExpressionPtr model, SelectionCriterionPtr selectionCriterion)
      : encoder(encoder), model(model), selectionCriterion(selectionCriterion) {}
    
    virtual void getObjectiveRange(double& worst, double& best) const
      {return selectionCriterion->getObjectiveRange(worst, best);}
    
    virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
    {
      std::vector<ObjectPtr> row;
      encoder->encodeIntoVariables(context, object, row);
      return selectionCriterion->evaluate(context, model->compute(context, row));
    }
    
    VariableEncoderPtr encoder;
    ExpressionPtr model;
    SelectionCriterionPtr selectionCriterion;
  };
   
  ProblemPtr createSurrogateOptimizationProblem(ExpressionPtr surrogateModel)
  {
    ProblemPtr res = new Problem();
    res->setDomain(problem->getDomain());
    selectionCriterion->initialize(problem);
    res->addObjective(new SurrogateBasedSelectionObjective(variableEncoder, surrogateModel, selectionCriterion));

    res->setInitialGuess(initialSamples->get(0)); // FIXME: do something better here
    
    for (size_t i = 0; i < problem->getNumObjectives(); ++i)
      res->addValidationObjective(problem->getObjective(i));
    
    return res;
  }

  ObjectPtr optimizeSurrogate(ExecutionContext& context, ExpressionPtr surrogateModel)
  {
    ProblemPtr surrogateProblem = createSurrogateOptimizationProblem(surrogateModel);
    ObjectPtr res;
    FitnessPtr bestFitness;
    surrogateSolver->solve(context, surrogateProblem, storeBestSolverCallback(res, bestFitness));
    if (verbosity >= verbosityDetailed)
      context.resultCallback("surrogateObjectiveValue", bestFitness);
    return res;
  }

  std::vector<ObjectPtr> makeTrainingSample(ExecutionContext& context, ObjectPtr object, FitnessPtr fitness)
  {
    std::vector<ObjectPtr> res;
    variableEncoder->encodeIntoVariables(context, object, res);
      
    if (fitness->getNumValues() == 1)
      res.push_back(new Double(fitness->getValue(0)));
    else
    {
      DenseDoubleVectorPtr fitnessVector(new DenseDoubleVector(fitnessClass));
      for (size_t i = 0; i < fitness->getNumValues(); ++i)
        fitnessVector->setValue(i, fitness->getValue(i));
      res.push_back(fitnessVector);
    }
    return res;
  }
};
  
typedef ReferenceCountedObjectPtr<SurrogateBasedSolver> SurrogateBasedSolverPtr;

class IncrementalSurrogateBasedSolver : public SurrogateBasedSolver
{
public:
  IncrementalSurrogateBasedSolver(SamplerPtr initialVectorSampler, IncrementalLearnerPtr surrogateLearner, SolverPtr surrogateSolver,
                       VariableEncoderPtr variableEncoder, SelectionCriterionPtr selectionCriterion, size_t numIterations)
    : SurrogateBasedSolver(initialVectorSampler, surrogateSolver, variableEncoder, selectionCriterion, numIterations), surrogateLearner(surrogateLearner)
  {
  }

  IncrementalSurrogateBasedSolver() {}
  
  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    SurrogateBasedSolver::startSolver(context, problem, callback, startingSolution);
    surrogateModel = surrogateLearner->createExpression(context, fitnessClass);
  }

  virtual ExpressionPtr getSurrogateModel(ExecutionContext& context)
    {return surrogateModel;}
  
  virtual void addFitnessSample(ExecutionContext& context, ObjectPtr object, FitnessPtr fitness)
    {surrogateLearner->addTrainingSample(context, makeTrainingSample(context, object, fitness), surrogateModel);}

protected:
  friend class IncrementalSurrogateBasedSolverClass;

  IncrementalLearnerPtr surrogateLearner;
  ExpressionPtr surrogateModel;
};

class BatchSurrogateBasedSolver : public SurrogateBasedSolver
{
public:
  BatchSurrogateBasedSolver(SamplerPtr initialVectorSampler, SolverPtr surrogateLearner, SolverPtr surrogateSolver,
                       VariableEncoderPtr variableEncoder, SelectionCriterionPtr selectionCriterion, size_t numIterations)
    : SurrogateBasedSolver(initialVectorSampler, surrogateSolver, variableEncoder, selectionCriterion, numIterations), surrogateLearner(surrogateLearner)
  {
  }
  BatchSurrogateBasedSolver() {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    SurrogateBasedSolver::startSolver(context, problem, callback, startingSolution);
    std::pair<ProblemPtr, TablePtr> p = createSurrogateLearningProblem(context, problem);
    surrogateLearningProblem = p.first;
    surrogateData = p.second;
  }

  // SurrogateBasedSolver
  virtual ExpressionPtr getSurrogateModel(ExecutionContext& context)
  {
    ExpressionPtr res;
    surrogateLearner->solve(context, surrogateLearningProblem, storeBestSolutionSolverCallback(*(ObjectPtr* )&res));
    return res;
  }

  virtual void addFitnessSample(ExecutionContext& context, ObjectPtr object, FitnessPtr fitness)
  {
    std::vector<ObjectPtr> row = makeTrainingSample(context, object, fitness);
    surrogateLearningProblem->getObjective(0).staticCast<LearningObjective>()->getIndices()->append(surrogateData->getNumRows());
    surrogateData->addRow(row);
  }

protected:
  friend class BatchSurrogateBasedSolverClass;

  SolverPtr surrogateLearner;

  ProblemPtr surrogateLearningProblem;
  TablePtr surrogateData;

  ExpressionDomainPtr createSurrogateDomain(ExecutionContext& context, ProblemPtr problem)
  {
    ExpressionDomainPtr res = new ExpressionDomain();
    
    DomainPtr domain = problem->getDomain();
    variableEncoder->createEncodingVariables(context, domain, res);
    res->createSupervision(fitnessClass, "y");
    return res;
  }
  
  std::pair<ProblemPtr, TablePtr> createSurrogateLearningProblem(ExecutionContext& context, ProblemPtr problem)
  {
    ExpressionDomainPtr surrogateDomain = createSurrogateDomain(context, problem);
    TablePtr data = surrogateDomain->createTable(0);
    ProblemPtr res = new Problem();
    res->setDomain(surrogateDomain);
    if (problem->getNumObjectives() == 1)
      res->addObjective(mseRegressionObjective(data, surrogateDomain->getSupervision()));
    else
      res->addObjective(mseMultiRegressionObjective(data, surrogateDomain->getSupervision()));
    return std::make_pair(res, data);
  }
};


}; /* namespace lbcpp */

#endif // !ML_SOLVER_SURROGATE_BASED_H_
