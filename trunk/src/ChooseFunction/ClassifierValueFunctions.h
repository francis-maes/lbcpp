/*-----------------------------------------.---------------------------------.
| Filename: ClassifierValueFunction.h      | Classifier-based value functions|
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 16:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_VALUE_FUNCTION_CLASSIFIER_H_
# define LBCPP_CORE_VALUE_FUNCTION_CLASSIFIER_H_

# include <lbcpp/ChooseFunction.h>
# include <lbcpp/LearningMachine.h>

namespace lbcpp
{

// /!\ only works with choices of type "size_t" 
class ClassifierBasedActionValueFunction : public ActionValueFunction
{
public:
  ClassifierBasedActionValueFunction(ClassifierPtr classifier)
    : classifier(classifier) {}
    
  virtual std::string toString() const
    {return "predictedActionValues(" + classifier->toString() + ")";}
    
  virtual double compute(VariablePtr vchoice) const
  {
    size_t choice = vchoice->getConstReference<size_t>();
    assert(scores && choice < scores->getNumValues());
    return scores->get(choice);
  }
  
  virtual bool load(std::istream& istr)
    {return read(istr, classifier);}

  virtual void save(std::ostream& ostr)
    {write(ostr, classifier);}
  
protected:
  ClassifierPtr classifier;
  DenseVectorPtr scores;
};

class ClassifierScoresActionValue : public ClassifierBasedActionValueFunction
{
public:
  ClassifierScoresActionValue(ClassifierPtr classifier = ClassifierPtr())
    : ClassifierBasedActionValueFunction(classifier) {}
    
  virtual void setChoose(ChoosePtr choose)
    {scores = classifier->predictScores(choose->computeStateFeatures());}
};

class ClassifierProbabilitiesActionValue  : public ClassifierBasedActionValueFunction
{
public:
  ClassifierProbabilitiesActionValue(ClassifierPtr classifier = ClassifierPtr())
    : ClassifierBasedActionValueFunction(classifier) {}
    
  virtual std::string toString() const
    {return "probabilitiesActionValues(" + classifier->toString() + ")";}

  virtual void setChoose(ChoosePtr choose)
    {scores = classifier->predictProbabilities(choose->computeStateFeatures());}
};

class GeneralizedClassifierScoresActionValue : public ActionValueFunction
{
public:
  GeneralizedClassifierScoresActionValue(GeneralizedClassifierPtr classifier = GeneralizedClassifierPtr())
    : classifier(classifier) {}
    
  virtual std::string toString() const
    {return "predictedActionValues(" + classifier->toString() + ")";}

  virtual void setChoose(ChoosePtr choose)
    {this->choose = choose;}

  virtual double compute(VariablePtr variable) const
    {return classifier->predictScore(choose->computeActionFeatures(variable));}

  virtual bool load(std::istream& istr)
    {return read(istr, classifier);}

  virtual void save(std::ostream& ostr)
    {write(ostr, classifier);}

protected:
  GeneralizedClassifierPtr classifier;
  ChoosePtr choose;
};

class GeneralizedClassifierProbabilitiesActionValue : public ActionValueFunction
{
public:
  GeneralizedClassifierProbabilitiesActionValue(GeneralizedClassifierPtr classifier = GeneralizedClassifierPtr())
    : classifier(classifier) {}
    
  virtual std::string toString() const
    {return "probabilitiesActionValues(" + classifier->toString() + ")";}

  virtual void setChoose(ChoosePtr choose)
  {
    DenseVectorPtr probabilities = classifier->predictProbabilities(choose->computeActionsFeatures(false));
    size_t i = 0;
    for (VariableIteratorPtr iterator = choose->newIterator(); iterator->exists(); iterator->next())
      probs[iterator->get()->toString()] = probabilities->get(i++);
  }

  virtual double compute(VariablePtr variable) const
  {
    assert(probs.find(variable->toString()) != probs.end());
    return probs.find(variable->toString())->second;
  }
  
  virtual bool load(std::istream& istr)
    {return read(istr, classifier);}

  virtual void save(std::ostream& ostr)
    {write(ostr, classifier);}

protected:
  GeneralizedClassifierPtr classifier;
  std::map<std::string, double> probs;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_VALUE_FUNCTION_CLASSIFIER_H_
