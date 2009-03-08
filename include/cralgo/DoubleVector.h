/*-----------------------------------------.---------------------------------.
| Filename: DoubleVector.h                 | Base class for real valued      |
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

class DoubleVector;
typedef ReferenceCountedObjectPtr<DoubleVector> DoubleVectorPtr;

class DoubleVector : public FeatureGeneratorDefaultImplementations<DoubleVector, FeatureGenerator>
{
public:
  DoubleVector(FeatureDictionary* dictionary = NULL)
    : dictionary(dictionary) {}

  /*
  ** Dictionary
  */
  bool hasDictionary() const
    {return dictionary != NULL;}
    
  FeatureDictionary& getDictionary()
    {assert(dictionary); return *dictionary;}

  void setDictionary(FeatureDictionary& dictionary)
    {this->dictionary = &dictionary;}
    
  void ensureDictionary(FeatureDictionary& dictionary)
  {
    if (this->dictionary)
    {
      if (this->dictionary != &dictionary)
      {
        std::cerr << "Dictionary mismatch. This dictionary = " << this->dictionary->getName()
          << ", required dictionary = " << dictionary.getName() << std::endl;
        assert(false);
      }
    }
    else
      this->dictionary = &dictionary;
  }

  /*
  ** Static FeatureGenerator
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionary& featureDictionary) const;
  
  /*
  ** FeatureGenerator
  */
  virtual std::string getName() const
    {return "DoubleVector";}
    
  virtual FeatureDictionary& getDefaultDictionary() const
    {static FeatureDictionary defaultDictionary("DoubleVector"); return dictionary ? *dictionary : defaultDictionary;}

protected:
  FeatureDictionary* dictionary;
};

}; /* namespace cralgo */

#endif // !CRALGO_DOUBLE_VECTOR_H_

