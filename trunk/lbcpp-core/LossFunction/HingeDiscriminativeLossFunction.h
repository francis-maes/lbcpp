/*-----------------------------------------.---------------------------------.
| Filename: HingeLossFunction.h            | Hinge Loss Function             |
| Author  : Francis Maes                   |                                 |
| Started : 03/05/2010 16:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_HINGE_H_
# define LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_HINGE_H_

# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

// f(x) = max(0, 1 - input)
class HingeDiscriminativeLossFunction : public DiscriminativeLossFunction
{
public:
  HingeDiscriminativeLossFunction(double margin = 1.0) : margin(margin) {}

  virtual bool isDerivable() const
    {return false;}

  virtual void computeDiscriminativeLoss(double score, double* output, double* derivative) const
  {
    if (score > margin)
    {
      if (output) *output = 0.0;
      if (derivative) *derivative = 0.0;
    }
    else if (score == margin)
    {
      if (output) *output = 0.0;
      if (derivative) *derivative = -1.0;
    }
    else
    {
      if (output) *output = margin - score;
      if (derivative) *derivative = -1.0;
    }
  }

  double getMargin() const
    {return margin;}

protected:
  double margin;
};

typedef ReferenceCountedObjectPtr<HingeDiscriminativeLossFunction> HingeDiscriminativeLossFunctionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_HINGE_H_
