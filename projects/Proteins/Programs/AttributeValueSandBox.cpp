/*-----------------------------------------.---------------------------------.
| Filename: AttributeValueSandBox.lcpp     | Attribute Value Representation  |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 11:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Protein/Protein.h"
#include "Protein/AminoAcid.h"
using namespace lbcpp;

extern void declareLBCppCoreClasses();
extern void declareProteinClasses();


class A : public Object
{
public:
  virtual String toString() const
    {return T("Hello A");}

  enum
  {
    anintVariable = 0,
    adoubleVariable,
    astringVariable,
    anobjectVariable,
  };

  virtual void accept(ObjectVisitorPtr visitor)
  {
    visitor->visitVariable(anintVariable, 51);
    visitor->visitVariable(adoubleVariable, 16.64);
    visitor->visitVariable(astringVariable, T("Hello world"));
    visitor->visitVariable(anobjectVariable, ObjectPtr(this));
  }
};

typedef ReferenceCountedObjectPtr<A> APtr;

class AClass : public DefaultClass_<A>
{
public:
  AClass() : DefaultClass_<A>(ObjectClass::getInstance())
  {
    addVariable(IntegerClass::getInstance(), T("anint"));
    addVariable(DoubleClass::getInstance(), T("adouble"));
    addVariable(StringClass::getInstance(), T("astring"));
    addVariable(ObjectClass::getInstance(), T("anobject"));
  }
};

////////////////////////////

class PrintObjectVisitor : public ObjectVisitor
{
public:
  PrintObjectVisitor(ObjectPtr object, std::ostream& ostr, int maxDepth = -1)
    : currentClasses(1, object->getClass()), ostr(ostr), maxDepth(maxDepth) {jassert(currentClasses[0]);}
  
  virtual void visitVariable(size_t variableNumber, bool value)
    {printVariable(variableNumber, lbcpp::toString(value));}

  virtual void visitVariable(size_t variableNumber, int value)
  {
    ClassPtr currentClass = currentClasses.back();
    jassert(currentClass);
    IntegerClassPtr variableClass = currentClass->getVariableType(variableNumber);
    jassert(variableClass);
    printVariable(variableNumber, variableClass->toString(value));
  }

  virtual void visitVariable(size_t variableNumber, double value)
    {printVariable(variableNumber, lbcpp::toString(value));}

  virtual void visitVariable(size_t variableNumber, const String& value)
    {printVariable(variableNumber, lbcpp::toString(value));}

  virtual void visitVariable(size_t variableNumber, ObjectPtr value)
  {
    if (value)
    {
      printVariable(variableNumber, value->getName());// + T(" (") + String((juce::int64)value.get()) + T(")"));
      if (maxDepth < 0 || (int)currentClasses.size() < maxDepth)
      {
        currentClasses.push_back(value->getClass());
        value->accept(ObjectVisitorPtr(this));
        currentClasses.pop_back();
      }
    }
    else
      printVariable(variableNumber, T("<null>"));
  }

  static void print(ObjectPtr object, std::ostream& ostr, int maxDepth = -1)
    {object->accept(new PrintObjectVisitor(object, ostr, maxDepth));}

protected:
  std::vector<ClassPtr> currentClasses;
  ClassPtr type;
  std::ostream& ostr;
  int maxDepth;

  void printVariable(size_t variableNumber, const String& valueAsString)
  {
    ClassPtr currentClass = currentClasses.back();
    jassert(currentClass);
    for (size_t i = 0; i < currentClasses.size() - 1; ++i)
      ostr << "  ";
    ostr << "[" << variableNumber << "] "
      << currentClass->getVariableType(variableNumber)->getName() << " "
      << currentClass->getVariableName(variableNumber) << " = "
      << valueAsString << std::endl;
  }
};
/*
int main(int argc, char** argv)
{
  Class::declare(new ObjectClass());
  Class::declare(new IntegerClass());
  Class::declare(new DoubleClass());
  Class::declare(new StringClass());
  Class::declare(new AClass());

  APtr a = Class::createInstanceAndCast<A>(T("A"));
  a->accept(new PrintObjectVisitor(std::cout));
  return 0;
}
*/

