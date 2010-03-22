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
| Filename: GradientBasedLearningMachine.h | Gradient based learning machines|
| Author  : Francis Maes                   |                                 |
| Started : 09/03/2009 20:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   GradientBasedLearningMachine.h
**@author Francis MAES
**@date   Mon Jun 15 19:21:11 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_GRADIENT_BASED_LEARNING_MACHINE_H_
# define LBCPP_GRADIENT_BASED_LEARNING_MACHINE_H_

# include "LearningMachine.h"
# include "GradientBasedLearner.h"
# include "RandomVariable.h"
# include "ObjectContainer.h"

namespace lbcpp
{

/*!
** @class GradientBasedLearningMachine
** @brief
*/
class GradientBasedLearningMachine
{
public:

  /*!
  **
  **
  */
  GradientBasedLearningMachine() : initializeParametersRandomly(false)
    {}

  /*!
  **
  **
  **
  ** @return
  */
  virtual ~GradientBasedLearningMachine() {}

  /*
  ** Abstract
  */
  /*!
  **
  **
  ** @param example
  **
  ** @return
  */
  virtual FeatureDictionaryPtr getInputDictionaryFromExample(ObjectPtr example) = 0;
  /*!
  **
  **
  ** @param inputDictionary
  ** @param initializeRandomly
  **
  ** @return
  */
  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const = 0;
  /*!
  **
  **
  ** @param example
  **
  ** @return
  */
  virtual ScalarVectorFunctionPtr getLoss(ObjectPtr example) const = 0;
  /*!
  **
  **
  ** @param examples
  **
  ** @return
  */
  virtual ScalarVectorFunctionPtr getEmpiricalRisk(ObjectContainerPtr examples) const = 0;
  /*!
  **
  **
  ** @param examples
  **
  ** @return
  */
  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(ObjectContainerPtr examples) const = 0;

  /*
  ** Input Dictionary
  */
  /*!
  **
  **
  **
  ** @return
  */
  FeatureDictionaryPtr getInputDictionary() const
    {return inputDictionary;}

  /*!
  **
  **
  ** @param dictionary
  **
  ** @return
  */
  bool ensureInputDictionary(FeatureDictionaryPtr dictionary)
  {
    if (inputDictionary)
      return inputDictionary->checkEquals(dictionary);
    inputDictionary = dictionary;
    return true;
  }

  /*
  ** Parameters
  */
  /*!
  **
  **
  **
  ** @return
  */
  DenseVectorPtr getParameters() const
    {return parameters;}

  /*!
  **
  **
  ** @param inputDictionary
  ** @param initializeRandomly
  */
  void createParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly)
  {
    ensureInputDictionary(inputDictionary);
    assert(!parameters);
    parameters = createInitialParameters(inputDictionary, initializeRandomly);
  }

  /*!
  **
  **
  ** @param parameters
  */
  void setParameters(DenseVectorPtr parameters)
    {this->parameters = parameters;}

  /*!
  **
  **
  */
  void setInitializeParametersRandomly()
  {
    assert(!parameters); // This function should be called before parameters creation.
    initializeParametersRandomly = true;
  }

  /*
  ** Regularizer
  */
  /*!
  **
  **
  **
  ** @return
  */
  ScalarVectorFunctionPtr getRegularizer() const
    {return regularizer;}

  /*!
  **
  **
  ** @param regularizer
  */
  void setRegularizer(ScalarVectorFunctionPtr regularizer)
    {this->regularizer = regularizer;}

  /*!
  **
  **
  ** @param weight
  */
  void setL2Regularizer(double weight)
    {regularizer = sumOfSquaresFunction(weight);}

  /*
  ** Learner
  */
  /*!
  **
  **
  **
  ** @return
  */
  GradientBasedLearnerPtr getLearner() const
    {return learner;}

  /*!
  **
  **
  ** @param learner
  */
  void setLearner(GradientBasedLearnerPtr learner)
    {this->learner = learner;}

  /*
  ** Shortcuts for functions computation
  */
  /*!
  **
  **
  **
  ** @return
  */
  double computeRegularizer() const
    {assert(parameters); return regularizer ? regularizer->compute(parameters) : 0.0;}

