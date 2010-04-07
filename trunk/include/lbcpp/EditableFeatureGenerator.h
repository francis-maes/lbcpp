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

/*!
**@file   EditableFeatureGenerator.h
**@author Francis MAES
**@date   Fri Jun 12 17:36:43 2009
**
**@brief  #FIXME: all
**
**
*/

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
      visitor.featureCall_(getDictionary(), i, featureGenerators[i]);
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

