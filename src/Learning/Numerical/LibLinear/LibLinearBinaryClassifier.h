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
  LibLinearBinaryClassifier(double C, LibLinearClassificationType solverType)
      : LibLinearClassifier(C, solverType) {}
  LibLinearBinaryClassifier() {}
  
  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}
  
  virtual double getSupervision(const ObjectPtr& example) const
  {
    Variable supervision = example->getVariable(1);
    return supervision.getDouble();
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
    predict_probability(model, &node[0], &probs[0]);
    delete [] node;

    // reorder classes and return probabilities
    std::vector<int> labelIndices(get_nr_class(model));
    get_labels(model, &labelIndices[0]);
    return probability(probs[labelIndices[1]]);
  }
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LIBLINEAR_BINARY_CLASSIFIER_H_
