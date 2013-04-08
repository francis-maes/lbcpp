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

namespace lbcpp
{

class SurrogateBasedSolverInformation : public Object
{
public:
  SurrogateBasedSolverInformation(size_t stepNumber = 0)
    : drawSolutions(true), layerToDraw(objectiveLayer), precision(0.2), colourScaleType(linear), stepNumber(stepNumber) {}

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

  bool getDrawSolutions() const
    {return drawSolutions;}

  void setDrawSolutions(bool draw)
    {drawSolutions = draw;}

  double getPrecision() const
    {return precision;}

  void setPrecision(double precision)
    {this->precision = precision;}

  enum Layer
  {
    objectiveLayer = 0,
    modelLayer,
    stddevLayer,
    expectedImprovementLayer,
    probabilityOfImprovementLayer,
    optimistic1Layer,
    optimistic2Layer,
    optimistic5Layer,

    numLayers
  };

  enum ColourScaleType
  {
    linear = 0,
    percentile
  };

  Layer getLayerToDraw() const
    {return layerToDraw;}

  void setLayerToDraw(Layer layerToDraw)
    {this->layerToDraw = layerToDraw;}

  ColourScaleType getColourScaleType() const
    {return colourScaleType;}

  void setColourScaleType(ColourScaleType newType)
    {colourScaleType = newType;}
  
  static string getLayerName(Layer layer)
  {
    static const char* names[] = {"Objective", "Model", "Uncertainty", "Expected Improvement", "Probability of Improvement of 10%", "Optimistic C=1", "Optimistic C=2", "Optimistic C=5"};
    jassert(layer < numLayers);
    return names[layer];
  }

private:
  friend class SurrogateBasedSolverInformationClass;

  bool drawSolutions;
  Layer layerToDraw;
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
