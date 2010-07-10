/*-----------------------------------------.---------------------------------.
| Filename: AttributeValueSandBox.cpp      | Attribute Value Representation  |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 11:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Protein/Data/Protein.h" 
using namespace lbcpp;

extern void declareLBCppCoreClasses();
extern void declareProteinClasses();

void printVariableLine(const Variable& value, std::ostream& ostr, size_t variableNumber, const String& name, int currentDepth)
{
  for (int i = 0; i < currentDepth; ++i)
    ostr << "  ";
  if (variableNumber != (size_t)-1)
    ostr << "[" << variableNumber << "] ";
  ostr << value.getTypeName();
  if (name.isNotEmpty())
    ostr << " " << name;
  String v = value.toString();
  if (v.length() > 30)
    v = v.substring(0, 30) + T("...");
  ostr << " = " << v << std::endl;
}

void printVariablesRecursively(const Variable& variable, std::ostream& ostr, int maxDepth, int currentDepth)
{
  if (maxDepth >= 0 && currentDepth >= maxDepth)
    return;
  TypePtr type = variable.getType();
  for (size_t i = 0; i < variable.size(); ++i)
  {
    String name;
    if (i < type->getNumStaticVariables())
      name = type->getStaticVariableName(i);
    printVariableLine(variable[i], ostr, i, name, currentDepth);
    printVariablesRecursively(variable[i], ostr, maxDepth, currentDepth + 1);
  }
}

void printVariable(const Variable& variable, std::ostream& ostr, int maxDepth = -1)
{
  printVariableLine(variable, ostr, (size_t)-1, String::empty, 0);
  printVariablesRecursively(variable, ostr, maxDepth, 1);
}

///////////////
/*
class VectorWindowAttributes : public Object
{
public:
  VectorWindowAttributes(size_t proteinVariableIndex, size_t size)
    : proteinVariableIndex(proteinVariableIndex), size(size) {}

protected:
  size_t proteinVariableIndex;
  size_t size;
};*/

class ProteinResidueInputAttributes : public Object
{
public:
  ProteinResidueInputAttributes(ProteinPtr protein, size_t position, size_t windowSize)
    : position(position), windowSize(windowSize)
  {
    primaryStructure = protein->getPrimaryStructure();
    positionSpecificScoringMatrix = protein->getPositionSpecificScoringMatrix();
  }

  virtual Variable getVariable(VectorPtr vector, size_t index) const
  {
    int targetPosition = (int)position - (int)(windowSize / 2) + (int)index;
    if (targetPosition >= 0 && targetPosition < (int)vector->size())
      return vector->getVariable(targetPosition);
    else
      return Variable::missingValue(vector->getElementsType());
  }

  virtual Variable getVariable(size_t index) const
  {
    if (index < windowSize)
      return getVariable(primaryStructure, index);
    else
    {
      index -= windowSize;
      size_t position = index / 20;
      Variable var = getVariable(positionSpecificScoringMatrix, position);
      return var ? var[index % 20] : Variable::missingValue(probabilityType());
    }
  }

private:
  VectorPtr primaryStructure;
  VectorPtr positionSpecificScoringMatrix;
  size_t position;
  size_t windowSize;
};

class ProteinResidueInputAttributesClass : public DynamicClass
{
public:
  ProteinResidueInputAttributesClass(size_t windowSize)
    : DynamicClass(T("ProteinResidueInputAttributes"), objectClass())
  {
    for (size_t i = 0; i < windowSize; ++i)
    {
      int offset = (int)i - (int)(windowSize / 2);
      addVariable(aminoAcidTypeEnumeration(), T("AA[") + String(offset) + T("]"));
    }
    for (size_t i = 0; i < windowSize; ++i)
    {
      int offset = (int)i - (int)(windowSize / 2);
      for (size_t j = 0; j < 20; ++j)
        addVariable(probabilityType(), T("PSSM[") + String(offset) + T("][") + String((int)j) + T("]"));
    }
  }
};

//////////////////////////////////////////

class AddVariablesToDynamicObjectVisitor : public ObjectVisitor
{
public:
  AddVariablesToDynamicObjectVisitor(DynamicObjectPtr target)
    : target(target) {}

  virtual void visit(size_t variableNumber, const Variable& value)
    {target->setVariable(variableNumber, value);}

private:
  DynamicObjectPtr target;
};

class Representation : public Function
{
public:
  virtual void getStaticVariables(DynamicClassPtr res) const = 0;
  virtual void compute(const Variable& input, ObjectVisitorPtr visitor) const = 0;
  
  // Function
  virtual TypePtr getOutputType(TypePtr inputType) const
  {
    const_cast<Representation* >(this)->ensureTypeIsComputed();
    return type;
  }

