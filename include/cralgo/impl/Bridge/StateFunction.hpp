/*-----------------------------------------.---------------------------------.
| Filename: StateFunction.hpp              | Static to Dynamic State         |
| Author  : Francis Maes                   |   functions                     |
| Started : 13/02/2009 17:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_STATIC_STATE_FUNCTION_HPP_
# define CRALGO_STATIC_STATE_FUNCTION_HPP_

# include "../../StateFunction.h"
# include "FeatureGenerator.hpp"

namespace cralgo
{


template<class CRAlgorithmType>
CRAlgorithmType& dynamicToStaticCRAlgorithm(CRAlgorithmPtr crAlgorithm);

template<class DynamicType>
struct StateFunctionTraits {};

/*
** String Description
*/
template<>
struct StateFunctionTraits<StateDescriptionFunction>
{
  template<class FunctionType, class CRAlgorithmType>
  struct StaticToDynamic : public StateDescriptionFunction
  {
    virtual std::string toString(CRAlgorithmPtr crAlgorithm) const
      {return FunctionType::function(dynamicToStaticCRAlgorithm<CRAlgorithmType>(crAlgorithm));}
  };

  template<class FunctionType, class CRAlgorithmType>
  static StateDescriptionFunction* create()
    {return new StaticToDynamic<FunctionType, CRAlgorithmType>();}
    
  typedef CompositeStateDescriptionFunction CompositeFunction; 

  static CompositeFunction* createComposite() {return new CompositeFunction();}
};

template<>
struct StateFunctionTraits<ActionDescriptionFunction>
{
  template<class FunctionType, class CRAlgorithmType>
  struct StaticToDynamic : public ActionDescriptionFunction
  {
    typedef typename FunctionType::__Param1Type__ ChoiceType;
    
    virtual std::string toString(CRAlgorithmPtr crAlgorithm, const void* choice) const
      {return FunctionType::function(dynamicToStaticCRAlgorithm<CRAlgorithmType>(crAlgorithm), *(const ChoiceType* )choice);}
    virtual std::string getName() const
      {return FunctionType::getName();}
  };

  template<class FunctionType, class CRAlgorithmType>
  static ActionDescriptionFunction* create()
    {return new StaticToDynamic<FunctionType, CRAlgorithmType>();}
    
  typedef CompositeActionDescriptionFunction CompositeFunction; 

  static CompositeFunction* createComposite() {return new CompositeFunction();}
};

/*
** Value functions
*/
template<>
struct StateFunctionTraits<StateValueFunction>
{
  template<class FunctionType, class CRAlgorithmType>
  struct StaticToDynamic : public StateValueFunction
  {
    virtual double compute(CRAlgorithmPtr crAlgorithm) const
      {return FunctionType::function(dynamicToStaticCRAlgorithm<CRAlgorithmType>(crAlgorithm));}
    virtual std::string getName() const
      {return FunctionType::getName();}
  };

  template<class FunctionType, class CRAlgorithmType>
  static StateValueFunction* create()
    {return new StaticToDynamic<FunctionType, CRAlgorithmType>();}
    
  typedef CompositeStateValueFunction CompositeFunction; 

  static CompositeFunction* createComposite() {return new CompositeFunction();}
};

template<>
struct StateFunctionTraits<ActionValueFunction>
{
  template<class FunctionType, class CRAlgorithmType>
  struct StaticToDynamic : public ActionValueFunction
  {
    typedef typename FunctionType::__Param1Type__ ChoiceType;

    virtual double compute(CRAlgorithmPtr crAlgorithm, const void* choice) const
      {return FunctionType::function(dynamicToStaticCRAlgorithm<CRAlgorithmType>(crAlgorithm), *(const ChoiceType* )choice);}
    virtual std::string getName() const
      {return FunctionType::getName();}
  };

  template<class FunctionType, class CRAlgorithmType>
  static ActionValueFunction* create()
    {return new StaticToDynamic<FunctionType, CRAlgorithmType>();}
    
  typedef CompositeActionValueFunction CompositeFunction; 

  static CompositeFunction* createComposite() {return new CompositeFunction();}
};