  /*!
  **
  **
  ** @param examples
  **
  ** @return
  */
  double computeEmpiricalRisk(ObjectContainerPtr examples) const
    {assert(parameters); return getEmpiricalRisk(examples)->compute(parameters);}

  /*!
  **
  **
  ** @param examples
  **
  ** @return
  */
  double computeRegularizedEmpiricalRisk(ObjectContainerPtr examples) const
    {assert(parameters); return getRegularizedEmpiricalRisk(examples)->compute(parameters);}

protected:
  FeatureDictionaryPtr inputDictionary; /*!< */
  DenseVectorPtr parameters;    /*!< */
  ScalarVectorFunctionPtr regularizer; /*!< */
  GradientBasedLearnerPtr learner; /*!< */
  bool initializeParametersRandomly; /*!< */

  /*
  ** Serialization
  */
  /*!
  **
  **
  ** @param ostr
  */
  void saveImpl(std::ostream& ostr) const;
  /*!
  **
  **
  ** @param istr
  **
  ** @return
  */
  bool loadImpl(std::istream& istr);
  /*!
  **
  **
  ** @param target
  */
  void cloneImpl(GradientBasedLearningMachine& target) const;

  /*
  ** Training
  */
  /*!
  **
  **
  ** @param inputDictionary
  */
  void trainStochasticBeginImpl(FeatureDictionaryPtr inputDictionary);
  /*!
  **
  **
  ** @param gradient
  ** @param weight
  */
  void trainStochasticExampleImpl(FeatureGeneratorPtr gradient, double weight);
  /*!
  **
  **
  ** @param example
  */
  void trainStochasticExampleImpl(ObjectPtr example);
  /*!
  **
  **
  */
  void trainStochasticEndImpl();
  /*!
  **
  **
  ** @param examples
  ** @param progress
  **
  ** @return
  */
  bool trainBatchImpl(ObjectContainerPtr examples, ProgressCallbackPtr progress);
};


template<class BaseClass, class ExampleType_>
class GradientBasedLearningMachine_ : public BaseClass, public GradientBasedLearningMachine
{
public:
  typedef ExampleType_ ExampleType;
  typedef ReferenceCountedObjectPtr<ExampleType> ExampleTypePtr;

  virtual FeatureDictionaryPtr getInputDictionaryFromExample(ObjectPtr example)
    {return example.staticCast<ExampleType>()->getInput()->getDictionary();}

  virtual FeatureDictionaryPtr getInputDictionary() const
    {return GradientBasedLearningMachine::getInputDictionary();}

  /*
  ** LearningMachine
  */
  virtual void trainStochasticBegin(FeatureDictionaryPtr inputDictionary)
    {trainStochasticBeginImpl(inputDictionary);}

  void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
    {trainStochasticExampleImpl(gradient, weight);}

  virtual void trainStochasticExample(ObjectPtr example)
    {trainStochasticExampleImpl(example);}

  virtual void trainStochasticEnd()
    {trainStochasticEndImpl();}

  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
    {return trainBatchImpl(examples, progress);}

  /*
  ** Serialization
  */
  virtual void save(std::ostream& ostr) const
    {BaseClass::save(ostr); saveImpl(ostr);}

  virtual bool load(std::istream& istr)
    {return BaseClass::load(istr) && loadImpl(istr);}

  void cloneImpl(GradientBasedLearningMachine_<BaseClass, ExampleType>& target) const
  {
    BaseClass::cloneImpl(target);
    GradientBasedLearningMachine::cloneImpl(target);
  }
};

/*
** Regression
*/
/*!
** @class GradientBasedRegressor
** @brief
*/
class GradientBasedRegressor : public GradientBasedLearningMachine_<Regressor, RegressionExample>
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  /*!
  **
  **
  ** @param inputDictionary
  ** @param initializeRandomly
  **
  ** @return
  */
  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual double predict(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

/*!
**
**
** @param filename
**
** @return
*/
inline GradientBasedRegressorPtr loadGradientBasedRegressor(const std::string& filename)
  {return Object::loadFromFileAndCast<GradientBasedRegressor>(filename);}

/*!
**
**
** @param learner
** @param l2Regularizer
**
** @return
*/
extern GradientBasedRegressorPtr leastSquaresLinearRegressor(GradientBasedLearnerPtr learner, double l2Regularizer = 0.0);

/*
** Classification
*/
/*!
** @class GradientBasedClassifier
** @brief
*/
class GradientBasedClassifier : public GradientBasedLearningMachine_<Classifier, ClassificationExample>
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual VectorArchitecturePtr getPredictionArchitecture() const = 0;

  /*!
  **
  **
  ** @param inputDictionary
  ** @param initializeRandomly
  **
  ** @return
  */
  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input)->toDenseVector();}

  /*!
  **
  **
  ** @param input
  ** @param output
  **
  ** @return
  */
  virtual double predictScore(const FeatureGeneratorPtr input, size_t output) const
    {return getPredictionArchitecture()->compute(parameters, input, output);}
};