  virtual Variable compute(const Variable& input) const
  {
    const_cast<Representation* >(this)->ensureTypeIsComputed();
    DynamicObjectPtr res = new DynamicObject(type);
    compute(input, ObjectVisitorPtr(new AddVariablesToDynamicObjectVisitor(res)));
    return res;
  }

private:
  DynamicClassPtr type;

  void ensureTypeIsComputed()
  {
    if (type)
      return;
    type = new DynamicClass(getClassName() + T("Class"));
    getStaticVariables(type);
  }
};

typedef ReferenceCountedObjectPtr<Representation> RepresentationPtr;

class DecoratorRepresentation : public Representation
{
public:
  DecoratorRepresentation(RepresentationPtr decorated)
    : decorated(decorated) {}

  virtual void getStaticVariables(DynamicClassPtr res) const
    {decorated->getStaticVariables(res);}

  virtual void compute(const Variable& input, ObjectVisitorPtr visitor) const
    {decorated->compute(input, visitor);}

protected:
  RepresentationPtr decorated;
};

class VectorWindowRepresentation : public Representation
{
public:
  VectorWindowRepresentation(const String& vectorName, TypePtr vectorType, size_t windowSize)
    : vectorName(vectorName), vectorType(vectorType), windowSize(windowSize) {}

  virtual TypePtr getInputType() const
    {return pairType(vectorType, integerType());}

  virtual void getStaticVariables(DynamicClassPtr res) const
  {
    TypePtr type = vectorType->getTemplateArgument(0);
    jassert(type);
    int relativePosition = -(int)(windowSize / 2);
    for (size_t i = 0; i < windowSize; ++i)
      res->addVariable(type, vectorName + T("[") + String(relativePosition++) + T("]"));
  }

  virtual void compute(const Variable& input, ObjectVisitorPtr visitor) const
  {
    ContainerPtr container = input[0].getObjectAndCast<Container>();
    int position = input[1].getInteger() - (int)(windowSize / 2);
    for (size_t i = 0; i < windowSize; ++i, ++position)
    {
      if (position >= 0 && position < (int)container->size())
        visitor->visit(i, container->getVariable(position));
      else
        visitor->visit(i, Variable());
    }
  }

protected:
  String vectorName;
  TypePtr vectorType;
  size_t windowSize;
};

class CompositeRepresentation : public Representation
{
public:

protected:
  std::vector<RepresentationPtr> subRepresentations;
  std::map<size_t, size_t> subIndexToIndexMap;
};

///////////////////////////////////////// 

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();
  Class::declare(new ProteinResidueInputAttributesClass(13));
  
  File workingDirectory(T("C:\\Projets\\LBC++\\projects\\temp"));
  //File workingDirectory(T("/Users/francis/tmp"));
  //ObjectContainerPtr oldStyleProteins = loadProteins(workingDirectory.getChildFile(T("SmallPDB\\protein")));
  
  // convert proteins
  VectorPtr proteins;// = convertProteins(oldStyleProteins);
  //oldStyleProteins = ObjectContainerPtr();
  return 0; // FIXME
  std::cout << proteins->size() << " proteins" << std::endl;

  //PrintObjectVisitor::print(proteins->getVariable(2), std::cout, 2);

  // make secondary structure classification examples
  VectorPtr secondaryStructureExamples = new Vector(pairType(
      Class::get(T("ProteinResidueInputAttributes")),
      secondaryStructureElementEnumeration()));
  for (size_t i = 0; i < proteins->size(); ++i)
  {
    ProteinPtr protein = proteins->getVariable(i).getObjectAndCast<Protein>();
    jassert(protein);
    size_t n = protein->getLength();
    VectorPtr secondaryStructure = protein->getSecondaryStructure();
    for (size_t j = 0; j < n; ++j)
    {
      Variable input(new ProteinResidueInputAttributes(protein, j, 13));
      Variable output = secondaryStructure->getVariable(j);
      secondaryStructureExamples->append(Variable::pair(input, output));
    }
  }

  printVariable(secondaryStructureExamples->getVariable(10), std::cout, 3);

  // make train and test set
  ContainerPtr data = secondaryStructureExamples->randomize();
  ContainerPtr trainingData = data->fold(0, 2);
  ContainerPtr testingData = data->fold(1, 2);
  std::cout << "Training Data: " << trainingData->size() << " Testing Data: " << testingData->size() << std::endl;

  // train
  InferencePtr inference = extraTreeInference(T("SS3"), 2, 15);
  InferenceContextPtr context = singleThreadedInferenceContext();
  context->train(inference, trainingData);

  // evaluate
  EvaluatorPtr evaluator = classificationAccuracyEvaluator(T("SS3"));
  context->evaluate(inference, trainingData, evaluator);
  std::cout << "Train: " << evaluator->toString() << std::endl;

  evaluator = classificationAccuracyEvaluator(T("SS3"));
  context->evaluate(inference, testingData, evaluator);
  std::cout << "Test: " << evaluator->toString() << std::endl;
  return 0;
}