////////////////////////////////////////////

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
    ProteinPtr protein = res->getAndCast<Protein>(i);
    jassert(protein);
    protein->computeMissingFields();
  }
  return res;
}

///

class ProteinResidue;
typedef ReferenceCountedObjectPtr<ProteinResidue> ProteinResiduePtr;

class ProteinResidue : public NameableObject
{
public:
  ProteinResidue(const String& name, AminoAcidPtr aminoAcid)
    : NameableObject(name), aminoAcid(aminoAcid) {}

  AminoAcidPtr getAminoAcid() const
    {return aminoAcid;}

  AminoAcidType getAminoAcidType() const
    {jassert(aminoAcid); return aminoAcid->getType();}

  virtual void accept(ObjectVisitorPtr visitor)
  {
    visitor->visitVariable(0, (ObjectPtr)aminoAcid);
    visitor->visitVariable(1, (ObjectPtr)previous);
    visitor->visitVariable(2, (ObjectPtr)next);
  }

  void setNext(ProteinResiduePtr next)
    {this->next = next; next->previous = ProteinResiduePtr(this);}

  ProteinResiduePtr getPrevious() const
    {return previous;}

  ProteinResiduePtr getNext() const
    {return next;}

  void clear()
    {previous = next = ProteinResiduePtr();}

private:
  AminoAcidPtr aminoAcid;
  ProteinResiduePtr previous;
  ProteinResiduePtr next;
};

class ProteinResidueClass : public Class
{
public:
  ProteinResidueClass() : Class(T("ProteinResidue"), ObjectClass::getInstance())
  {
    addVariable(AminoAcid::getCollection(), T("aminoAcid"));
    addVariable(ClassPtr(this), T("previous"));
    addVariable(ClassPtr(this), T("next"));
  }
  
  virtual ClassPtr getBaseClass() const
    {return ObjectClass::getInstance();}

};

void createResidues(ProteinPtr protein, std::vector<ProteinResiduePtr>& res)
{
  LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
  jassert(aminoAcidSequence);
  size_t n = aminoAcidSequence->size();
  res.reserve(res.size() + n);

  CollectionPtr aminoAcidCollection = AminoAcid::getCollection();

  for (size_t i = 0; i < n; ++i)
  {
    AminoAcidPtr aminoAcid;
    if (aminoAcidSequence->hasObject(i))
    {
      size_t index = aminoAcidSequence->getIndex(i);
      if (index < AminoAcidDictionary::unknown)
        aminoAcid = aminoAcidCollection->getElement(index);
    }
    ProteinResiduePtr residue = new ProteinResidue(protein->getName() + T("(") + lbcpp::toString(i) + T(")"), aminoAcid);
    if (i > 0)
      res.back()->setNext(residue);
    res.push_back(residue);
  }
}

/////////////////////////////////////////

class ProteinResidueInputAttributes : public Object
{
public:
  ProteinResidueInputAttributes(ProteinResiduePtr residue, size_t windowSize)
    : residue(residue), windowSize(windowSize) {}

  virtual void accept(ObjectVisitorPtr visitor)
  {
    size_t index = 0;
    visitor->visitVariable(index++, residue->getAminoAcidType());

    ProteinResiduePtr prev = residue->getPrevious();
    for (size_t i = 0; prev && i < windowSize; ++i, prev = prev->getPrevious())
      visitor->visitVariable(1 + i, prev->getAminoAcidType());

    ProteinResiduePtr next = residue->getNext();
    for (size_t i = 0; next && i < windowSize; ++i, next = next->getNext())
      visitor->visitVariable(1 + windowSize + i, next->getAminoAcidType());
  }

private:
  ProteinResiduePtr residue;
  size_t windowSize;
};

