/*-----------------------------------------.---------------------------------.
| Filename: SandBoxNewFeatures.cpp         | New Feature Generators          |
| Author  : Francis Maes                   |                                 |
| Started : 20/08/2010 19:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Data/Protein.h" 
#include "Inference/ProteinInferenceFactory.h"
#include "Inference/ProteinInference.h"
#include "Inference/ContactMapInference.h"
#include "Perception/PerceptionToFeatures.h"
#include "Evaluator/ProteinEvaluator.h"
using namespace lbcpp;

extern void declareProteinClasses();


///////////////////////////////////////////////////////////////////////

// VectorMaths.h
namespace lbcpp
{

// Const-unary operations
extern size_t l0norm(ObjectPtr object);
extern size_t l0norm(PerceptionPtr perception, const Variable& input);

extern double l1norm(ObjectPtr object);
extern double l1norm(PerceptionPtr perception, const Variable& input);

extern double sumOfSquares(ObjectPtr object);
extern double sumOfSquares(PerceptionPtr perception, const Variable& input);

inline double l2norm(ObjectPtr object)
  {return sqrt(sumOfSquares(object));}

inline double l2norm(PerceptionPtr perception, const Variable& input)
  {return sqrt(sumOfSquares(perception, input));}

// Const-binary operations
extern double dotProduct(ObjectPtr object, PerceptionPtr perception, const Variable& input);

};

///////////////////////////////////////////////////////////////////////
// VectorMaths.cpp

/*
** DoubleConstUnaryOperation
*/
struct DoubleConstUnaryOperation
{
  void sense(double value)
    {jassert(false);}
  void sense(PerceptionPtr perception, const Variable& input)
    {jassert(false);}
  void sense(ObjectPtr object)
    {jassert(false);}
};

template<class OperationType>
void doubleConstUnaryOperation(OperationType& operation, ObjectPtr object)
{
  size_t n = object->getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable v = object->getVariable(i);
    if (v.isMissingValue())
      continue;

    if (v.isObject())
      operation.sense(v.getObject());
    else
    {
      jassert(v.isDouble());
      operation.sense(v.getDouble());
    }
  }
}

template<class OperationType>
struct DoubleConstUnaryOperationCallback : public PerceptionCallback
{
  DoubleConstUnaryOperationCallback(OperationType& operation)
    : operation(operation) {}

  OperationType& operation;

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    jassert(value.isDouble() && !value.isMissingValue());
    operation.sense(value.getDouble());
  }

  virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
    {operation.sense(subPerception, input);}
};

template<class OperationType>
void doubleConstUnaryOperation(OperationType& operation, PerceptionPtr perception, const Variable& input)
{
  typedef DoubleConstUnaryOperationCallback<OperationType> Callback;
  ReferenceCountedObjectPtr<Callback> callback(new Callback(operation));
  perception->computePerception(input, callback);
}

/*
** L0 Norm
*/
struct ComputeL0NormOperation : public DoubleConstUnaryOperation
{
  ComputeL0NormOperation() : res(0) {}

  size_t res;

  void sense(double value)
    {if (value) ++res;}

  void sense(PerceptionPtr perception, const Variable& input)
    {res += lbcpp::l0norm(perception, input);}

  void sense(ObjectPtr object)
    {res += lbcpp::l0norm(object);}
};

size_t lbcpp::l0norm(ObjectPtr object)
  {ComputeL0NormOperation operation; doubleConstUnaryOperation(operation, object); return operation.res;}

size_t lbcpp::l0norm(PerceptionPtr perception, const Variable& input)
  {ComputeL0NormOperation operation; doubleConstUnaryOperation(operation, perception, input); return operation.res;}

/*
** L1 Norm
*/
struct ComputeL1NormOperation : public DoubleConstUnaryOperation
{
  ComputeL1NormOperation() : res(0.0) {}

  double res;

  void sense(double value)
    {res += fabs(value);}

  void sense(PerceptionPtr perception, const Variable& input)
    {res += lbcpp::l1norm(perception, input);}

  void sense(ObjectPtr object)
    {res += lbcpp::l1norm(object);}
};

