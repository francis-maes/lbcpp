/*-----------------------------------------.---------------------------------.
| Filename: ParameterizedInference.h       | Parameterized Inference         |
| Author  : Francis Maes                   |   base class                    |
| Started : 27/05/2010 21:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_PARAMETERIZED_H_
# define LBCPP_INFERENCE_PARAMETERIZED_H_

# include "Inference.h"
# include "InferenceVisitor.h"
# include "InferenceContext.h"
# include "InferenceCallback.h"
# include "../FeatureGenerator/FeatureGenerator.h"
# include "../FeatureGenerator/DenseVector.h"
# include "../LearningMachine.h"
# include "../Object/ObjectPair.h"

namespace lbcpp
{

/*
** ParameterizedInference
*/
class ParameterizedInference : public Inference
{
public:
  ParameterizedInference(const String& name) : Inference(name) {}
  ParameterizedInference() {}

  virtual void accept(InferenceVisitorPtr visitor);

  virtual FeatureGeneratorPtr getExampleGradient(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput, double& exampleLossValue) = 0;

  DenseVectorPtr getParameters() const
    {return parameters;}
 
  void setParameters(DenseVectorPtr parameters)
    {this->parameters = parameters; validateParametersChange();}

  virtual ObjectPtr clone() const;

  virtual void validateParametersChange() {}

protected:
  DenseVectorPtr parameters;

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};


// Input: FeatureGenerator
// Output: FeatureVector
// Supervision: Label
class ClassificationInferenceStep : public Inference
{
public:
  ClassificationInferenceStep(const String& name)
    : Inference(name) {}
  ClassificationInferenceStep() {}

  virtual String toString() const
  {
    String res = getClassName();
    if (labels)
      res += T("(") + labels->getName() + T(")");
    return res;
  }

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(ClassificationInferenceStepPtr(this));}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runClassification(ClassificationInferenceStepPtr(this), input, supervision, returnCode);}

  ClassifierPtr getClassifier() const
    {return classifier;}

  void setClassifier(ClassifierPtr classifier)
    {this->classifier = classifier;}

  void setLabels(FeatureDictionaryPtr labels)
    {this->labels = labels;}

  FeatureDictionaryPtr getLabels() const
    {return labels;}

protected:
  FeatureDictionaryPtr labels;
  ClassifierPtr classifier;

  virtual bool load(InputStream& istr)
    {return Inference::load(istr) && lbcpp::read(istr, classifier);}

  virtual void save(OutputStream& ostr) const
    {Inference::save(ostr); lbcpp::write(ostr, classifier);}
};

// Input: FeatureGenerator
// Output: Scalar
// Supervision: Scalar
class RegressionInferenceStep : public Inference
{
public:
  RegressionInferenceStep(const String& name)
    : Inference(name) {}
  RegressionInferenceStep() {}

  virtual String toString() const
    {return getClassName();}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(RegressionInferenceStepPtr(this));}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runRegression(RegressionInferenceStepPtr(this), input, supervision, returnCode);}

  RegressorPtr getRegressor() const
    {return regressor;}

  void setRegressor(RegressorPtr regressor)
    {this->regressor = regressor;}

protected:
  RegressorPtr regressor;

  virtual bool load(InputStream& istr)
    {return Inference::load(istr) && lbcpp::read(istr, regressor);}

  virtual void save(OutputStream& ostr) const
    {Inference::save(ostr); lbcpp::write(ostr, regressor);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_PARAMETERIZED_H_