class ProteinResidueInputAttributesClass : public Class
{
public:
  ProteinResidueInputAttributesClass(size_t windowSize)
    : Class(T("ProteinResidueInputAttributes"), ObjectClass::getInstance())
  {
    EnumerationPtr aminoAcidTypeEnumeration = Enumeration::get(T("AminoAcidType"));
    addVariable(aminoAcidTypeEnumeration, T("AA[i]"));
    for (size_t i = 0; i < windowSize; ++i)
      addVariable(aminoAcidTypeEnumeration, T("AA[i - ") + lbcpp::toString(i + 1) + T("]"));
    for (size_t i = 0; i < windowSize; ++i)
      addVariable(aminoAcidTypeEnumeration, T("AA[i + ") + lbcpp::toString(i + 1) + T("]"));
  }
};

/////////////////////////////////////////

namespace lbcpp 
{

// todo: extract VariantUnion

class Variant
{
public:
  Variant(ClassPtr type, bool boolValue)
    : type(type) {jassert(isBoolean()); u.boolValue = boolValue;}
  Variant(ClassPtr type, int intValue)
    : type(type) {jassert(isInteger()); u.intValue = intValue;} 
  Variant(ClassPtr type, double doubleValue)
    : type(type) {jassert(isDouble()); u.doubleValue = doubleValue;}
  Variant(ClassPtr type, const String& stringValue)
    : type(type) {jassert(isString()); u.stringValue = new String(stringValue);}
  Variant(ClassPtr type, ObjectPtr objectValue)
    : type(type)
  {
    jassert(isObject());
    u.objectValue = objectValue.get();
    if (objectValue)
      objectValue->incrementReferenceCounter();
  }

  Variant(const Variant& otherVariant)
    {*this = otherVariant;}

  Variant()
    {u.intValue = 0;}
  
  ~Variant()
    {clear();}

  void clear()
  {
    if (!type)
      return;

    if (isString())
    {
      delete u.stringValue;
      u.stringValue = NULL;
    }
    else if (isObject())
    {
      if (u.objectValue)
      {
        u.objectValue->decrementReferenceCounter();
        u.objectValue = NULL;
      }
    }
    type = ClassPtr();
  }

  Variant& operator =(const Variant& otherVariant)
  {
    clear();
    type = otherVariant.getType();
    if (isUndefined())
      return *this;
    else if (isBoolean())
      u.boolValue = otherVariant.u.boolValue;
    else if (isInteger())
      u.intValue = otherVariant.u.intValue;
    else if (isDouble())
      u.doubleValue = otherVariant.u.doubleValue;
    else if (isString())
      u.stringValue = new String(*otherVariant.u.stringValue);
    else if (isObject())
    {
      u.objectValue = otherVariant.u.objectValue;
      if (u.objectValue)
        u.objectValue->incrementReferenceCounter();
    }
    else
    {
      Object::error(T("Variant::operator ="), T("Unrecognized type of variant"));
      jassert(false);
    }
    return *this;
  }

  ClassPtr getType() const
    {return type;}

  bool isUndefined() const
    {return !type;}

  bool isBoolean() const
    {return type && type.isInstanceOf<BooleanClass>();}

  bool isInteger() const
    {return type && type.isInstanceOf<IntegerClass>();}

  bool isDouble() const
    {return type && type.isInstanceOf<DoubleClass>();}

  bool isString() const
    {return type && type.isInstanceOf<StringClass>();}

  bool isObject() const
    {return type && !type.isInstanceOf<BuiltinTypeClass>();}

private:
  ClassPtr type;

  union
  {
    bool boolValue;
    int intValue;
    double doubleValue;
    String* stringValue;
    Object* objectValue;
  } u;
};

class BinaryDecisionTree : public Object
{
public:
  void reserveNodes(size_t size)
    {nodes.reserve(size);}

