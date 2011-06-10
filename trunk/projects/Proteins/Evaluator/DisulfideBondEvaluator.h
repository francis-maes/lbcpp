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
    {
      score->addValidGraphPrediction(true);
      score->addPrediction(true);
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

class DisulfidePatternBuilderEvaluator : public DisulfidePatternEvaluator
{
public:
  DisulfidePatternBuilderEvaluator(double threshold = 0.5, size_t minimumDistanceFromDiagonal = 1)
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

    buildPattern(predictedMatrix, res);
    
    DisulfidePatternEvaluator::addPrediction(context, res, supervision, result);
  }
  
protected:
  virtual void buildPattern(const SymmetricMatrixPtr& predictedMatrix, const SymmetricMatrixPtr& resultMatrix) const = 0; 
  
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

class NaiveDisulfidePatternBuilderEvaluator : public DisulfidePatternBuilderEvaluator
{
public:
  NaiveDisulfidePatternBuilderEvaluator(double threshold = 0.5, size_t minimumDistanceFromDiagonal = 1)
    : DisulfidePatternBuilderEvaluator(threshold, minimumDistanceFromDiagonal) {}

protected:
  virtual void buildPattern(const SymmetricMatrixPtr& predictedMatrix, const SymmetricMatrixPtr& resultMatrix) const 
  {
    const size_t dimension = predictedMatrix->getDimension();
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
    {
      // Find best predicted element of row i
      size_t bestIndex = (size_t)-1;
      double bestValue = -DBL_MAX;
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
      {
        if (resultMatrix->getElement(i, j).getDouble() > bestValue)
        {
          bestIndex = j;
          bestValue = resultMatrix->getElement(i, j).getDouble();
        }
      }
      jassert(bestIndex != (size_t)-1);
      // Update predictd matrix with a connection between i and bestIndex if bestValue > threshold
      if (bestValue != 0.0)
        updateMatrix(resultMatrix, i, bestIndex, bestValue > threshold);
    }
  }
};

class BestFirstDisulfidePatternBuilderEvaluator : public DisulfidePatternBuilderEvaluator
{
public:
  BestFirstDisulfidePatternBuilderEvaluator(double threshold = 0.5, size_t minimumDistanceFromDiagonal = 1)
    : DisulfidePatternBuilderEvaluator(threshold, minimumDistanceFromDiagonal) {}

protected:  
  virtual void buildPattern(const SymmetricMatrixPtr& predictedMatrix, const SymmetricMatrixPtr& resultMatrix) const 
  {
    while (true)
    {
      double value = -DBL_MAX;
      size_t i = (size_t)-1;
      size_t j = (size_t)-1;
      findBestValue(resultMatrix, value, i, j);

      if (value > 0.0)
        updateMatrix(resultMatrix, i, j, value > threshold);
      else
        return;
    }
  }

  void findBestValue(const SymmetricMatrixPtr& resultMatrix, double& bestValue, size_t& bestI, size_t& bestJ) const
  {
    const size_t dimension = resultMatrix->getDimension();
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
      {
        const double value = resultMatrix->getElement(i, j).getDouble();
        if (value < 1. && value > bestValue)
        {
          bestI = i;
          bestJ = j;
          bestValue = value;
        }
      }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_DISULFIDE_BOND_H_
