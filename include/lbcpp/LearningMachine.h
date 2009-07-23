/*
** $PROJECT_PRESENTATION_AND_CONTACT_INFOS$
**
** Copyright (C) 2009 Francis MAES
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
| Filename: LearningMachine.h              | Learning machines               |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 22:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   LearningMachine.h
**@author Francis MAESS
**@date   Mon Jun 15 23:48:37 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_LEARNING_MACHINE_H_
# define LBCPP_LEARNING_MACHINE_H_

# include "ContinuousFunction.h"
# include "ObjectContainer.h"

namespace lbcpp
{

/*!
** @class LearningMachine
** @brief
*/

class LearningMachine : public Object
{
public:
  /*
  ** Stochastic training
  */
  /*!
  **
  **
  ** @param examples
  ** @param progress
  */
  virtual void trainStochastic(ObjectStreamPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr());

  /*!
  **
  **
  ** @param examples
  ** @param progress
  */
  virtual void trainStochastic(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr());

  /*!
  **
  **
  ** @param inputDictionary
  */
  virtual void trainStochasticBegin(FeatureDictionaryPtr inputDictionary) {assert(false);}

  /*!
  **
  **
  ** @param example
  */
  virtual void trainStochasticExample(ObjectPtr example) {assert(false);}

  /*!
  **
  **
  */
  virtual void trainStochasticEnd() {assert(false);}

  /*
  ** Batch training
  */
  /*!
  **
  **
  ** @param examples
  ** @param progress
  **
  ** @return
  */
  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
    {assert(false); return false;}
  /*!
  **
  **
  ** @param examples
  ** @param progress
  **
  ** @return
  */
  virtual bool trainBatch(ObjectStreamPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr());

  /*
  ** Input dictionary
  */
  /*!
  **
  **
  **
  ** @return
  */
  virtual FeatureDictionaryPtr getInputDictionary() const
    {return FeatureDictionaryPtr();}

  //LearningMachinePtr batchToStochasticMachine() const;

protected:
  void cloneImpl(LearningMachine& target) const {}
};
typedef ReferenceCountedObjectPtr<LearningMachine> LearningMachinePtr;


/*!
** @class Classifier
** @brief
*/
class Classifier : public LearningMachine
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  Classifier() : labels(NULL) {}

  /*
  ** Abstract
  */
  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr input) const = 0;

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual size_t predict(const FeatureGeneratorPtr input) const;

  /*!
  **
  **
  ** @param input
  ** @param output
  **
  ** @return
  */
  virtual double predictScore(const FeatureGeneratorPtr input, size_t output) const;

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual DenseVectorPtr predictProbabilities(const FeatureGeneratorPtr input) const;

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual size_t sample(const FeatureGeneratorPtr input) const;

  /*
  ** Labels
  */
  /*!
  **
  **
  **
  ** @return
  */
  size_t getNumLabels() const
    {assert(labels); return labels->getNumElements();}

  /*!
  **
  **
  **
  ** @return
  */
  StringDictionaryPtr getLabels() const
    {assert(labels); return labels;}

  /*!
  **
  **
  ** @param labels
  */
  virtual void setLabels(StringDictionaryPtr labels)
    {this->labels = labels;}

  /*
  ** Evaluation
  */
  /*!
  **
  **
  ** @param examples
  **
  ** @return
  */
  double evaluateAccuracy(ObjectStreamPtr examples) const;
  /*!
  **
  **
  ** @param examples
  **
  ** @return
  */
  double evaluateAccuracy(ObjectContainerPtr examples) const
    {return evaluateAccuracy(examples->toStream());}

  /*!
  **
  **
  ** @param examples
  **
  ** @return
  */
  double evaluateWeightedAccuracy(ObjectStreamPtr examples) const;
  /*!
  **
  **
  ** @param examples
  **
  ** @return
  */
  double evaluateWeightedAccuracy(ObjectContainerPtr examples) const
    {return evaluateWeightedAccuracy(examples->toStream());}

  /*
  ** Serialization
  */
  /*!
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const;
  /*!
  **
  **
  ** @param istr
  **
  ** @return
  */
  virtual bool load(std::istream& istr);

protected:
  StringDictionaryPtr labels;   /*!< */

  /*!
  **
  **
  ** @param target
  */
  void cloneImpl(Classifier& target) const
    {target.labels = labels;}
};