  size_t addNode()
  {
    size_t res = nodes.size();
    nodes.push_back(Node()); // FIXME
    return res;
  }

  void reserveLeaves(size_t size)
    {leaves.reserve(size);}

protected:
  struct Node
  {
    size_t indexOfLeftChild;
    size_t splitVariable;
    Variant splitArgument;
  };
  std::vector<Node> nodes;
  std::vector<Variant> leaves;
};

typedef ReferenceCountedObjectPtr<BinaryDecisionTree> BinaryDecisionTreePtr;

class ExtraTreeInference : public Inference
{
public:
  ExtraTreeInference(const String& name, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);

  void addTree(BinaryDecisionTreePtr tree)
    {trees.push_back(tree);}

protected:
  std::vector<BinaryDecisionTreePtr> trees;

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    return ObjectPtr();
  }
};

typedef ReferenceCountedObjectPtr<ExtraTreeInference> ExtraTreeInferencePtr;

// Input: (Inference, training data ObjectContainer) pair
// Supervision: None
// Output: BinaryDecisionTree
class SingleExtraTreeInferenceLearner : public Inference
{
public:
  SingleExtraTreeInferenceLearner(size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
    : Inference(T("SingleExtraTreeInferenceLearner")),
      numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
      minimumSizeForSplitting(minimumSizeForSplitting) {}

protected:
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ObjectPairPtr inferenceAndTrainingData = input.dynamicCast<ObjectPair>();
    jassert(inferenceAndTrainingData);
    InferencePtr inference = inferenceAndTrainingData->getFirst().dynamicCast<Inference>();
    ObjectContainerPtr trainingData = inferenceAndTrainingData->getSecond().dynamicCast<ObjectContainer>();
    jassert(inference && trainingData);

    ObjectPairPtr firstExample = trainingData->getAndCast<ObjectPair>(0);
    jassert(firstExample);
    ClassPtr inputClass = firstExample->getFirst()->getClass();
    ClassPtr outputClass = firstExample->getSecond()->getClass();
#ifdef JUCE_DEBUG
    for (size_t i = 1; i < trainingData->size(); ++i)
    {
      ObjectPairPtr example = trainingData->getAndCast<ObjectPair>(i);
      jassert(example->getFirst()->getClass() == inputClass);
      jassert(example->getSecond()->getClass() == outputClass);
    }
#endif // JUCE_DEBUG

    return sampleTree(inputClass, outputClass, trainingData);
  }

  BinaryDecisionTreePtr sampleTree(ClassPtr inputClass, ClassPtr outputClass, ObjectContainerPtr trainingData)
  {
    size_t n = trainingData->size();
    if (!n)
      return BinaryDecisionTreePtr();

    BinaryDecisionTreePtr res = new BinaryDecisionTree();
    res->reserveNodes(n);
    res->reserveLeaves(n);
    std::set<size_t> indices;
    for (size_t i = 0; i < n; ++i)
      indices.insert(i);
    std::set<size_t> nonConstantAttributes;
    // todo: fill nonConstantAttributes
    sampleTreeRecursively(res, inputClass, outputClass, trainingData, indices, nonConstantAttributes);
    return res;
  }

  static bool areOutputObjectsEqual(ObjectPtr object1, ObjectPtr object2)
    {return object1 == object2;}

  bool createLeaf(ObjectContainerPtr trainingData, const std::set<size_t>& indices, const std::set<size_t>& nonConstantAttributes) const
  {
    if (indices.empty() || indices.size() < numAttributeSamplesPerSplit || nonConstantAttributes.empty())
      return true;
    std::set<size_t>::const_iterator it = indices.begin();
    ObjectPtr firstOutput = trainingData->getAndCast<ObjectPair>(*it)->getSecond();
    for (++it; it != indices.end(); ++it)
    {
      ObjectPtr output = trainingData->getAndCast<ObjectPair>(*it)->getSecond();
      if (!areOutputObjectsEqual(firstOutput, output))
        return false;
    }
    return true;
  }

