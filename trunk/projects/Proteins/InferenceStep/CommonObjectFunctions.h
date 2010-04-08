/*-----------------------------------------.---------------------------------.
| Filename: CommonObjectFunctions.h        | Common ObjectFunctions          |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 18:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_COMMON_OBJECT_FUNCTIONS_H_
# define LBCPP_COMMON_OBJECT_FUNCTIONS_H_

# include <lbcpp/lbcpp.h>

using juce::Time;

namespace lbcpp
{

class Chain2ObjectFunction : public ObjectFunction
{
public:
  Chain2ObjectFunction(ObjectFunctionPtr f1, ObjectFunctionPtr f2)
    : f1(f1), f2(f2) {}

  virtual String getInputClassName() const
    {return f1->getInputClassName();}
  
  virtual String getOutputClassName(const String& inputClassName) const
  {
    String className = f1->getOutputClassName(inputClassName);
    if (className.isEmpty())
      return String::empty;
    return f2->getOutputClassName(className);
  }

  virtual ObjectPtr function(ObjectPtr object) const
  {
    ObjectPtr res1 = f1->function(object);
    return res1 ? f2->function(res1) : ObjectPtr();
  }

protected:
  ObjectFunctionPtr f1, f2;
};

class StringToObjectMapAccessorFunction : public ObjectFunction
{
public:
  StringToObjectMapAccessorFunction(const String& key)
    : key(key) {}

  virtual String getInputClassName() const
    {return T("StringToObjectMap");}
  
  virtual String getOutputClassName(const String& ) const
    {return T("Object");}

  virtual ObjectPtr function(ObjectPtr object) const
  {
    StringToObjectMapPtr m = object.dynamicCast<StringToObjectMap>();
    jassert(m);
    return m->getObject(key);
  }

protected:
  String key;
};

class StringToObjectMapSetterFunction : public ObjectFunction
{
public:
  StringToObjectMapSetterFunction(StringToObjectMapPtr objects, const String& key)
    : objects(objects), key(key) {}

  virtual String getInputClassName() const
    {return T("Object");}
  
  virtual String getOutputClassName(const String& ) const
    {return String::empty;}

  virtual ObjectPtr function(ObjectPtr object) const
  {
    objects->setObject(key, object);
    return ObjectPtr();
  }

protected:
  StringToObjectMapPtr objects;
  String key;
};


class ClassifierFunction : public ObjectFunction
{
public:
  ClassifierFunction(ClassifierPtr classifier = ClassifierPtr())
    : classifier(classifier) {}

  virtual String getInputClassName() const
    {return T("FeatureGenerator");}
  
  virtual String getOutputClassName(const String& ) const
    {return T("FeatureVector");}

protected:
  ClassifierPtr classifier;
};

class ProbabilisticClassifierFunction : public ClassifierFunction
{
public:
  ProbabilisticClassifierFunction(ClassifierPtr classifier = ClassifierPtr())
    : ClassifierFunction(classifier) {}

  virtual ObjectPtr function(ObjectPtr object) const
    {return classifier->predictProbabilities(object.dynamicCast<FeatureGenerator>());}
};

class DeterministicClassifier : public ClassifierFunction
{
public:
  DeterministicClassifier(ClassifierPtr classifier = ClassifierPtr())
    : ClassifierFunction(classifier)
  {
    if (classifier)
      dictionary = FeatureDictionaryManager::getInstance().getFlatVectorDictionary(classifier->getLabels());
  }

  virtual ObjectPtr function(ObjectPtr object) const
  {
    jassert(dictionary);
    SparseVectorPtr res = new SparseVector(dictionary);
    res->set(classifier->predict(object.dynamicCast<FeatureGenerator>()), 1.0);
    return res;
  }

private:
  FeatureDictionaryPtr dictionary;
};

}; /* namespace lbcpp */

#endif //!LBCPP_COMMON_OBJECT_FUNCTIONS_H_
