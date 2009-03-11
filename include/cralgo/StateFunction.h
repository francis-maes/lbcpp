/*-----------------------------------------.---------------------------------.
| Filename: StateFunction.h                | State function                  |
| Author  : Francis Maes                   |                                 |
| Started : 13/02/2009 17:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_STATE_FUNCTION_H_
# define CRALGO_STATE_FUNCTION_H_

# include "Object.h"
# include "FeatureGenerator.h"
# include <vector>

namespace cralgo
{

class CRAlgorithm;
typedef ReferenceCountedObjectPtr<CRAlgorithm> CRAlgorithmPtr;

class StateFunction : public Object
{
public:
  static std::string makeCompositeName(const std::vector<StateFunction* >& stateFunctions, const char* defaultName)
  {
    std::string res;
    for (size_t i = 0; i < stateFunctions.size(); ++i)
    {
      res += stateFunctions[i]->getName();
      if (i < stateFunctions.size() - 1)
        res += "\n";
    }
    return res.size() ? res : defaultName;
  }
};

typedef ReferenceCountedObjectPtr<StateFunction> StateFunctionPtr;

class StateDescriptionFunction : public StateFunction
{
public:
  virtual std::string toString(CRAlgorithmPtr crAlgorithm) const = 0;
};

typedef ReferenceCountedObjectPtr<StateDescriptionFunction> StateDescriptionFunctionPtr;

class CompositeStateDescriptionFunction : public StateDescriptionFunction
{
public:
  void add(StateDescriptionFunction& description)
    {descriptions.push_back(&description);}

  virtual std::string toString(CRAlgorithmPtr crAlgorithm) const
  {
    std::string res;
    for (size_t i = 0; i < descriptions.size(); ++i)
    {
      res += descriptions[i]->toString(crAlgorithm);
      if (i < descriptions.size() - 1)
        res += "\n";
    }
    return res;
  }
  
  virtual std::string getName() const
    {return makeCompositeName(*(const std::vector<StateFunction* >* )&descriptions, "emptyStateDescription");}

private:
  std::vector<StateDescriptionFunction* > descriptions;
};

class ActionDescriptionFunction : public StateFunction
{
public:
  virtual std::string toString(CRAlgorithmPtr crAlgorithm, const void* choice) const = 0;
};

typedef ReferenceCountedObjectPtr<ActionDescriptionFunction> ActionDescriptionFunctionPtr;

class CompositeActionDescriptionFunction : public ActionDescriptionFunction
{
public:
  void add(ActionDescriptionFunction& description)
    {descriptions.push_back(&description);}

  virtual std::string toString(CRAlgorithmPtr crAlgorithm, const void* choice) const
  {
    std::string res;
    for (size_t i = 0; i < descriptions.size(); ++i)
    {
      res += descriptions[i]->toString(crAlgorithm, choice);
      if (i < descriptions.size() - 1)
        res += "\n";
    }
    return res;
  }

  virtual std::string getName() const
    {return makeCompositeName(*(const std::vector<StateFunction* >* )&descriptions, "emptyActionDescription");}
  
private:
  std::vector<ActionDescriptionFunction* > descriptions;
};

class StateValueFunction : public StateFunction
{
public:
  virtual double compute(CRAlgorithmPtr crAlgorithm) const = 0;
};

typedef ReferenceCountedObjectPtr<StateValueFunction> StateValueFunctionPtr;

class CompositeStateValueFunction : public StateValueFunction
{
public:
  CompositeStateValueFunction() : stateValue(NULL) {}
  
  void add(StateValueFunction& stateValue)
  {
    if (this->stateValue)
      std::cerr << "Warning: only the first state value will be used in choose." << std::endl;
    else
      this->stateValue = &stateValue;
  }

  virtual double compute(CRAlgorithmPtr crAlgorithm) const
    {return stateValue ? stateValue->compute(crAlgorithm) : 0.0;}

  virtual std::string getName() const
    {return stateValue ? stateValue->getName() : "emptyStateValue";}
  
private:
  StateValueFunction* stateValue;
};

class ActionValueFunction : public StateFunction
{
public:
  virtual double compute(CRAlgorithmPtr crAlgorithm, const void* choice) const = 0;
};

typedef ReferenceCountedObjectPtr<ActionValueFunction> ActionValueFunctionPtr;

class CompositeActionValueFunction : public ActionValueFunction
{
public:
  CompositeActionValueFunction() : actionValue(NULL) {}
  
  void add(ActionValueFunction& actionValue)
  {
    if (this->actionValue)
      std::cerr << "Warning: only the first action value will be used in choose." << std::endl;
    else
      this->actionValue = &actionValue;
  }

  virtual double compute(CRAlgorithmPtr crAlgorithm, const void* choice) const
    {return actionValue ? actionValue->compute(crAlgorithm, choice) : 0.0;}

  virtual std::string getName() const
    {return actionValue ? actionValue->getName() : "emptyActionValue";}
  
private:
  ActionValueFunction* actionValue;
};

class StateFeaturesFunction : public StateFunction
{
public:
  virtual FeatureGeneratorPtr featureGenerator(CRAlgorithmPtr crAlgorithm) const = 0;
};
typedef ReferenceCountedObjectPtr<StateFeaturesFunction> StateFeaturesFunctionPtr;


class CompositeStateFeaturesFunction : public StateFeaturesFunction
{
public:
  void add(StateFeaturesFunction& featureFunction)
    {featureFunctions.push_back(&featureFunction);}
    
  virtual FeatureGeneratorPtr featureGenerator(CRAlgorithmPtr crAlgorithm) const
  {
    SumFeatureGeneratorPtr res(new SumFeatureGenerator());
    for (size_t i = 0; i < featureFunctions.size(); ++i)
      res->add(featureFunctions[i]->featureGenerator(crAlgorithm));
    return res;
  }

  virtual std::string getName() const
    {return makeCompositeName(*(const std::vector<StateFunction* >* )&featureFunctions, "emptyStateFeatures");}
        
private:
  std::vector<StateFeaturesFunction* > featureFunctions;
};

class ActionFeaturesFunction : public StateFunction
{
public:
  virtual FeatureGeneratorPtr featureGenerator(CRAlgorithmPtr crAlgorithm, const void* choice) const = 0;
};
typedef ReferenceCountedObjectPtr<ActionFeaturesFunction> ActionFeaturesFunctionPtr;

class CompositeActionFeaturesFunction : public ActionFeaturesFunction
{
public:
  void add(ActionFeaturesFunction& featureFunction)
    {featureFunctions.push_back(&featureFunction);}
    
  virtual FeatureGeneratorPtr featureGenerator(CRAlgorithmPtr crAlgorithm, const void* choice) const
  {
    SumFeatureGeneratorPtr res(new SumFeatureGenerator());
    for (size_t i = 0; i < featureFunctions.size(); ++i)
      res->add(featureFunctions[i]->featureGenerator(crAlgorithm, choice));
    return res;
  }

  virtual std::string getName() const
    {return makeCompositeName(*(const std::vector<StateFunction* >* )&featureFunctions, "emptyActionFeatures");}
        
private:
  std::vector<ActionFeaturesFunction* > featureFunctions;
};

}; /* namespace cralgo */

#endif // !CRALGO_STATE_FUNCTION_H_
