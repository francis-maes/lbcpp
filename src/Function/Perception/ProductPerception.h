/*-----------------------------------------.---------------------------------.
| Filename: ProductPerception.h            | Product of two Perceptions      |
| Author  : Francis Maes                   |                                 |
| Started : 04/10/2010 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_PRODUCT_H_
# define LBCPP_FUNCTION_PERCEPTION_PRODUCT_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{

class ProductPerception : public VariableVectorPerception
{
public:
  ProductPerception(PerceptionPtr perception1, PerceptionPtr perception2)
    : perception1(perception1), perception2(perception2)
  {
    jassert(perception1->getInputType() == perception2->getInputType());
    computeOutputVariables();
  }

  ProductPerception() {}

  virtual TypePtr getInputType() const
    {return perception1->getInputType();}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    // FIXME
  }

protected:
  PerceptionPtr perception1;
  PerceptionPtr perception2;

  void computeOutputVariables()
  {
    size_t n1 = perception1->getNumOutputVariables();
    size_t n2 = perception2->getNumOutputVariables();
    outputVariables.reserve(n1 * n2);
    for (size_t i = 0; i < n1; ++i)
      for (size_t j = 0; j < n2; ++j)
      {
        PerceptionPtr sub1 = perception1->getOutputVariableGenerator(i);
        TypePtr type1 = perception1->getOutputVariableType(i);
        String name1 = perception1->getOutputVariableName(i);

        PerceptionPtr sub2 = perception2->getOutputVariableGenerator(j);
        TypePtr type2 = perception2->getOutputVariableType(j);
        String name2 = perception2->getOutputVariableName(j);

        String name = name1 + T(" x ") + name2;
        if (!sub1 && !sub2)
        {
          TypePtr type = pairType(type1, type2);
          addOutputVariable(type, name, PerceptionPtr());
        }
        else if (sub1 && sub2)
        {
          PerceptionPtr subProduct = new ProductPerception(sub1, sub2);
          addOutputVariable(subProduct->getOutputType(), name, subProduct);
        }
        else
        {
        }
      }

  }

};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_PRODUCT_H_
