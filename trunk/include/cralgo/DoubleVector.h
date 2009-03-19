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

class DoubleVector : public FeatureGeneratorDefaultImplementations<DoubleVector, FeatureGenerator>
{
public:
  DoubleVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr())
    : dictionary(dictionary) {}

  /*
  ** DoubleVector
  */
  virtual void clear() = 0;
  
  /*
  ** Dictionary
  */
  void setDictionary(FeatureDictionaryPtr dictionary)
    {this->dictionary = dictionary;}
    
  void ensureDictionary(FeatureDictionaryPtr dictionary)
  {
    if (this->dictionary)
    {
      if (this->dictionary != dictionary)
      {
        std::cerr << "Error: dictionary mismatch. This dictionary = '" << this->dictionary->getName() << "', "
                     "required dictionary = '" << dictionary->getName() << "'" << std::endl;
        assert(false);
      }
    }
    else
      this->dictionary = dictionary;
  }
  
  /*
  ** Static FeatureGenerator
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionaryPtr featureDictionary) const;
  
  /*
  ** FeatureGenerator
  */
  virtual std::string getName() const
    {return "DoubleVector";}
    
  virtual FeatureDictionaryPtr getDictionary() const
  {
    if (dictionary)
      return dictionary;
    static FeatureDictionaryPtr defaultDictionary = new FeatureDictionary("DoubleVector");
    return defaultDictionary;
  }

protected:
  FeatureDictionaryPtr dictionary;
};

}; /* namespace cralgo */

#endif // !CRALGO_DOUBLE_VECTOR_H_

