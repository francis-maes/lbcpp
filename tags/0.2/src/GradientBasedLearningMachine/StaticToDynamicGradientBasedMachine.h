/*-----------------------------------------.---------------------------------.
| Filename: StaticToDynamicGradientBase...h| Gradient based                  |
| Author  : Francis Maes                   |        learning machines        |
| Started : 09/06/2009 14:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNING_MACHINE_STATIC_TO_DYNAMIC_H_
# define LBCPP_GRADIENT_BASED_LEARNING_MACHINE_STATIC_TO_DYNAMIC_H_

# include <lbcpp/GradientBasedLearningMachine.h>
# include <lbcpp/impl/impl.h> // ContinuousFunctions

namespace lbcpp
{

template<class ExactType, class BaseClass>
class StaticToDynamicGradientBasedLearningMachine : public BaseClass
{
public:
  typedef typename BaseClass::ExampleType ExampleType;
  
  virtual String toString() const
  {
    String className = BaseClass::getClassName();
    if (className[0] >= 'A' && className[0] <= 'Z')
      className[0] += 'a' - 'A';
    return className + "(" + BaseClass::getLearner()->toString() + ", " + BaseClass::getRegularizer()->toString() + ")";
  }
  
  // abstract: static functions for architecture() and loss()
  virtual ObjectPtr clone() const
  {
    ReferenceCountedObjectPtr<ExactType> res = new ExactType();
    BaseClass::cloneImpl(*res);
    return res;
  }

  virtual ScalarVectorFunctionPtr getLoss(ObjectPtr example) const
    {return impl::staticToDynamic(impl::exampleRisk(_this().architecture(), _this().loss(), *example.staticCast<ExampleType>()));}
    
  virtual ScalarVectorFunctionPtr getEmpiricalRisk(ObjectContainerPtr examples) const
    {return impl::staticToDynamic(impl::empiricalRisk(_this().architecture(), _this().loss(), examples, (ExampleType* )0));}
    
  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(ObjectContainerPtr examples) const
  {
    if (BaseClass::regularizer)
      return impl::staticToDynamic(impl::add(impl::empiricalRisk(_this().architecture(), _this().loss(), examples, (ExampleType* )0),
          impl::dynamicToStatic(BaseClass::regularizer)));
    else
      return getEmpiricalRisk(examples);
  }

protected:
  const ExactType& _this() const  {return *(const ExactType* )this;}
};

}; /* namespace lbcpp*/

#endif // !LBCPP_GRADIENT_BASED_LEARNING_MACHINE_STATIC_TO_DYNAMIC_H_
