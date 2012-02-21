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

  virtual void addPrediction(bool isCorrect) // FIXME: remove virtual (used by ExporteDisulfidePatternScoreObject)
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
    {return matrixClass(doubleType);}

  virtual TypePtr getRequiredSupervisionType() const
    {return matrixClass(probabilityType);}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
  {
    MatrixPtr predictedMatrix = prediction.getObjectAndCast<Matrix>();
    MatrixPtr supervisedMatrix = supervision.getObjectAndCast<Matrix>();
    jassert(predictedMatrix->getNumRows() == predictedMatrix->getNumColumns()
            && supervisedMatrix->getNumRows() == supervisedMatrix->getNumColumns()
            && supervisedMatrix->getNumRows() == predictedMatrix->getNumRows());

    DisulfidePatternScoreObjectPtr score = result.staticCast<DisulfidePatternScoreObject>();
    const size_t dimension = supervisedMatrix->getNumRows();

    if (minimumDistanceFromDiagonal >= dimension)
    {
      score->addValidGraphPrediction(true);
      score->addPrediction(true);
      return;
    }
    
    if (decoratedFunction)
      predictedMatrix = decoratedFunction->compute(context, predictedMatrix).getObjectAndCast<Matrix>(context);

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

class DecoratorSupervisedEvaluator : public SupervisedEvaluator
{
public:
  DecoratorSupervisedEvaluator(SupervisedEvaluatorPtr decorated)
    : decorated(decorated) {}
  DecoratorSupervisedEvaluator() {}

  /* SupervisedEvaluator */
  virtual TypePtr getRequiredPredictionType() const
    {return decorated->getRequiredPredictionType();}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return decorated->getRequiredSupervisionType();}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
    {decorated->addPrediction(context, prediction, supervision, result);}
  
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return decorated->createEmptyScoreObject(context, function);}
  
  virtual void finalizeScoreObject(const ScoreObjectPtr& score, const FunctionPtr& function) const
    {decorated->finalizeScoreObject(score, function);}

protected:
  friend class DecoratorSupervisedEvaluatorClass;

  SupervisedEvaluatorPtr decorated;
};

class MatrixToSymmetricMatrix : public DecoratorSupervisedEvaluator
{
public:
  MatrixToSymmetricMatrix(SupervisedEvaluatorPtr decorated)
    : DecoratorSupervisedEvaluator(decorated) {}

  virtual TypePtr getRequiredPredictionType() const
    {return matrixClass(doubleType);}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return matrixClass(doubleType);}

  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
  {
    MatrixPtr predictedMatrix = prediction.getObjectAndCast<Matrix>();
    MatrixPtr supervisedMatrix = supervision.getObjectAndCast<Matrix>();
    jassert(predictedMatrix && supervisedMatrix);
    jassert(predictedMatrix->getNumRows() == predictedMatrix->getNumColumns()
            && supervisedMatrix->getNumRows() == supervisedMatrix->getNumColumns()
            && supervisedMatrix->getNumRows() == predictedMatrix->getNumRows());
    const size_t dimension = predictedMatrix->getNumRows();
    SymmetricMatrixPtr predSymMatrix = symmetricMatrix(predictedMatrix->getElementsType(), dimension);
    SymmetricMatrixPtr supSymMatrix = symmetricMatrix(supervisedMatrix->getElementsType(), dimension);

    for (size_t i = 0; i < dimension; ++i)
      for (size_t j = i; j < dimension; ++j)
      {
        if (i == j)
        {
          predSymMatrix->setElement(i, j, Variable::missingValue(predictedMatrix->getElementsType()));
          supSymMatrix->setElement(i, j, Variable::missingValue(supervisedMatrix->getElementsType()));
          continue;
        }
        Variable a = predictedMatrix->getElement(i, j);
        Variable b = predictedMatrix->getElement(j, i);
        predSymMatrix->setElement(i, j, (a < b) ? a : b);

        a = supervisedMatrix->getElement(i, j);
        b = supervisedMatrix->getElement(j, i);
        supSymMatrix->setElement(i, j, (a > b) ? a : b);
      }
    
    decorated->addPrediction(context, predSymMatrix, supSymMatrix, result);
  }
};

class DoNotApplyOnDimensionGreaterThan : public DecoratorSupervisedEvaluator
{
public:
  DoNotApplyOnDimensionGreaterThan(SupervisedEvaluatorPtr decorated, size_t maxCysteins)
    : DecoratorSupervisedEvaluator(decorated), maxCysteins(maxCysteins) {}
  
  /* SupervisedEvaluator */
  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
  {
    SymmetricMatrixPtr matrix = supervision.getObjectAndCast<SymmetricMatrix>(context);
    if (!matrix || matrix->getDimension() > maxCysteins)
      return;
    decorated->addPrediction(context, prediction, supervision, result);
  }

protected:
  size_t maxCysteins;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_DISULFIDE_BOND_H_
