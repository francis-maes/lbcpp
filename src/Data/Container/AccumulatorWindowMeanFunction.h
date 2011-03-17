/*-----------------------------------------.---------------------------------.
| Filename: AccumulatorWindowMeanFunction.h| Window Mean                     |
| Author  : Becker Julien                  |                                 |
| Started : 17/02/2011 14:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATOR_WINDOW_MEAN_FUNCTION_H_
# define LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATOR_WINDOW_MEAN_FUNCTION_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

// Accumulator<T>, PositiveInteger, PositiveInteger => DenseDoubleVector<T>
class AccumulatorWindowMeanFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)containerClass(doubleVectorClass());}

  virtual String getOutputPostFix() const
    {return T("WindowMean");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return Container::getTemplateParameter(inputVariables[0]->getType());}
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const ContainerPtr& accumulator = inputs[0].getObjectAndCast<Container>();
    size_t n = accumulator->getNumElements();

    int startPosition = inputs[1].getInteger();
    int endPosition = inputs[2].getInteger();
    jassert(startPosition >= 0 && startPosition < (int)n);
    jassert(endPosition >= 0 && endPosition < (int)n);
    jassert(startPosition <= endPosition);
    int numMissingBegin = 0, numMissingEnd = 0;

    DenseDoubleVectorPtr begin, end;
    if (startPosition > 0)
      begin = accumulator->getElement(startPosition).getObjectAndCast<DenseDoubleVector>();
    else
      numMissingBegin = -startPosition;
    if (endPosition < (int)n)
      end = accumulator->getElement(endPosition).getObjectAndCast<DenseDoubleVector>();
    else
    {
      end = accumulator->getElement(n - 1).getObjectAndCast<DenseDoubleVector>();
      numMissingEnd = endPosition - (int)n + 1;
    }

    double Z = 1.0 / (double)(endPosition - startPosition);
    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType());
    end->addWeightedTo(res, 0, Z);
    if (begin)
      begin->addWeightedTo(res, 0, -Z);

    // the last element is supposed to be the "missing" frequency
    res->incrementValue(res->getNumElements() - 1, Z * (numMissingEnd + numMissingBegin));
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATOR_WINDOW_MEAN_FUNCTION_H_