double lbcpp::l1norm(ObjectPtr object)
  {ComputeL1NormOperation operation; doubleConstUnaryOperation(operation, object); return operation.res;}

double lbcpp::l1norm(PerceptionPtr perception, const Variable& input)
  {ComputeL1NormOperation operation; doubleConstUnaryOperation(operation, perception, input); return operation.res;}

/*
** Sum of squares
*/
struct ComputeSumOfSquaresOperation : public DoubleConstUnaryOperation
{
  ComputeSumOfSquaresOperation() : res(0.0) {}

  double res;

  void sense(double value)
    {res += value * value;}

  void sense(PerceptionPtr perception, const Variable& input)
    {res += lbcpp::sumOfSquares(perception, input);}

  void sense(ObjectPtr object)
    {res += lbcpp::sumOfSquares(object);}
};

double lbcpp::sumOfSquares(ObjectPtr object)
  {ComputeSumOfSquaresOperation operation; doubleConstUnaryOperation(operation, object); return operation.res;}

double lbcpp::sumOfSquares(PerceptionPtr perception, const Variable& input)
  {ComputeSumOfSquaresOperation operation; doubleConstUnaryOperation(operation, perception, input); return operation.res;}

/*
** Dot-product
*/
class StackBasedPerceptionCallback : public PerceptionCallback
{
public:
  StackBasedPerceptionCallback(ObjectPtr currentObject)
    : currentObject(currentObject) {}
  
protected:
  ObjectPtr getCurrentObject() const
    {return currentObject;}

  void pushObject(ObjectPtr object)
    {stack.push_back(currentObject); currentObject = object;}

  void popObject()
    {jassert(stack.size()); currentObject = stack.back(); stack.pop_back();}

private:
  ObjectPtr currentObject;
  std::vector<ObjectPtr> stack;
};

struct ComputeDotProductCallback : public StackBasedPerceptionCallback
{
  ComputeDotProductCallback(ObjectPtr object)
    : StackBasedPerceptionCallback(object), res(0.0) {}

  double res;

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    jassert(value.isDouble() && !value.isMissingValue());
    res += getCurrentObject()->getVariable(variableNumber).getDouble() * value.getDouble();
  }

  virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
    {res += dotProduct(getCurrentObject(), subPerception, input);}
};

double lbcpp::dotProduct(ObjectPtr object, PerceptionPtr perception, const Variable& input)
{
  ReferenceCountedObjectPtr<ComputeDotProductCallback> callback(new ComputeDotProductCallback(object));
  perception->computePerception(input, callback);
  return callback->res;
}


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();
  
  File workingDirectory(T("C:\\Projets\\LBC++\\projects\\temp"));
  Variable protein = Variable::createFromFile(workingDirectory.getChildFile(T("PDB30Medium/1A3A.xml")));
  if (!protein)
    return 1;

  
  ProteinInferenceFactory factory;
  PerceptionPtr perception = factory.createResiduePerception(String::empty);
  perception = perceptionToFeatures(perception);
  Variable input = Variable::pair(protein, 3);

  Variable parameters = Variable::create(perception->getOutputType());
  std::cout << "Parameters: " << std::endl;
  parameters.printRecursively(std::cout);

  Variable output = perception->compute(input);
  std::cout << "Features: " << std::endl;
  output.printRecursively(std::cout, 1);

  std::cout << "L0: " << l0norm(perception, input) << " " << l0norm(output) << std::endl;
  std::cout << "L1: " << l1norm(perception, input) << " " << l1norm(output) << std::endl;
  std::cout << "L2: " << l2norm(perception, input) << " " << l2norm(output) << std::endl;
  std::cout << "Dot Product: " << dotProduct(output, perception, input) << std::endl;


/*
  parameters.addWeighted(output, -0.1);
  std::cout << "Parameters: " << std::endl;
  parameters.printRecursively(std::cout);

  parameters.addWeighted(output, +0.1);
  std::cout << "Parameters: " << std::endl;
  parameters.printRecursively(std::cout);*/


  return 0;
}
