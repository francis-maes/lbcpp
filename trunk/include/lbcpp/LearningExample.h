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
| Filename: LearningExample.h              | Learning examples               |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 16:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   LearningExample.h
**@author Francis MAES
**@date   Fri Jun 12 18:26:54 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_LEARNING_EXAMPLE_H_
# define LBCPP_LEARNING_EXAMPLE_H_

# include "FeatureGenerator.h"
# include "EditableFeatureGenerator.h"
# include "Utilities.h"

namespace lbcpp
{

/*!
** @class LearningExample
** @brief
*/
class LearningExample : public Object
{
public:
  /*!
  **
  **
  ** @param other
  **
  ** @return
  */
  LearningExample(const LearningExample& other) : weight(other.weight) {}

  /*!
  **
  **
  **
  ** @return
  */
  LearningExample() : weight(1.0) {}

  /*!
  **
  **
  **
  ** @return
  */
  double getWeight() const
    {return weight;}

private:
  double weight;                /*!< */
};

typedef ReferenceCountedObjectPtr<LearningExample> LearningExamplePtr;


class ClassificationExample : public LearningExample
{
public:
  ClassificationExample(const ClassificationExample& other)
    : LearningExample(other), x(other.x), y(other.y) {}

  ClassificationExample(FeatureGeneratorPtr x, size_t y)
    : x(x), y(y) {}

  ClassificationExample()
    : y(0) {}

  virtual String toString() const
    {return T("Y = ") + String((int)y) + T(" X = ") + x->toString();}

  double getMarginMultiplier() const // binary classification
    {return y == 0 ? -1.0 : 1.0;}

  FeatureGeneratorPtr getInput() const
    {return x;}

  size_t getOutput() const
    {return y;}


private:
  FeatureGeneratorPtr x;
  size_t y;
};

typedef ReferenceCountedObjectPtr<ClassificationExample> ClassificationExamplePtr;


class GeneralizedClassificationExample : public LearningExample
{
public:
  GeneralizedClassificationExample(const FeatureGeneratorPtr alternatives, size_t output)
    : alternatives(alternatives), output(output) {}

  FeatureGeneratorPtr getInput() const
    {return alternatives;}

  size_t getOutput() const
    {return output;}

  size_t getNumAlternatives() const
    {return alternatives->getNumSubGenerators();}

  FeatureGeneratorPtr getAlternative(size_t index) const
    {return alternatives->getSubGeneratorWithIndex(index);}

private:
  FeatureGeneratorPtr alternatives;
  size_t output;
};

typedef ReferenceCountedObjectPtr<GeneralizedClassificationExample> GeneralizedClassificationExamplePtr;

class RegressionExample : public LearningExample
{
public:
  RegressionExample(FeatureGeneratorPtr x, double y)
    : x(x), y(y) {}

  virtual String toString() const
    {return T("Y = ") + String(y) + T(" X = ") + x->toString();}

  FeatureGeneratorPtr getInput() const
    {return x;}

  double getOutput() const
    {return y;}

private:
  FeatureGeneratorPtr x;        /*!< */
  double y;                     /*!< */
};

typedef ReferenceCountedObjectPtr<RegressionExample> RegressionExamplePtr;


/*!
** @class RankingExample
** @brief
*/
class RankingExample : public LearningExample
{
public:
  /*!
  **
  **
  ** @param alternatives
  ** @param costs
  **
  ** @return
  */
  RankingExample(const FeatureGeneratorPtr alternatives, const std::vector<double>& costs)
    : alternatives(alternatives), costs(costs) {}

  /*!
  **
  **
  **
  ** @return
  */
  FeatureGeneratorPtr getInput() const
    {return alternatives;}

  /*!
  **
  **
  **
  ** @return
  */
  const std::vector<double>& getCosts() const
    {return costs;}

  /*!
  **
  **
  ** @param alternativeIndex
  **
  ** @return
  */
  FeatureGeneratorPtr getAlternative(size_t alternativeIndex) const
    {return alternatives->getSubGenerator(alternativeIndex);}

  /*!
  **
  **
  ** @param alternativeIndex
  **
  ** @return
  */
  double getCost(size_t alternativeIndex) const
    {jassert(alternativeIndex < costs.size()); return costs[alternativeIndex];}

private:
  FeatureGeneratorPtr alternatives; /*!< */
  std::vector<double> costs;    /*!< */
};

typedef ReferenceCountedObjectPtr<RankingExample> RankingExamplePtr;

/*!
**
**
** @param labels
**
** @return
*/
extern ObjectFunctionPtr transformClassificationExampleIntoRankingExample(StringDictionaryPtr labels);

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_EXAMPLE_H_
