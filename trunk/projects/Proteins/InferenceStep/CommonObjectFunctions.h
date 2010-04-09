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

class ObjectPair : public Object
{
public:
  ObjectPair(ObjectPtr first, ObjectPtr second)
    : first(first), second(second) {}
  ObjectPair() {}

  ObjectPtr getFirst() const
    {return first;}

  ObjectPtr getSecond() const
    {return second;}

protected:
  ObjectPtr first;
  ObjectPtr second;
};

typedef ReferenceCountedObjectPtr<ObjectPair> ObjectPairPtr;

class ObjectPairContainer : public ObjectContainer
{
public:
  virtual String getFirstClassName() const
    {return T("Object");}

  virtual String getSecondClassName() const
    {return T("Object");}

  virtual std::pair<ObjectPtr, ObjectPtr> getPair(size_t index) const = 0;

  // ObjectContainer
  virtual String getContentClassName() const
    {return T("ObjectPair");}

  virtual ObjectPtr get(size_t index) const
  {
    std::pair<ObjectPtr, ObjectPtr> p = getPair(index);
    return new ObjectPair(p.first, p.second);
  }
};
typedef ReferenceCountedObjectPtr<ObjectPairContainer> ObjectPairContainerPtr;

class ObjectToObjectPairFunction : public ObjectFunction
{
public:
  ObjectToObjectPairFunction(bool copyObjectIntoFirst = true, bool copyObjectIntoSecond = true)
    : copyObjectIntoFirst(copyObjectIntoFirst), copyObjectIntoSecond(copyObjectIntoSecond) {}

  virtual String getInputClassName() const
    {return T("Object");}
  
  virtual String getOutputClassName(const String& ) const
    {return T("ObjectPair");}

  virtual ObjectPtr function(ObjectPtr object) const
    {return new ObjectPair(copyObjectIntoFirst ? object : ObjectPtr(), copyObjectIntoSecond ? object : ObjectPtr());}

private:
  bool copyObjectIntoFirst;
  bool copyObjectIntoSecond;
};

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
  }

  virtual ObjectPtr function(ObjectPtr object) const
  {
    jassert(dictionary);
    jassert(classifier);
    return classifier->predictLabel(object.dynamicCast<FeatureGenerator>());
  }

private:
  FeatureDictionaryPtr dictionary;
};

}; /* namespace lbcpp */

#endif //!LBCPP_COMMON_OBJECT_FUNCTIONS_H_