/*!
**
**
** @param filename
**
** @return
*/
inline ClassifierPtr loadClassifier(const std::string& filename)
  {return Object::loadFromFileAndCast<Classifier>(filename);}


/*!
** @class BinaryClassifier
** @brief
*/
class BinaryClassifier : public Classifier
{
public:
  /*!
  **
  **
  ** @param labels
  */
  virtual void setLabels(StringDictionaryPtr labels)
    {this->labels = labels; outputsDictionary = new FeatureDictionary("BinaryClassifier outputs", labels, StringDictionaryPtr());}

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual double predictScoreOfPositiveClass(const FeatureGeneratorPtr input) const = 0;

  /*!
  **
  **
  ** @param score
  **
  ** @return
  */
  virtual double scoreToProbability(double score) const
    {return 1.0 / (1.0 + exp(-score));} // default: apply a sigmoid

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual size_t predict(const FeatureGeneratorPtr input) const;

  /*!
  **
  **
  ** @param input
  ** @param output
  **
  ** @return
  */
  virtual double predictScore(const FeatureGeneratorPtr input, size_t output) const;

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr input) const;

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual DenseVectorPtr predictProbabilities(const FeatureGeneratorPtr input) const;

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual size_t sample(const FeatureGeneratorPtr input) const;

private:
  FeatureDictionaryPtr outputsDictionary; /*!< */
};

/*!
**
**
** @param filename
**
** @return
*/
inline BinaryClassifierPtr loadBinaryClassifier(const std::string& filename)
  {return Object::loadFromFileAndCast<BinaryClassifier>(filename);}


/*!
** @class GeneralizedClassifier
** @brief
*/
class GeneralizedClassifier : public LearningMachine
{
public:
  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual double predictScore(const FeatureGeneratorPtr input) const = 0;

  /*!
  **
  **
  ** @param example
  **
  ** @return
  */
  virtual size_t predict(const GeneralizedClassificationExample& example) const;

  /*!
  **
  **
  ** @param compositeInput
  **
  ** @return
  */
  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr compositeInput) const;

  /*!
  **
  **
  ** @param compositeInput
  **
  ** @return
  */
  virtual DenseVectorPtr predictProbabilities(const FeatureGeneratorPtr compositeInput) const;

  /*!
  **
  **
  ** @param compositeInput
  **
  ** @return
  */
  virtual size_t sample(const FeatureGeneratorPtr compositeInput) const;
};


/*!
** @class Regressor
** @brief
*/
class Regressor : public LearningMachine
{
public:
  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual double predict(const FeatureGeneratorPtr input) const = 0;

  /*!
  **
  **
  ** @param examples
  **
  ** @return
  */
  double evaluateMeanAbsoluteError(ObjectStreamPtr examples) const;

  /*!
  **
  **
  ** @param examples
  **
  ** @return
  */
  double evaluateMeanAbsoluteError(ObjectContainerPtr examples) const
    {return evaluateMeanAbsoluteError(examples->toStream());}
};

/*!
**
**
** @param filename
**
** @return
*/
inline RegressorPtr loadRegressor(const std::string& filename)
  {return Object::loadFromFileAndCast<Regressor>(filename);}

/*!
**
**
** @param ostr
**
** @return
*/
extern RegressorPtr verboseRegressor(std::ostream& ostr);


/*!
** @class Ranker
** @brief
*/
class Ranker : public LearningMachine
{
public:
  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual double predictScore(const FeatureGeneratorPtr input) const = 0;

  /*!
  **
  **
  ** @param compositeInput
  **
  ** @return
  */
  virtual size_t predict(const FeatureGeneratorPtr compositeInput) const;

  /*!
  **
  **
  ** @param compositeInput
  **
  ** @return
  */
  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr compositeInput) const;

  /*!
  **
  **
  ** @param examples
  **
  ** @return
  */
  double evaluateMeanTopRankCost(ObjectStreamPtr examples) const;

  /*!
  **
  **
  ** @param examples
  **
  ** @return
  */
  double evaluateMeanTopRankCost(ObjectContainerPtr examples) const
    {return evaluateMeanTopRankCost(examples->toStream());}
};

/*!
**
**
** @param filename
**
** @return
*/
inline RankerPtr loadRanker(const std::string& filename)
  {return Object::loadFromFileAndCast<Ranker>(filename);}

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_MACHINE_H_
