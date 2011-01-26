/*-----------------------------------------.---------------------------------.
| Filename: InputLabelPairPerception.h     | Pair(Input,Label) Perception    |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2010 18:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_INPUT_LABEL_PAIR_H_
# define LBCPP_FUNCTION_PERCEPTION_INPUT_LABEL_PAIR_H_

# include <lbcpp/Perception/Perception.h>
# include <lbcpp/Core/Pair.h>

namespace lbcpp
{

// duplicate inputPerception for each possible classes
class InputLabelPairPerception : public Perception
{
public:
  InputLabelPairPerception(PerceptionPtr inputPerception, EnumerationPtr classes)
    : inputPerception(inputPerception), classes(classes) {}
  InputLabelPairPerception() {}

  virtual bool isSparse() const
    {return true;}

  virtual TypePtr getInputType() const
    {return pairClass(inputPerception->getInputType(), classes);}

  virtual void computeOutputType()
  {
    size_t n = classes->getNumElements();
    reserveOutputVariables(n);
    for (size_t i = 0; i < n; ++i)
      addOutputVariable(classes->getElementName(i), inputPerception);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>(context);
    if (pair->getSecond().exists())
      callback->sense((size_t)pair->getSecond().getInteger(), inputPerception, pair->getFirst());
  }

protected:
  friend class InputLabelPairPerceptionClass;

  PerceptionPtr inputPerception;
  EnumerationPtr classes;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_INPUT_LABEL_PAIR_H_