/*
** Feature generators
*/
template<>
struct StateFunctionTraits<StateFeaturesFunction>
{
  template<class FunctionType, class CRAlgorithmType>
  struct StaticToDynamic : public StateFeaturesFunction
  {
    virtual FeatureGeneratorPtr featureGenerator(CRAlgorithmPtr crAlgorithm) const
      {return FunctionType::function(dynamicToStaticCRAlgorithm<CRAlgorithmType>(crAlgorithm));}
    virtual std::string getName() const
      {return FunctionType::getName();}
  };

  template<class FunctionType, class CRAlgorithmType>
  static StateFeaturesFunction* create()
    {return new StaticToDynamic<FunctionType, CRAlgorithmType>();}
    
  typedef CompositeStateFeaturesFunction CompositeFunction; 
    
  static CompositeFunction* createComposite() {return new CompositeFunction();}
};

template<>
struct StateFunctionTraits<ActionFeaturesFunction>
{
  template<class FunctionType, class CRAlgorithmType>
  struct StaticToDynamic : public ActionFeaturesFunction
  {
    typedef typename FunctionType::__Param1Type__ ChoiceType;

    virtual FeatureGeneratorPtr featureGenerator(CRAlgorithmPtr crAlgorithm, const void* choice) const
      {return FunctionType::function(dynamicToStaticCRAlgorithm<CRAlgorithmType>(crAlgorithm), *(const ChoiceType* )choice);}
    virtual std::string getName() const
      {return FunctionType::getName();}
  };

  template<class FunctionType, class CRAlgorithmType>
  static ActionFeaturesFunction* create()
    {return new StaticToDynamic<FunctionType, CRAlgorithmType>();}
    
  typedef CompositeActionFeaturesFunction CompositeFunction; 

  static CompositeFunction* createComposite() {return new CompositeFunction();}
};


#if 0

////////////////////////////////////////////////////////////////////////////////

/*
** StateFunction
*/
template<class ExactType, class BaseClassType, class StaticType, class CRAlgorithmType>
class StaticToDynamicChooseArgument : public BaseClassType
{
public:      
  virtual std::string getName() const
    {return StaticType::getName();}
};

/*
** Any stateFunction
*/
template<class FunctionType, class CRAlgorithmType>
class StaticToDynamicStateFunction 
  : public StaticToDynamicChooseArgument<StaticToDynamicStateFunction<FunctionType, CRAlgorithmType>, 
            StateFunction, FunctionType, CRAlgorithmType>  {};

/*
** State Description
*/
template<class FunctionType, class CRAlgorithmType>
class StaticToDynamicStateDescription 
  : public StaticToDynamicChooseArgument<StaticToDynamicStateDescription< FunctionType, CRAlgorithmType>, 
            StateDescriptionFunction, FunctionType, CRAlgorithmType>
{
public:
  virtual std::string toString(CRAlgorithmPtr crAlgorithm) const
    {return FunctionType::function(dynamicToStaticCRAlgorithm<CRAlgorithmType>(crAlgorithm));}
};

/*
** Action Description
*/
template<class FunctionType, class CRAlgorithmType>
class StaticToDynamicActionDescription 
  : public StaticToDynamicChooseArgument<StaticToDynamicActionDescription<FunctionType, CRAlgorithmType>, 
            ActionDescriptionFunction, FunctionType, CRAlgorithmType>
{
public:
  typedef typename FunctionType::__Param1Type__ ChoiceType;
  
  virtual std::string toString(CRAlgorithmPtr crAlgorithm, const void* choice) const
    {return FunctionType::function(dynamicToStaticCRAlgorithm<CRAlgorithmType>(crAlgorithm),
      *(const ChoiceType* )choice);}
};

/*
** State Value
*/
template<class FunctionType, class CRAlgorithmType>
class StaticToDynamicStateValue 
  : public StaticToDynamicChooseArgument<StaticToDynamicStateValue<FunctionType, CRAlgorithmType>, 
            StateValueFunction, FunctionType, CRAlgorithmType>
{
public:
  virtual double compute(CRAlgorithmPtr crAlgorithm) const
    {return FunctionType::function(dynamicToStaticCRAlgorithm<CRAlgorithmType>(crAlgorithm));}
};

