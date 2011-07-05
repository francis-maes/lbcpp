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
  DisulfidePatternEvaluator(FunctionPtr decoratedFunction = FunctionPtr(), double threshold = 0.5, size_t minimumDistanceFromDiagonal = 1)
    : decoratedFunction(decoratedFunction), threshold(threshold), minimumDistanceFromDiagonal(minimumDistanceFromDiagonal) {}

  /* SupervisedEvaluator */
  virtual TypePtr getRequiredPredictionType() const
    {return symmetricMatrixClass(doubleType);}

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
        if (predictedMatrix->getElement(i, j).getDouble() > threshold != supervisedMatrix->getElement(i, j).getDouble() > 0.5)
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

  FunctionPtr decoratedFunction;
  double threshold;
  size_t minimumDistanceFromDiagonal;
};

class DoNotApplyOnDimensionGreaterThan : public SupervisedEvaluator
{
public:
  DoNotApplyOnDimensionGreaterThan(SupervisedEvaluatorPtr decorated, size_t maxCysteins)
    : decorated(decorated), maxCysteins(maxCysteins) {}
  
  /* SupervisedEvaluator */
  virtual TypePtr getRequiredPredictionType() const
    {return decorated->getRequiredPredictionType();}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return decorated->getRequiredSupervisionType();}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
  {
    SymmetricMatrixPtr matrix = supervision.getObjectAndCast<SymmetricMatrix>(context);
    if (!matrix || matrix->getDimension() > maxCysteins)
      return;
    decorated->addPrediction(context, prediction, supervision, result);
  }

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return decorated->createEmptyScoreObject(context, function);}
  
  virtual void finalizeScoreObject(const ScoreObjectPtr& score, const FunctionPtr& function) const
    {decorated->finalizeScoreObject(score, function);}
  
protected:
  SupervisedEvaluatorPtr decorated;
  size_t maxCysteins;
};

class ExhaustiveDisulfidePatternBuilder : public SimpleUnaryFunction
{
public:
  ExhaustiveDisulfidePatternBuilder(double threshold = 0.5, TypePtr elementsType = probabilityType, size_t minimumDistanceFromDiagonal = 1)
  : SimpleUnaryFunction(symmetricMatrixClass(elementsType), symmetricMatrixClass(elementsType), T("ExhaustivePattern"))
  , threshold(threshold), minimumDistanceFromDiagonal(minimumDistanceFromDiagonal) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    SymmetricMatrixPtr matrix = input.getObjectAndCast<SymmetricMatrix>(context);
    if (!matrix)
      return Variable::missingValue(getOutputType());
    
    const size_t dimension = matrix->getDimension();
    if (dimension <= minimumDistanceFromDiagonal)
      return matrix;
    
    //std::cout << "Starting matrix:" << std::endl << matrix->toString() << std::endl;
    SymmetricMatrixPtr mask;
    double score = findBestMatrixScore(context, matrix, mask);
    //std::cout << "Score: " << score << std::endl;
    SymmetricMatrixPtr res = makeResult(context, matrix, mask);
    //std::cout << "Pattern: " << std::endl << mask->toString() << std::endl;
    return res;
  }
  
protected:
  double threshold;
  size_t minimumDistanceFromDiagonal;
  enum {notBridged = -1, bridged = -2};
  
  double findBestMatrixScore(ExecutionContext& context, const SymmetricMatrixPtr& matrix, SymmetricMatrixPtr& result) const
  {
    const size_t dimension = matrix->getDimension();

    double bestScore = -DBL_MAX;
    SymmetricMatrixPtr bestMatrix;
    
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
    {
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
      {
        const double value = matrix->getElement(i, j).getDouble();
        if (value == notBridged || value == bridged)
          continue;
        SymmetricMatrixPtr clone = matrix->cloneAndCast<SymmetricMatrix>(context);
        updateMatrix(clone, i, j, value > threshold);
        SymmetricMatrixPtr resultMatrix;
        double score = ((value > threshold) ? value : 0.0) + findBestMatrixScore(context, clone, resultMatrix);

        if (score > bestScore)
        {
          bestScore = score;
          bestMatrix = resultMatrix;
        }
      }
    }
    
    if (!bestMatrix)
    {
      result = matrix;
      return 0.0;
    }
    
    result = bestMatrix;
    return bestScore;
  }
                                         
  void updateMatrix(const SymmetricMatrixPtr& matrix, size_t x, size_t y, bool isConnected) const
  {
    const size_t dimension = matrix->getDimension();
    for (size_t i = 0; i < dimension; ++i)
    {
      matrix->setElement(x, i, probability((double)notBridged));
      matrix->setElement(y, i, probability((double)notBridged));
    }
    if (isConnected)
      matrix->setElement(x, y, probability((double)bridged));
  }
  
  SymmetricMatrixPtr makeResult(ExecutionContext& context, const SymmetricMatrixPtr& matrix, const SymmetricMatrixPtr& mask) const
  {
    SymmetricMatrixPtr res = matrix->cloneAndCast<SymmetricMatrix>(context);
    
    const size_t dimension = matrix->getDimension();
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
        if (mask->getElement(i, j).getDouble() == (double)notBridged)
          res->setElement(i, j, probability(0.f));
    
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_DISULFIDE_BOND_H_