/*!
**
**
** @param filename
**
** @return
*/
inline GradientBasedClassifierPtr loadGradientBasedClassifier(const std::string& filename)
  {return Object::loadFromFileAndCast<GradientBasedClassifier>(filename);}

/*!
**
**
** @param learner
** @param labels
** @param l2regularizer
**
** @return
*/
extern GradientBasedClassifierPtr maximumEntropyClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels, double l2regularizer = 0.0);


/*
** Binary Classification
*/
class GradientBasedBinaryClassifier : public GradientBasedLearningMachine_<BinaryClassifier, ClassificationExample>
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  /*!
  **
  **
  ** @param inputDictionary
  ** @param initializeRandomly
  **
  ** @return
  */
  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual double predictScoreOfPositiveClass(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

/*!
**
**
** @param filename
**
** @return
*/
inline GradientBasedBinaryClassifierPtr loadGradientBasedBinaryClassifier(const std::string& filename)
  {return Object::loadFromFileAndCast<GradientBasedBinaryClassifier>(filename);}

/*!
**
**
** @param learner
** @param labels
**
** @return
*/
extern GradientBasedBinaryClassifierPtr linearSVMBinaryClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels);

/*!
**
**
** @param learner
** @param labels
** @param l2regularizer
**
** @return
*/
extern GradientBasedBinaryClassifierPtr logisticRegressionBinaryClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels, double l2regularizer = 0.0);

/*
** Generalized Classification
*/
/*!
** @class GradientBasedGeneralizedClassifier
**
*/
class GradientBasedGeneralizedClassifier
  : public GradientBasedLearningMachine_<GeneralizedClassifier, GeneralizedClassificationExample>
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  /*!
  **
  **
  ** @param inputDictionary
  ** @param initializeRandomly
  **
  ** @return
  */
  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual double predictScore(const FeatureGeneratorPtr input) const
    {return parameters ? getPredictionArchitecture()->compute(parameters, input) : 0.0;}
};

extern GradientBasedGeneralizedClassifierPtr linearGeneralizedClassifier(GradientBasedLearnerPtr learner);

/*
** Ranker
*/
/*!
** @class GradientBasedRanker
** @brief
*/
class GradientBasedRanker
  : public GradientBasedLearningMachine_<Ranker, RankingExample>
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  /*!
  **
  **
  ** @param inputDictionary
  ** @param initializeRandomly
  **
  ** @return
  */
  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary->getSubDictionary(0), initializeRandomly);}

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual double predictScore(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

/*!
**
**
** @param filename
**
** @return
*/
inline GradientBasedRankerPtr loadGradientBasedRanker(const std::string& filename)
  {return Object::loadFromFileAndCast<GradientBasedRanker>(filename);}

/*!
**
**
** @param learner
**
** @return
*/
extern GradientBasedRankerPtr largeMarginAllPairsLinearRanker(GradientBasedLearnerPtr learner);

/*!
**
**
** @param learner
**
** @return
*/
extern GradientBasedRankerPtr largeMarginMostViolatedPairLinearRanker(GradientBasedLearnerPtr learner);

/*!
**
**
** @param learner
**
** @return
*/
extern GradientBasedRankerPtr largeMarginBestAgainstAllLinearRanker(GradientBasedLearnerPtr learner);

/*!
**
**
** @param learner
**
** @return
*/
extern GradientBasedRankerPtr logBinomialAllPairsLinearRanker(GradientBasedLearnerPtr learner);

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNING_MACHINE_H_
