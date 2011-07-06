/*-----------------------------------------.---------------------------------.
| Filename: LibLinearBinaryClassifier.h       | Lib Linear Binary Classifier    |
| Author  : Becker Julien                  |                                 |
| Started : 05/07/2011 18:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_LIBLINEAR_BINARY_CLASSIFIER_H_
# define LBCPP_LEARNING_NUMERICAL_LIBLINEAR_BINARY_CLASSIFIER_H_

# include "LibLinearClassifier.h"

namespace lbcpp
{

class LibLinearBinaryClassifier : public LibLinearClassifier
{
public:
  LibLinearBinaryClassifier(double C, LibLinearSolverType solverType)
      : LibLinearClassifier(C, solverType) {}
  LibLinearBinaryClassifier() {}
  
  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}
  
  virtual int getSupervision(const ObjectPtr& example) const
  {
    Variable supervision = example->getVariable(1);
    if (supervision.isBoolean())
      return supervision.getBoolean() ? 1 : 0;
    return supervision.getDouble() > 0.5 ? 1 : 0;
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    if (!model)
      return Variable();
    
    DoubleVectorPtr input = inputs[0].getObjectAndCast<DoubleVector>();
    // predict probabilities
    std::vector<double> probs(get_nr_class(model));
    struct feature_node* node = convertDoubleVector(input);
    Variable res;
    if (check_probability_model(model))
    {
      predict_probability(model, &node[0], &probs[0]);
      // reorder classes and return probabilities
      std::vector<int> labelIndices(get_nr_class(model));
      get_labels(model, &labelIndices[0]);
      
      res = probability(probs[labelIndices[1]]);
    }
    else
    {
      predict_values(model, &node[0], &probs[0]);
      res = convertToProbability(probs[0]);
    }
    delete [] node;
    return res;
  }
  
protected:
  Variable convertToProbability(double score) const
  {
    static const double temperature = 1.0;
    return Variable(1.0 / (1.0 + exp(-score * temperature)), probabilityType);
  }
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LIBLINEAR_BINARY_CLASSIFIER_H_