  size_t sampleTreeRecursively(BinaryDecisionTreePtr tree, ClassPtr inputClass, ClassPtr outputClass, ObjectContainerPtr trainingData, const std::set<size_t>& indices, const std::set<size_t>& nonConstantAttributes)
  {
    if (createLeaf(trainingData, indices, nonConstantAttributes))
    {
    }
    else
    {
    }
    return 0; // FIXME    
  }
};

class ExtraTreeInferenceLearner : public SharedParallelInference
{
public:
  ExtraTreeInferenceLearner(size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0)
    : SharedParallelInference(T("ExtraTreeInferenceLearner"),
        new SingleExtraTreeInferenceLearner(numAttributeSamplesPerSplit, minimumSizeForSplitting)), numTrees(numTrees) {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    for (size_t i = 0; i < numTrees; ++i)
      res->addSubInference(subInference, new ObjectPair(input, supervision) /* tmp */, ObjectPtr());
    return res;
  }

  virtual ObjectPtr finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    ExtraTreeInferencePtr learnedInference = state->getInput().dynamicCast<ExtraTreeInference>(); // tmp 
    jassert(learnedInference); 
    for (size_t i = 0; i < numTrees; ++i)
    {
      BinaryDecisionTreePtr decisionTree = state->getSubOutput(i).dynamicCast<BinaryDecisionTree>();
      jassert(decisionTree);
      learnedInference->addTree(decisionTree);
    }
    return ObjectPtr();
  }

private:
  size_t numTrees;
};

ExtraTreeInference::ExtraTreeInference(const String& name, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
  : Inference(name)
{
  setBatchLearner(new ExtraTreeInferenceLearner(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting));
}

}; /* namespace lbcpp */

/////////////////////////////////////////

int main(int argc, char** argv)
{
  declareProteinClasses();
  Class::declare(new ProteinResidueClass());
  Class::declare(new ProteinResidueInputAttributesClass(8));

  File workingDirectory(T("C:\\Projets\\LBC++\\projects\\temp"));
  ObjectContainerPtr proteins = loadProteins(workingDirectory.getChildFile(T("L50DB")));
  
  VectorObjectContainerPtr secondaryStructureExamples = new VectorObjectContainer();
  for (size_t i = 0; i < proteins->size(); ++i)
  {
    ProteinPtr protein = proteins->getAndCast<Protein>(i);
    jassert(protein);
    LabelSequencePtr secondaryStructure = protein->getSecondaryStructureSequence();
    jassert(secondaryStructure);

    std::vector<ProteinResiduePtr> residues;
    createResidues(protein, residues);
    for (size_t position = 0; position < residues.size(); ++position)
    {
      ProteinResiduePtr residue = residues[position];

      /*std::cout << "========" << position << "==========" << std::endl;
      PrintObjectVisitor::print(residue, std::cout, 2);
      std::cout << std::endl;*/
      
      ObjectPtr input = new ProteinResidueInputAttributes(residue, 8);
      ObjectPtr output = secondaryStructure->get(position);
      secondaryStructureExamples->append(new ObjectPair(input, output));
    }
  }
  std::cout << secondaryStructureExamples->size() << " secondary structure examples" << std::endl;
  PrintObjectVisitor::print(secondaryStructureExamples->getAndCast<ObjectPair>(10)->getFirst(), std::cout, 2);

  InferencePtr inference = new ExtraTreeInference(T("SS3"));
  InferenceContextPtr context = singleThreadedInferenceContext();

  ObjectContainerPtr trainingData = secondaryStructureExamples->fold(0, 2);
  ObjectContainerPtr testingData = secondaryStructureExamples->fold(1, 2);

  context->train(inference, trainingData);
  
  // todo: evaluate on train and on test
  return 0;
}
