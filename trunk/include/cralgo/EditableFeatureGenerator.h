/*-----------------------------------------.---------------------------------.
| Filename: EditableFeatureGenerator.h                 | Base class for real valued      |
| Author  : Francis Maes                   |     vectors                     |
| Started : 19/02/2009 17:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_DOUBLE_VECTOR_H_
# define CRALGO_DOUBLE_VECTOR_H_

# include "FeatureGenerator.h"
# include <iostream>

namespace cralgo
{

class EditableFeatureGenerator : public FeatureGeneratorDefaultImplementations<EditableFeatureGenerator, FeatureGenerator>
{
public:
  EditableFeatureGenerator(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr())
    : dictionary(dictionary) {}

  /*
  ** EditableFeatureGenerator
  */
  virtual void clear() = 0;
  
  /*
  ** Dictionary
  */
  void setDictionary(FeatureDictionaryPtr dictionary);
  void ensureDictionary(FeatureDictionaryPtr dictionary);
    
  /*
  ** Static FeatureGenerator
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionaryPtr featureDictionary) const;
  
  /*
  ** FeatureGenerator
  */
  virtual FeatureDictionaryPtr getDictionary() const;
  
protected:
  FeatureDictionaryPtr dictionary;
};

class FeatureVector : public EditableFeatureGenerator
{
public:
  FeatureVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr())
    : EditableFeatureGenerator(dictionary) {}
    
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionaryPtr featureDictionary) const;  
};

class LazyFeatureVector : public EditableFeatureGenerator
{
public:
  LazyFeatureVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr())
    : EditableFeatureGenerator(dictionary) {}
  
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionaryPtr featureDictionary) const
    {getResult()->staticFeatureGenerator(visitor, featureDictionary);}
  
  virtual size_t getNumSubGenerators() const;
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const;
  virtual size_t getSubGeneratorIndex(size_t num) const;
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const;

protected:
  virtual FeatureVectorPtr computeVector() const = 0;

  FeatureVectorPtr getResult() const;

private:
  FeatureVectorPtr result;
};

/*
** (FG1, ..., FGn)
*/
class CompositeFeatureGenerator : 
  public FeatureGeneratorDefaultImplementations<CompositeFeatureGenerator, EditableFeatureGenerator>
{
public:
  typedef FeatureGeneratorDefaultImplementations<CompositeFeatureGenerator, EditableFeatureGenerator> BaseClass;

  CompositeFeatureGenerator(const std::vector<FeatureGeneratorPtr>& featureGenerators)
    : featureGenerators(featureGenerators) {}
  CompositeFeatureGenerator(size_t numSubGenerators, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr())
    : BaseClass(dictionary), featureGenerators(numSubGenerators, FeatureGeneratorPtr()) {}
  CompositeFeatureGenerator() {}
    
  /*
  ** Sub feature-generators
  */
  void setSubGenerator(size_t index, FeatureGeneratorPtr featureGenerator);
  void appendSubGenerator(FeatureGeneratorPtr featureGenerator);

  /*
  ** EditableFeatureGenerator
  */
  virtual void clear();

  /*
  ** FeatureGenerator
  */
  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor, FeatureDictionaryPtr dictionary) const
  {
    for (size_t i = 0; i < featureGenerators.size(); ++i)
      visitor.featureCall(dictionary, i, featureGenerators[i]);
  }
  
  virtual size_t getNumSubGenerators() const;
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const;
  virtual size_t getSubGeneratorIndex(size_t num) const;
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const;
  
protected:
  std::vector<FeatureGeneratorPtr> featureGenerators;
};

}; /* namespace cralgo */

#endif // !CRALGO_DOUBLE_VECTOR_H_