/*
** Action Value
*/
template<class FunctionType, class CRAlgorithmType>
class StaticToDynamicActionValue 
  : public StaticToDynamicChooseArgument<StaticToDynamicActionValue<FunctionType, CRAlgorithmType>, 
            ActionValueFunction, FunctionType, CRAlgorithmType>
{
public:
  typedef typename FunctionType::__Param1Type__ ChoiceType;
  
  virtual double compute(CRAlgorithmPtr crAlgorithm, const void* choice) const
    {return FunctionType::function(dynamicToStaticCRAlgorithm<CRAlgorithmType>(crAlgorithm),
      *(const ChoiceType* )choice);}
};

/*
** State Features
*/
template<class FunctionType, class CRAlgorithmType>
class StaticToDynamicStateFeatures 
  : public StaticToDynamicChooseArgument<StaticToDynamicStateFeatures<FunctionType, CRAlgorithmType>, 
            StateFeaturesFunction, FunctionType, CRAlgorithmType>
{
public:
  virtual FeatureGeneratorPtr featureGenerator(CRAlgorithmPtr crAlgorithm) const
    {return FunctionType::function(dynamicToStaticCRAlgorithm<CRAlgorithmType>(crAlgorithm));}
};

/*
** Action Features
*/
template<class FunctionType, class CRAlgorithmType>
class StaticToDynamicActionFeatures 
  : public StaticToDynamicChooseArgument<StaticToDynamicActionFeatures<FunctionType, CRAlgorithmType>, 
            ActionFeaturesFunction, FunctionType, CRAlgorithmType>
{
public:
  typedef typename FunctionType::__Param1Type__ ChoiceType;
  
  virtual FeatureGeneratorPtr featureGenerator(CRAlgorithmPtr crAlgorithm, const void* choice) const
    {return FunctionType::function(dynamicToStaticCRAlgorithm<CRAlgorithmType>(crAlgorithm), *(ChoiceType* )choice);}
};


/*
** Constructor
*/
template<class ReturnType, unsigned NumParameters>
struct StaticToDynamicStateFunctionConstructor
{
  template<class FunctionType, class CRAlgorithmType>
  static StateFunction* create(const FunctionType& dummy1, const CRAlgorithmType& dummy2)
    {return new StaticToDynamicStateFunction<FunctionType, CRAlgorithmType>();}
};

#define STATE_FUNCTION_STATIC_CONSTRUCTOR(returnType, numArguments, StaticToDynamicClass) \
  template<> \
  struct StaticToDynamicStateFunctionConstructor<returnType, numArguments> \
  { \
    template<class FunctionType, class CRAlgorithmType> \
    static StateFunction* create(const FunctionType& dummy1, const CRAlgorithmType& dummy2) \
      {return new StaticToDynamicClass <FunctionType, CRAlgorithmType>();} \
  }
  
STATE_FUNCTION_STATIC_CONSTRUCTOR(std::string, 1, StaticToDynamicStateDescription);
STATE_FUNCTION_STATIC_CONSTRUCTOR(std::string, 2, StaticToDynamicActionDescription);
STATE_FUNCTION_STATIC_CONSTRUCTOR(double, 1, StaticToDynamicStateValue);
STATE_FUNCTION_STATIC_CONSTRUCTOR(double, 2, StaticToDynamicActionValue);
STATE_FUNCTION_STATIC_CONSTRUCTOR(FeatureGeneratorPtr, 1, StaticToDynamicStateFeatures);
STATE_FUNCTION_STATIC_CONSTRUCTOR(FeatureGeneratorPtr, 2, StaticToDynamicActionFeatures);

template<class FunctionType, class CRAlgorithmType>
inline StateFunctionPtr staticToDynamicStateFunction()
{
  typedef typename FunctionType::__ReturnType__ ReturnType;
  enum {NumParameters = FunctionType::numParameters};
  typedef StaticToDynamicStateFunctionConstructor<ReturnType, NumParameters> ConstructorType;
  return StateFunctionPtr(ConstructorType::create(*(const FunctionType* )0, *(const CRAlgorithmType* )0));
}

#endif // 0

}; /* namespace cralgo */

#endif // !CRALGO_STATIC_CHOOSE_ARGUMENT_HPP_
