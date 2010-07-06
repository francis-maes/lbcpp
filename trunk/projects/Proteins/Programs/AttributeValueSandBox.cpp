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


class PrintObjectVisitor : public ObjectVisitor
{
public:
  PrintObjectVisitor(ObjectPtr object, std::ostream& ostr, int maxDepth = -1)
    : currentClasses(1, object->getClass()), ostr(ostr), maxDepth(maxDepth) {jassert(currentClasses[0]);}
  
  virtual void visit(size_t variableNumber, const Variable& value)
  {
    printVariable(variableNumber, value);
    if (value.isObject())
    {
      ObjectPtr object = value.getObject();
      if (object && (maxDepth < 0 || (int)currentClasses.size() < maxDepth))
      {
        currentClasses.push_back(object->getClass());
        object->accept(ObjectVisitorPtr(this));
        currentClasses.pop_back();
      }
    }
  }

  static void print(ObjectPtr object, std::ostream& ostr, int maxDepth = -1)
    {object->accept(new PrintObjectVisitor(object, ostr, maxDepth));}

protected:
  std::vector<TypePtr> currentClasses;
  TypePtr type;
  std::ostream& ostr;
  int maxDepth;

  void printVariable(size_t variableNumber, const Variable& value)
  {
    TypePtr currentClass = currentClasses.back();
    jassert(currentClass);
    for (size_t i = 0; i < currentClasses.size() - 1; ++i)
      ostr << "  ";
    ostr << "[" << variableNumber << "] " << value.getTypeName();
    if (currentClass && variableNumber < currentClass->getNumStaticVariables())
      ostr << " " << currentClass->getStaticVariableName(variableNumber);
    ostr << " = " << value.toString() << std::endl;
  }
};

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

// FIXME: support for empty labels (empty variable values in Vectors)

ProteinPtr convertProtein(ProteinObjectPtr protein)
{
  ProteinPtr res = new Protein(protein->getName());
  res->setPrimaryStructure(convertLabelSequence(protein->getAminoAcidSequence(), aminoAcidTypeEnumeration()));
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

class ProteinResidueInputAttributes : public Object
{
public:
  ProteinResidueInputAttributes(ResiduePtr residue, size_t windowSize)
    : residue(residue), windowSize(windowSize)
  {
    variables.push_back(Variable(residue->getAminoAcidType(), aminoAcidTypeEnumeration()));

    ResiduePtr prev = residue->getPrevious();
    for (size_t i = 0; i < windowSize; ++i, prev = prev ? prev->getPrevious() : ResiduePtr())
      variables.push_back(prev ? Variable(prev->getAminoAcidType(), aminoAcidTypeEnumeration()) : Variable());

    ResiduePtr next = residue->getNext();
    for (size_t i = 0; i < windowSize; ++i, next = next ? next->getNext() : ResiduePtr())
      variables.push_back(next ? Variable(next->getAminoAcidType(), aminoAcidTypeEnumeration()) : Variable());
  }

  virtual Variable getVariable(size_t index) const
    {jassert(index < variables.size()); return variables[index];}

private:
  ResiduePtr residue;
  size_t windowSize;
  std::vector<Variable> variables;
};

class ProteinResidueInputAttributesClass : public Class
{
public:
  ProteinResidueInputAttributesClass(size_t windowSize)
    : Class(T("ProteinResidueInputAttributes"), objectClass())
  {
    addVariable(aminoAcidTypeEnumeration(), T("AA[i]"));
    for (size_t i = 0; i < windowSize; ++i)
      addVariable(aminoAcidTypeEnumeration(), T("AA[i - ") + lbcpp::toString(i + 1) + T("]"));
    for (size_t i = 0; i < windowSize; ++i)
      addVariable(aminoAcidTypeEnumeration(), T("AA[i + ") + lbcpp::toString(i + 1) + T("]"));
  }
};

/////////////////////////////////////////

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();
  Class::declare(new ProteinResidueInputAttributesClass(8));

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
  
  File workingDirectory(T("C:\\Projets\\LBC++\\projects\\temp"));
  ObjectContainerPtr oldStyleProteins = loadProteins(workingDirectory.getChildFile(T("L50DB")));
  
  // convert proteins
  VectorPtr proteins = convertProteins(oldStyleProteins);
  std::cout << proteins->size() << " proteins" << std::endl;
  //PrintObjectVisitor::print(proteins->getVariable(2), std::cout, 3);

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
      Variable input(new ProteinResidueInputAttributes(protein->getResidue(j), 8));
      Variable output = secondaryStructure->getVariable(j);
      secondaryStructureExamples->append(Variable::pair(input, output));
    }
  }

  // make train and test set
  VariableContainerPtr trainingData = secondaryStructureExamples->fold(0, 2);
  VariableContainerPtr testingData = secondaryStructureExamples->fold(1, 2);
  std::cout << "Training Data: " << trainingData->size() << " Testing Data: " << testingData->size() << std::endl;

  // train
  InferencePtr inference = extraTreeInference(T("SS3"), 200, 1);
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
