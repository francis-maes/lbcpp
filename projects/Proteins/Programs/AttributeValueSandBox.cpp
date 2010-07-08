/*-----------------------------------------.---------------------------------.
| Filename: AttributeValueSandBox.cpp      | Attribute Value Representation  |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 11:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Protein/ProteinObject.h" // old
#include "Protein/Data/Protein.h" // new
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

ObjectContainerPtr loadProteins(const File& directory, size_t maxCount = 0)
{
  ObjectStreamPtr proteinsStream = directoryObjectStream(directory, T("*.protein"));
#ifdef JUCE_DEBUG
  ObjectContainerPtr res = proteinsStream->load(maxCount ? maxCount : 7)->randomize();
#else
  ObjectContainerPtr res = proteinsStream->load(maxCount)->randomize();
#endif
  for (size_t i = 0; i < res->size(); ++i)
  {
    ProteinObjectPtr protein = res->getAndCast<ProteinObject>(i);
    jassert(protein);
    protein->computeMissingFields();
  }
  return res;
}

///////////////

static VectorPtr convertLabelSequence(LabelSequencePtr sequence, EnumerationPtr targetType)
{
  if (!sequence)
    return VectorPtr();
  size_t n = sequence->size();
  VectorPtr res = new Vector(targetType, n);
  for (size_t i = 0; i < n; ++i)
    if (sequence->hasObject(i))
      res->setVariable(i, Variable((int)sequence->getIndex(i), targetType));
  return res;
}

static VectorPtr convertScoreVectorSequence(ScoreVectorSequencePtr sequence, EnumerationPtr targetType)
{
  if (!sequence)
    return VectorPtr();

  size_t n = sequence->size();
  size_t numScores = sequence->getNumScores();

  VectorPtr res = new Vector(discreteProbabilityDistributionClass(targetType), n);
  for (size_t i = 0; i < n; ++i)
    if (sequence->hasObject(i))
    {
      DiscreteProbabilityDistributionPtr distribution = new DiscreteProbabilityDistribution(targetType);
      for (size_t j = 0; j < numScores; ++j)
        distribution->setVariable(j, sequence->getScore(i, j));
      res->setVariable(i, distribution);
    }
  return res;
}

// FIXME: support for empty labels (empty variable values in Vectors)

ProteinPtr convertProtein(ProteinObjectPtr protein)
{
  ProteinPtr res = new Protein(protein->getName());
  res->setPrimaryStructure(convertLabelSequence(protein->getAminoAcidSequence(), aminoAcidTypeEnumeration()));
  res->setPositionSpecificScoringMatrix(convertScoreVectorSequence(protein->getPositionSpecificScoringMatrix(), aminoAcidTypeEnumeration()));

  res->setSecondaryStructure(convertLabelSequence(protein->getSecondaryStructureSequence(), secondaryStructureElementEnumeration()));
  res->setDSSPSecondaryStructure(convertLabelSequence(protein->getDSSPSecondaryStructureSequence(), dsspSecondaryStructureElementEnumeration()));
  // FIXME: the rest ...

  res->computeMissingVariables();
  return res;
}

VectorPtr convertProteins(ObjectContainerPtr oldStyleProteins)
{
  VectorPtr proteins = new Vector(proteinClass(), oldStyleProteins->size());
  for (size_t i = 0; i < proteins->size(); ++i)
  {
    ProteinObjectPtr oldStyleProtein = oldStyleProteins->getAndCast<ProteinObject>(i);
    jassert(oldStyleProtein);
    ProteinPtr protein = convertProtein(oldStyleProtein);
    jassert(protein);
    proteins->setVariable(i, protein);
  }
  return proteins;
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
      return Variable::missingValue(vector->getStaticType());
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

class Function : public Object
{
public:
  virtual TypePtr getInputType() const = 0;
  virtual TypePtr getOutputType(TypePtr inputType) const = 0;
  virtual Variable compute(const Variable& input) const = 0;
};

typedef ReferenceCountedObjectPtr<Function> FunctionPtr;

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
    VariableContainerPtr container = input[0].getObjectAndCast<VariableContainer>();
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

class A : public Object
{
public:
  virtual String toString() const
    {return T("aaa  ");}
};

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();
  Class::declare(new ProteinResidueInputAttributesClass(13));
  LBCPP_DECLARE_CLASS(A, Object);

  Variable myBoolean(true);
  Variable myMissingBoolean = Variable::missingValue(booleanType());
  Variable myInt(51);
  Variable myMissingInt = Variable::missingValue(integerType());
  Variable myDouble(8.6);
  Variable myMissingDouble = Variable::missingValue(doubleType());
  Variable myString(T("Hello"));
  Variable myMissingString = Variable::missingValue(stringType());
  Variable myObject(new A());
  Variable myMissingObject = Variable::missingValue(Class::get(T("A")));
  Variable myPair = Variable::pair(myBoolean, myObject);
  Variable myMissingPair = Variable::missingValue(pairType(booleanType(), Class::get(T("A"))));
  
  std::cout << myBoolean << " " << myMissingBoolean << std::endl
            << myInt << " " << myMissingInt << std::endl
            << myDouble << " " << myMissingDouble << std::endl
            << myString << " " << myMissingString << std::endl
            << myObject << " " << myMissingObject << std::endl
            << myPair << " " << myMissingPair << std::endl;
      
  std::cout << myBoolean.isMissingValue() << " " << myMissingBoolean.isMissingValue() << std::endl
            << myInt.isMissingValue() << " " << myMissingInt.isMissingValue() << std::endl
            << myDouble.isMissingValue() << " " << myMissingDouble.isMissingValue() << std::endl
            << myString.isMissingValue() << " " << myMissingString.isMissingValue() << std::endl
            << myObject.isMissingValue() << " " << myMissingObject.isMissingValue() << std::endl
            << myPair.isMissingValue() << " " << myMissingPair.isMissingValue() << std::endl;
  

  /*
  Variable myEnumValue(asparticAcid, aminoAcidTypeEnumeration());
  std::cout << myEnumValue << std::endl;

  Variable container = Variable::pair(16.64, 51);
  std::cout << "pair: " << container << " size: " << container.size() << std::endl;
  for (size_t i = 0; i < container.size(); ++i)
    std::cout << "  elt " << i << " = " << container[i].toString() << " (type = " << container[i].getType()->getName() << ")" << std::endl;
  Variable containerCopy = container;
  std::cout << "container copy: " << containerCopy << " (type = "
    << containerCopy.getType()->getName() << " equals: " << Variable(container == containerCopy) << std::endl;
  */
  
//  File workingDirectory(T("C:\\Projets\\LBC++\\projects\\temp"));
  File workingDirectory(T("/Users/francis/tmp"));
  ObjectContainerPtr oldStyleProteins = loadProteins(workingDirectory.getChildFile(T("SmallPDB\\protein")));
  
  // convert proteins
  VectorPtr proteins = convertProteins(oldStyleProteins);
  oldStyleProteins = ObjectContainerPtr();
  std::cout << proteins->size() << " proteins" << std::endl;
  proteins->getVariable(0).saveToFile(workingDirectory.getChildFile(T("NewProt.xml")));
  return 0;
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
  VariableContainerPtr data = secondaryStructureExamples->randomize();
  VariableContainerPtr trainingData = data->fold(0, 2);
  VariableContainerPtr testingData = data->fold(1, 2);
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
