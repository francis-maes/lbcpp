/*-----------------------------------------.---------------------------------.
| Filename: DoubleMatrixComponent.h        | DoubleMatrix Component          |
| Author  : Francis Maes                   |                                 |
| Started : 16/05/2011 11:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_DOUBLE_MATRIX_H_
# define LBCPP_USER_INTERFACE_COMPONENT_DOUBLE_MATRIX_H_

# include <lbcpp/UserInterface/MatrixComponent.h>

namespace lbcpp
{

class DoubleMatrixComponent : public MatrixComponent
{
public:
   DoubleMatrixComponent(const DoubleMatrixPtr& matrix, const String& name)
     : MatrixComponent(matrix)
    {matrix->getExtremumValues(minValue, maxValue);}

  virtual juce::Colour selectColour(const Variable& element)
  {
    if (minValue == maxValue)
      return Colours::grey;
    double k = (element.getDouble()  - minValue) / (maxValue - minValue);
    return juce::Colour((float)k, 1.f, 0.75f, (juce::uint8)255);
  }

  virtual bool doPaintShortString(const Variable& element, int width, int height)
    {return width > 5;}

protected:
  double minValue, maxValue;
};


}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_DOUBLE_MATRIX_H_
