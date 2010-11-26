/*-----------------------------------------.---------------------------------.
| Filename: ProductPerception.h            | Product of two Perceptions      |
| Author  : Francis Maes                   |                                 |
| Started : 04/10/2010 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_PRODUCT_H_
# define LBCPP_FUNCTION_PERCEPTION_PRODUCT_H_

# include <lbcpp/Perception/Perception.h>
# include <list>

namespace lbcpp
{

class ProductPerception;
typedef ReferenceCountedObjectPtr<ProductPerception> ProductPerceptionPtr;

class ProductPerception : public Perception
{
public:
  ProductPerception(FunctionPtr multiplyFunction, PerceptionPtr perception1, PerceptionPtr perception2, bool symmetricFunction, bool singleInputForBothPerceptions);
  ProductPerception() {}

  virtual TypePtr getInputType() const;

  virtual String toString() const
    {return perception1->toString() + T(" x ") + perception2->toString();}

  virtual bool isSparse() const
    {return perception1->isSparse() || perception2->isSparse();}

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const;

  PerceptionPtr getPerception1() const
    {return perception1;}

  PerceptionPtr getPerception2() const
    {return perception2;}

  FunctionPtr getMultiplyFunction() const
    {return multiplyFunction;}

protected:
  friend class ProductPerceptionClass;

  FunctionPtr multiplyFunction;
  bool symmetricFunction;
  PerceptionPtr perception1;
  PerceptionPtr perception2;
  bool singleInputForBothPerceptions;

  virtual void computeOutputType();
  void addOutputVariable(const String& name, TypePtr type1, PerceptionPtr sub1, TypePtr type2, PerceptionPtr sub2);
};

class ProductWithVariablePerception : public Perception
{
public:
  ProductWithVariablePerception(FunctionPtr multiplyFunction, PerceptionPtr perception, TypePtr variableType, bool swapVariables);
  ProductWithVariablePerception() {}

  virtual TypePtr getInputType() const;
  virtual String toString() const;
  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const;

  FunctionPtr getMultiplyFunction() const
    {return multiplyFunction;}

  PerceptionPtr getPerception() const
    {return perception;}

  TypePtr getVariableType() const
    {return variableType;}

  bool areVariablesSwapped() const
    {return swapVariables;}

protected:
  virtual void computeOutputType();

  friend class ProductWithVariablePerceptionClass;

  FunctionPtr multiplyFunction;
  PerceptionPtr perception;
  TypePtr variableType;
  bool swapVariables;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_PRODUCT_H_
