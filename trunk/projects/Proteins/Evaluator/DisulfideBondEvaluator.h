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
  DisulfidePatternEvaluator(double threshold = 0.5, size_t minimumDistanceFromDiagonal = 1)
    : threshold(threshold), minimumDistanceFromDiagonal(minimumDistanceFromDiagonal) {}

  /* SupervisedEvaluator */
  virtual TypePtr getRequiredPredictionType() const
    {return symmetricMatrixClass(probabilityType);}

  virtual TypePtr getRequiredSupervisionType() const
    {return symmetricMatrixClass(probabilityType);}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
  {
    SymmetricMatrixPtr predictedMatrix = prediction.getObjectAndCast<SymmetricMatrix>();
    SymmetricMatrixPtr supervisedMatrix = supervision.getObjectAndCast<SymmetricMatrix>();
    DisulfidePatternScoreObjectPtr score = result.staticCast<DisulfidePatternScoreObject>();
    const size_t dimension = supervisedMatrix->getDimension();
    jassert(supervisedMatrix && predictedMatrix && predictedMatrix->getDimension() == dimension);

    if (minimumDistanceFromDiagonal >= dimension)
    { // FIXME
      //score->addValidGraphPrediction(isValidGraph);
      //score->addPrediction(true);
      return;
    }
    
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

  double threshold;
  size_t minimumDistanceFromDiagonal;
};

class NaiveDisulfidePatternBuilderEvaluator : public DisulfidePatternEvaluator
{
public:
  NaiveDisulfidePatternBuilderEvaluator(double threshold = 0.5, size_t minimumDistanceFromDiagonal = 1)
    : DisulfidePatternEvaluator(threshold, minimumDistanceFromDiagonal) {}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
  {
    SymmetricMatrixPtr predictedMatrix = prediction.getObjectAndCast<SymmetricMatrix>();
    SymmetricMatrixPtr supervisedMatrix = supervision.getObjectAndCast<SymmetricMatrix>();
    const size_t dimension = supervisedMatrix->getDimension();
    jassert(supervisedMatrix && predictedMatrix && predictedMatrix->getDimension() == dimension);

    if (minimumDistanceFromDiagonal >= dimension)
    {
      DisulfidePatternEvaluator::addPrediction(context, prediction, supervision, result);
      return;
    }

    SymmetricMatrixPtr res = predictedMatrix->cloneAndCast<SymmetricMatrix>(context);
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
    {
      // Find best predicted element of row i
      size_t bestIndex = (size_t)-1;
      double bestValue = -DBL_MAX;
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
      {
        if (res->getElement(i, j).getDouble() > bestValue)
        {
          bestIndex = j;
          bestValue = res->getElement(i, j).getDouble();
        }
      }
      jassert(bestIndex != (size_t)-1);
      // Update predictd matrix with a connection between i and bestIndex if bestValue > threshold
      if (bestValue != 0.0)
        updateMatrix(res, i, bestIndex, bestValue > threshold);
    }

    DisulfidePatternEvaluator::addPrediction(context, res, supervision, result);
  }

private:
  void updateMatrix(const SymmetricMatrixPtr& matrix, size_t x, size_t y, bool isConnected) const
  {
    const size_t dimension = matrix->getDimension();
    for (size_t i = 0; i < dimension; ++i)
    {
      matrix->setElement(x, i, probability(0.0));
      matrix->setElement(y, i, probability(0.0));
    }
    if (isConnected)
      matrix->setElement(x, y, probability(1.0));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_DISULFIDE_BOND_H_
