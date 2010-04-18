/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: EditableFeatureGenerator.h     | Base class for real valued      |
| Author  : Francis Maes                   |     vectors                     |
| Started : 19/02/2009 17:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DOUBLE_VECTOR_H_
# define LBCPP_DOUBLE_VECTOR_H_

# include "FeatureGenerator.h"
# include <iostream>

namespace lbcpp
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
  bool hasDictionary() const
    {return dictionary != FeatureDictionaryPtr();}
  void setDictionary(FeatureDictionaryPtr dictionary);
  void ensureDictionary(FeatureDictionaryPtr dictionary);

  /*
  ** Static FeatureGenerator
  */
  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor) const;

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
  void staticFeatureGenerator(FeatureVisitor& visitor) const;
};


class Scalar : public FeatureGeneratorDefaultImplementations<Scalar, FlatFeatureGenerator>
{
public:
  Scalar(double value = 0.0)
    : value(value) {}

  static FeatureDictionaryPtr getDictionaryInstance()
  {
    static FeatureDictionaryPtr dictionary;
    if (!dictionary)
    {
      dictionary = new FeatureDictionary(T("Scalar"), new StringDictionary(T("Scalar features")), StringDictionaryPtr());
      dictionary->addFeature(T("value"));
    }
    return dictionary;
  }

  virtual FeatureDictionaryPtr getDictionary() const
    {return getDictionaryInstance();}

  virtual String toString() const
    {return String(value);}

  double getValue() const
    {return value;}

  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor) const;

private:
  double value;
};

typedef ReferenceCountedObjectPtr<Scalar> ScalarPtr;

class Label : public FeatureGeneratorDefaultImplementations<Label, EditableFeatureGenerator>
{
public:
  Label(FeatureDictionaryPtr featureDictionary, size_t index = 0)
    : index(index)
    {setDictionary(featureDictionary);}

  Label(StringDictionaryPtr stringDictionary, size_t index = 0)
    : index(index)
    {setDictionary(FeatureDictionaryManager::getInstance().getFlatVectorDictionary(stringDictionary));}

  Label() : index(0) {}

  virtual String toString() const
    {return getString();}

  virtual void clear()
    {index = 0;}

  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor) const;

  size_t getIndex() const
    {return index;}

  String getString() const
    {return getDictionary()->getFeature(index);}

protected:
  // FeatureGenerator
  virtual size_t getNumSubGenerators() const
    {return 0;}
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {jassert(false); return FeatureGeneratorPtr();}
  virtual size_t getSubGeneratorIndex(size_t num) const
    {jassert(false); return (size_t)-1;}
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
    {jassert(false); return FeatureGeneratorPtr();}

private:
  size_t index;
};

typedef ReferenceCountedObjectPtr<Label> LabelPtr;

class LazyFeatureVector : public EditableFeatureGenerator
{
public:
  LazyFeatureVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr())
    : EditableFeatureGenerator(dictionary) {}

  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor) const
    {getResult()->staticFeatureGenerator(visitor);}

  virtual size_t getNumSubGenerators() const;
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const;
  virtual size_t getSubGeneratorIndex(size_t num) const;
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const;

protected:
  virtual FeatureVectorPtr computeVector() const = 0;

  FeatureVectorPtr getResult() const;
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

  CompositeFeatureGenerator(FeatureDictionaryPtr dictionary, const std::vector<FeatureGeneratorPtr>& featureGenerators)
    : BaseClass(dictionary), featureGenerators(featureGenerators) {}
  CompositeFeatureGenerator(FeatureDictionaryPtr dictionary, size_t numSubGenerators)
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
  void staticFeatureGenerator(VisitorType& visitor) const
  {
    for (size_t i = 0; i < featureGenerators.size(); ++i)
      visitor.featureCall_(featureGenerators[i], getDictionary(), i);
  }

  virtual size_t getNumSubGenerators() const;
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const;
  virtual size_t getSubGeneratorIndex(size_t num) const;
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const;

protected:
  std::vector<FeatureGeneratorPtr> featureGenerators;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DOUBLE_VECTOR_H_

