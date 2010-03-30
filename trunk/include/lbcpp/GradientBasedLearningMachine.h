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

#ifndef LBCPP_GRADIENT_BASED_LEARNING_MACHINE_H_
# define LBCPP_GRADIENT_BASED_LEARNING_MACHINE_H_

# include "LearningMachine.h"
# include "GradientBasedLearner.h"
# include "RandomVariable.h"
# include "ObjectContainer.h"

namespace lbcpp
{

class GradientBasedLearningMachine
{
public:
  GradientBasedLearningMachine() : initializeParametersRandomly(false)
    {}

  virtual ~GradientBasedLearningMachine() {}

  /*
  ** Abstract
  */
  virtual FeatureDictionaryPtr getInputDictionaryFromExample(ObjectPtr example) = 0;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const = 0;
  virtual ScalarVectorFunctionPtr getLoss(ObjectPtr example) const = 0;
  virtual ScalarVectorFunctionPtr getEmpiricalRisk(ObjectContainerPtr examples) const = 0;
  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(ObjectContainerPtr examples) const = 0;

  /*
  ** Input Dictionary
  */
  FeatureDictionaryPtr getInputDictionary() const
    {return inputDictionary;}

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
  DenseVectorPtr getParameters() const
    {return parameters;}

  void createParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly)
  {
    ensureInputDictionary(inputDictionary);
    jassert(!parameters);
    parameters = createInitialParameters(inputDictionary, initializeRandomly);
  }

  void setParameters(DenseVectorPtr parameters)
    {this->parameters = parameters;}

  void setInitializeParametersRandomly()
  {
    jassert(!parameters); // This function should be called before parameters creation.
    initializeParametersRandomly = true;
  }

  /*
  ** Regularizer
  */
  ScalarVectorFunctionPtr getRegularizer() const
    {return regularizer;}

  void setRegularizer(ScalarVectorFunctionPtr regularizer)
    {this->regularizer = regularizer;}

  void setL2Regularizer(double weight)
    {regularizer = sumOfSquaresFunction(weight);}

  /*
  ** Learner
  */
  GradientBasedLearnerPtr getLearner() const
    {return learner;}

  void setLearner(GradientBasedLearnerPtr learner)
    {this->learner = learner;}

  /*
  ** Shortcuts for functions computation
  */
  double computeRegularizer() const
    {jassert(parameters); return regularizer ? regularizer->compute(parameters) : 0.0;}

  double computeEmpiricalRisk(ObjectContainerPtr examples) const
    {jassert(parameters); return getEmpiricalRisk(examples)->compute(parameters);}

  double computeRegularizedEmpiricalRisk(ObjectContainerPtr examples) const
    {jassert(parameters); return getRegularizedEmpiricalRisk(examples)->compute(parameters);}

protected:
  FeatureDictionaryPtr inputDictionary; /*!< */
  DenseVectorPtr parameters;    /*!< */
  ScalarVectorFunctionPtr regularizer; /*!< */
  GradientBasedLearnerPtr learner; /*!< */
  bool initializeParametersRandomly; /*!< */

  /*
  ** Serialization
  */
  void saveImpl(OutputStream& ostr) const;
  bool loadImpl(InputStream& istr);
  void cloneImpl(GradientBasedLearningMachine& target) const;

  /*
  ** Training
  */
  void trainStochasticBeginImpl();
  void trainStochasticExampleImpl(FeatureGeneratorPtr gradient, double weight);
  void trainStochasticExampleImpl(ObjectPtr example);
  void trainStochasticEndImpl();

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

  /*
  ** LearningMachine
  */
  virtual void trainStochasticBegin()
    {trainStochasticBeginImpl();}

  void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
    {trainStochasticExampleImpl(gradient, weight);}

  virtual void trainStochasticExample(ObjectPtr example)
  {
    if (!inputDictionary)
      inputDictionary = getInputDictionaryFromExample(example);
    if (!parameters && inputDictionary)
      createParameters(inputDictionary, initializeParametersRandomly);
    trainStochasticExampleImpl(example);
  }

