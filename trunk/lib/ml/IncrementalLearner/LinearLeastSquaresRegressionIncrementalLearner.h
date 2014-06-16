/*-------------------------------------------------------------.-----------------------------------------.
 | Filename: LinearLeastSquaresRegressionIncrementalLearner.h  | Incremental Linear Least Squares        |
 | Author  : Denny Verbeeck                                    |                                         |
 | Started : 12/06/2014 12:41                                  |                                         |
 `-------------------------------------------------------------/                                         |
                                  |                                                                      |
                                  `---------------------------------------------------------------------*/
#ifndef ML_LLSQ_REGRESSION_INCREMENTAL_LEARNER_H_
# define ML_LLSQ_REGRESSION_INCREMENTAL_LEARNER_H_

# include <Array/Array2D.h>
# include <LinAlg/LinAlg.h>
# include <ml/IncrementalLearner.h>
# include <ml/Expression.h>

namespace lbcpp
{

class LLSQStatistics : public Object
{
public:
  LLSQStatistics() : xtx(Array<double>(1,1)), xty(Array<double>(1,1)), examplesSeen(0) {}

  void addTrainingSample(const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output)
  {
    examplesSeen++;
    size_t numAttr = input->getNumValues() + 1;
    DenseDoubleVectorPtr extendedInput = new DenseDoubleVector(numAttr, 1.0);
    for (size_t i = 1; i < numAttr; ++i)
      extendedInput->setValue(i, input->getValue(i - 1));
    
    // initialise xtx and xty matrices
    if (xtx.dim(0) != numAttr)
    {
      xtx.resize(numAttr, numAttr);
      xty.resize(numAttr, output->getNumValues());
      for (size_t i = 0; i < numAttr; ++i)
      {
        for (size_t j = 0; j < numAttr; ++j)
          xtx(i,j) = 0.0;
        for (size_t j = 0; j < output->getNumValues(); ++j)
          xty(i,j) = 0.0;
      }
    }
    
    for (size_t i = 0; i < numAttr; ++i)
    {
      xtx(i, i) += extendedInput->getValue(i) * extendedInput->getValue(i);
      for (size_t j = i + 1; j < numAttr; ++j)
      {
        double val = extendedInput->getValue(i) * extendedInput->getValue(j);
        xtx(i, j) += val;
        xtx(j, i) += val;
      }
      for (size_t j = 0; j < output->getNumValues(); ++j)
        xty(i, j) += extendedInput->getValue(i) * output->getValue(j);
    }
  }

  size_t getExamplesSeen() const
    {return examplesSeen;}

  const Array<double>& getXTX() const
    {return xtx;}

  const Array<double>& getXTY() const
    {return xty;}

protected:
  Array<double> xtx;
  Array<double> xty;
  size_t examplesSeen;
};

typedef ReferenceCountedObjectPtr<LLSQStatistics> LLSQStatisticsPtr;

class LinearLeastSquaresRegressionIncrementalLearner : public IncrementalLearner
{
public:
  LinearLeastSquaresRegressionIncrementalLearner() {}

  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const
  {
    LinearModelExpressionPtr model = new LinearModelExpression();
    model->setLearnerStatistics(new LLSQStatistics());
    return model;
  }
  
  virtual void addTrainingSample(ExecutionContext& context, ExpressionPtr expression, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const
  {
    LLSQStatisticsPtr stats = expression->getLearnerStatistics().staticCast<LLSQStatistics>();
    stats->addTrainingSample(input, output);
    if (stats->getExamplesSeen() < input->getNumValues() + 1)
      return;

    const Array<double>& xtx = stats->getXTX();
    const Array<double>& xty = stats->getXTY();
    
    Array<double> xtxinv(xtx.dim(0), xtx.dim(1));
    xtxinv = invert(xtx);
    Array<double> b;
    matMat(b, xtxinv, xty);

    LinearModelExpressionPtr model = expression.staticCast<LinearModelExpression>();
    for (size_t i = 0; i < b.dim(0); ++i)
      model->getWeights()->setValue(i, b(i));
  }

};

} /* namespace lbcpp */

#endif // !ML_LLSQ_REGRESSION_INCREMENTAL_LEARNER_H_
