/*-----------------------------------------.---------------------------------.
| Filename: AccumulatorLocalMeanFunction.h | Local Mean                      |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2011 14:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATOR_LOCAL_MEAN_FUNCTION_H_
# define LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATOR_LOCAL_MEAN_FUNCTION_H_

# include <lbcpp/Function/Function.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

  // Accumulator<T>, PositiveInteger => DenseDoubleVector<T>
class AccumulatorLocalMeanFunction : public Function
{
public:
  AccumulatorLocalMeanFunction(size_t windowSize = 0)
    : windowSize(windowSize) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)containerClass(doubleVectorClass());}

  virtual String getOutputPostFix() const
    {return T("LocalMean");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return Container::getTemplateParameter(inputVariables[0]->getType());}
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectVectorPtr& accumulator = inputs[0].getObjectAndCast<ObjectVector>();
    size_t n = accumulator->getNumElements();
    jassert(windowSize);

    int position = inputs[1].getInteger();
    jassert(position >= 0 && position < (int)n);
    int startPosition = -1 - (int)(windowSize / 2);
    int endPosition = startPosition + windowSize;
    int numMissingBegin = 0, numMissingEnd = 0;

    DenseDoubleVectorPtr begin, end;
    if (startPosition > 0)
      begin = accumulator->get(startPosition);
    else
      numMissingBegin = -startPosition;
    if (endPosition < (int)n)
      end = accumulator->get(endPosition);
    else
    {
      end = accumulator->get(n - 1);
      numMissingEnd = endPosition - (int)n + 1;
    }

    double Z = 1.0 / (double)windowSize;
    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType());
    end->addWeightedTo(res, 0, Z);
    if (begin)
      begin->addWeightedTo(res, 0, -Z);

    // the last element is supposed to be the "missing" frequency
    res->incrementValue(res->getNumElements() - 1, Z * (numMissingEnd - numMissingBegin));
    return res;
  }

protected:
  friend class AccumulatorLocalMeanFunctionClass;

  size_t windowSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATOR_LOCAL_MEAN_FUNCTION_H_