  virtual void trainStochasticEnd()
    {trainStochasticEndImpl();}

  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
    {return trainBatchImpl(examples, progress);}

  /*
  ** Serialization
  */
  virtual void save(OutputStream& ostr) const
    {BaseClass::save(ostr); saveImpl(ostr);}

  virtual bool load(InputStream& istr)
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
class GradientBasedRegressor : public GradientBasedLearningMachine_<Regressor, RegressionExample>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

  virtual double predict(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

inline GradientBasedRegressorPtr loadGradientBasedRegressor(const File& file)
  {return Object::loadFromFileAndCast<GradientBasedRegressor>(file);}

extern GradientBasedRegressorPtr leastSquaresLinearRegressor(GradientBasedLearnerPtr learner, double l2Regularizer = 0.0);

/*
** Classification
*/
class GradientBasedClassifier : public GradientBasedLearningMachine_<Classifier, ClassificationExample>
{
public:
  virtual VectorArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr input) const
  {
    if (parameters)
      return getPredictionArchitecture()->compute(parameters, input)->toDenseVector();
    else
      return new DenseVector(new FeatureDictionary(T("classes"), getLabels(), StringDictionaryPtr()), getNumLabels());
  }

  virtual double predictScore(const FeatureGeneratorPtr input, size_t output) const
    {return getPredictionArchitecture()->compute(parameters, input, output);}

  virtual FeatureDictionaryPtr getInputDictionary() const
    {return GradientBasedLearningMachine::getInputDictionary();}
};

inline GradientBasedClassifierPtr loadGradientBasedClassifier(const File& file)
  {return Object::loadFromFileAndCast<GradientBasedClassifier>(file);}

extern GradientBasedClassifierPtr maximumEntropyClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels, double l2regularizer = 0.0);

/*
** Binary Classification
*/
class GradientBasedBinaryClassifier : public GradientBasedLearningMachine_<BinaryClassifier, ClassificationExample>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

  virtual double predictScoreOfPositiveClass(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}

  virtual FeatureDictionaryPtr getInputDictionary() const
    {return GradientBasedLearningMachine::getInputDictionary();}
};

inline GradientBasedBinaryClassifierPtr loadGradientBasedBinaryClassifier(const File& file)
  {return Object::loadFromFileAndCast<GradientBasedBinaryClassifier>(file);}

extern GradientBasedBinaryClassifierPtr linearSVMBinaryClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels);
extern GradientBasedBinaryClassifierPtr logisticRegressionBinaryClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels, double l2regularizer = 0.0);

/*
** Generalized Classification
*/
class GradientBasedGeneralizedClassifier
  : public GradientBasedLearningMachine_<GeneralizedClassifier, GeneralizedClassificationExample>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

  virtual double predictScore(const FeatureGeneratorPtr input) const
    {return parameters ? getPredictionArchitecture()->compute(parameters, input) : 0.0;}
};

extern GradientBasedGeneralizedClassifierPtr linearGeneralizedClassifier(GradientBasedLearnerPtr learner);

/*
** Ranker
*/
class GradientBasedRanker
  : public GradientBasedLearningMachine_<Ranker, RankingExample>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary->getSubDictionary(0), initializeRandomly);}

  virtual double predictScore(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

inline GradientBasedRankerPtr loadGradientBasedRanker(const File& file)
  {return Object::loadFromFileAndCast<GradientBasedRanker>(file);}

extern GradientBasedRankerPtr largeMarginAllPairsLinearRanker(GradientBasedLearnerPtr learner);
extern GradientBasedRankerPtr largeMarginMostViolatedPairLinearRanker(GradientBasedLearnerPtr learner);
extern GradientBasedRankerPtr largeMarginBestAgainstAllLinearRanker(GradientBasedLearnerPtr learner);
extern GradientBasedRankerPtr logBinomialAllPairsLinearRanker(GradientBasedLearnerPtr learner);

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNING_MACHINE_H_
