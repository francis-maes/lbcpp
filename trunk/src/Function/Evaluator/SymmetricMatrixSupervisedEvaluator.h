/*-----------------------------------------.---------------------------------.
| Filename: SymmetricMatrixSupervisedEv...h| Symmetric Matrix Supervised     |
| Author  : Julien Becker                  |             Evaluator           |
| Started : 25/03/2011 13:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SUPERVISED_EVALUATOR_SYMMETRIC_MATRIX_H_
# define LBCPP_FUNCTION_SUPERVISED_EVALUATOR_SYMMETRIC_MATRIX_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Data/SymmetricMatrix.h>

namespace lbcpp
{

class SymmetricMatrixSupervisedEvaluator : public SupervisedEvaluator
{
public:
  SymmetricMatrixSupervisedEvaluator(SupervisedEvaluatorPtr elementEvaluator, size_t minimumDistanceFromDiagonal)
    : elementEvaluator(elementEvaluator), minimumDistanceFromDiagonal(minimumDistanceFromDiagonal) {}
  SymmetricMatrixSupervisedEvaluator() {}

  virtual TypePtr getRequiredPredictionType() const
    {return elementEvaluator->getRequiredPredictionType();}

  virtual TypePtr getRequiredSupervisionType() const
    {return elementEvaluator->getRequiredSupervisionType();}

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return elementEvaluator->createEmptyScoreObject(context, function);}

  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
  {
    SymmetricMatrixPtr supervisionContainer = example->getVariable(1).dynamicCast<SymmetricMatrix>();
    SymmetricMatrixPtr predictedContainer = output.dynamicCast<SymmetricMatrix>();

    const size_t dimension = predictedContainer->getDimension();
    jassert(supervisionContainer->getDimension() == dimension);

    if (minimumDistanceFromDiagonal > dimension)
      return true;

    const size_t numRows = dimension - minimumDistanceFromDiagonal;
    for (size_t i = 0; i < numRows; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
      {
        Variable supervision = supervisionContainer->getElement(i, j);
        Variable predicted = predictedContainer->getElement(i, j);
        if (!supervision.exists())
          continue;
        if (!predicted.exists())
        {
          context.errorCallback(T("ContainerSupervisedEvaluator::updateScoreObject"), T("Missing prediction"));
          return false;
        }
        //jassertfalse;
        addPrediction(context, predicted, supervision, scores);
      }
    return true;
  }

  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
    {elementEvaluator->addPrediction(context, prediction, supervision, result);}

  virtual void finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const
    {elementEvaluator->finalizeScoreObject(scores, function);}

protected:
  friend class SymmetricMatrixSupervisedEvaluatorClass;

  SupervisedEvaluatorPtr elementEvaluator;
  size_t minimumDistanceFromDiagonal;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SUPERVISED_EVALUATOR_SYMMETRIC_MATRIX_H_
