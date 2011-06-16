/*-----------------------------------------.---------------------------------.
| Filename: DisulfideBondEvaluator.h       | The evaluator disulfide bonds   |
| Author  : Julien Becker                  |                                 |
| Started : 29/03/2011 16:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_EVALUATOR_DISULFIDE_BOND_H_
# define LBCPP_PROTEIN_EVALUATOR_DISULFIDE_BOND_H_

# include <lbcpp/Function/Evaluator.h>
# include "../Data/Protein.h"
    
namespace lbcpp
{

class DisulfidePatternScoreObject : public ScoreObject
{
public:
  DisulfidePatternScoreObject()
    : accuracy(0.0), validGraphAccuracy(0.0),
      accuracyVector(new ScalarVariableMean()), validGraphAccuracyVector(new ScalarVariableMean()) {}

  virtual double getScoreToMinimize() const
    {return 1.0 - accuracy;}

  void addPrediction(bool isCorrect)
    {accuracyVector->push(isCorrect);}
  
  void addValidGraphPrediction(bool isValid)
    {validGraphAccuracyVector->push(isValid);}
  
  void finalize()
  {
    accuracy = accuracyVector->getMean();
    validGraphAccuracy = validGraphAccuracyVector->getMean();
  }

protected:
  friend class DisulfidePatternScoreObjectClass;

  double accuracy;
  double validGraphAccuracy;
  ScalarVariableMeanPtr accuracyVector;
  ScalarVariableMeanPtr validGraphAccuracyVector;
};

typedef ReferenceCountedObjectPtr<DisulfidePatternScoreObject> DisulfidePatternScoreObjectPtr;

class DisulfidePatternEvaluator : public SupervisedEvaluator
{
public:
  DisulfidePatternEvaluator(bool isFinalEvaluation = false, FunctionPtr decoratedFunction = FunctionPtr(), double threshold = 0.5, size_t minimumDistanceFromDiagonal = 1)
    : isFinalEvaluation(isFinalEvaluation), decoratedFunction(decoratedFunction), threshold(threshold), minimumDistanceFromDiagonal(minimumDistanceFromDiagonal) {}

  /* SupervisedEvaluator */
  virtual TypePtr getRequiredPredictionType() const
    {return symmetricMatrixClass(probabilityType);}

  virtual TypePtr getRequiredSupervisionType() const
    {return symmetricMatrixClass(probabilityType);}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
  {
    if (!isFinalEvaluation)
      return;

    SymmetricMatrixPtr predictedMatrix = prediction.getObjectAndCast<SymmetricMatrix>();
    SymmetricMatrixPtr supervisedMatrix = supervision.getObjectAndCast<SymmetricMatrix>();
    DisulfidePatternScoreObjectPtr score = result.staticCast<DisulfidePatternScoreObject>();
    const size_t dimension = supervisedMatrix->getDimension();
    jassert(supervisedMatrix && predictedMatrix && predictedMatrix->getDimension() == dimension);

    if (minimumDistanceFromDiagonal >= dimension)
    {
      score->addValidGraphPrediction(true);
      score->addPrediction(true);
      return;
    }
    
    if (decoratedFunction)
      predictedMatrix = decoratedFunction->compute(context, predictedMatrix).getObjectAndCast<SymmetricMatrix>(context);

    const size_t numRows = dimension - minimumDistanceFromDiagonal;
    // check validity of graph
    bool isValidGraph = true;
    for (size_t i = 0; i < dimension; ++i)
    {
      size_t numBridges = 0;
      for (size_t j = 0; j < dimension; ++j)
      {
        if (i == j)
          continue;
        if (predictedMatrix->getElement(i, j).getDouble() > threshold)
          ++numBridges;
      }
      if (numBridges > 1)
      {
        isValidGraph = false;
        break;
      }
    }
    score->addValidGraphPrediction(isValidGraph);
    // check pattern
    for (size_t i = 0; i < numRows; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
        if (predictedMatrix->getElement(i, j).getDouble() > threshold != supervisedMatrix->getElement(i, j).getDouble() > threshold)
        {
          score->addPrediction(false);
          return;
        }
    score->addPrediction(true);
  }
  
  /* Evaluator */
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new DisulfidePatternScoreObject();}
  
  virtual void finalizeScoreObject(const ScoreObjectPtr& score, const FunctionPtr& function) const
    {score.staticCast<DisulfidePatternScoreObject>()->finalize();}

protected:
  friend class DisulfidePatternEvaluatorClass;

  bool isFinalEvaluation;
  FunctionPtr decoratedFunction;
  double threshold;
  size_t minimumDistanceFromDiagonal;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_DISULFIDE_BOND_H_
