/*-----------------------------------------.---------------------------------.
| Filename: LibLinearClassifier.h          | Classifier of LibLinear library |
| Author  : Becker Julien                  |                                 |
| Started : 05/07/2011 16:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_LIBLINEAR_CLASSIFIER_H_
# define LBCPP_LEARNING_NUMERICAL_LIBLINEAR_CLASSIFIER_H_

# include "LibLinearLearningMachine.h"

namespace lbcpp
{

class LibLinearClassifier : public LibLinearLearningMachine
{
public:
  LibLinearClassifier(double C, LibLinearSolverType solverType)
    : LibLinearLearningMachine(C, solverType) {}
  LibLinearClassifier() {}

  virtual TypePtr getSupervisionType() const
    {return sumType(denseDoubleVectorClass(), enumValueType);}

  virtual struct feature_node* getInput(const ObjectPtr& example) const
    {return convertDoubleVector(example->getVariable(0).getObjectAndCast<DoubleVector>());}

  virtual int getSupervision(const ObjectPtr& example) const
  {
    Variable supervision = example->getVariable(1);
    if (supervision.isEnumeration())
      return supervision.getInteger();
    else
    {
      DoubleVectorPtr vector = supervision.dynamicCast<DoubleVector>();
      if (vector)
        return (int)vector->getIndexOfMaximumValue();
      else
      {
        jassert(false);
        return 0;
      }
    }
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    TypePtr supervisionType = inputVariables[1]->getType();
    if (supervisionType.isInstanceOf<Enumeration>())
      labels = supervisionType.staticCast<Enumeration>();
    else
    {
      labels = DoubleVector::getElementsEnumeration(supervisionType);
      jassert(labels);
    }
    return denseDoubleVectorClass(labels, probabilityType);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    if (!model)
      return Variable();

    DoubleVectorPtr input = inputs[0].getObjectAndCast<DoubleVector>();

    // predict probabilities
    std::vector<double> probs(get_nr_class(model));
    struct feature_node* node = convertDoubleVector(input);
    
    if (check_probability_model(model))
    {
      predict_probability(model, &node[0], &probs[0]);
    }
    else 
    {
      predict_values(model, &node[0], &probs[0]);
      double sum = 0.0;
      for (size_t i = 0; i < probs.size(); ++i)
        sum += probs[i];
      if (sum > 0.0 + DBL_EPSILON)
        for (size_t i = 0; i < probs.size(); ++i)
          probs[i] /= sum;
    }
    delete [] node;

    // reorder classes and return probabilities
    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType());
    std::vector<int> labelIndices(get_nr_class(model));
    get_labels(model, &labelIndices[0]);
    for (int i = 0; i < get_nr_class(model); ++i)
      res->setValue(labelIndices[i], probs[i]);
    return res;
  }

protected:
  EnumerationPtr labels;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LIBLINEAR_CLASSIFIER_H_
