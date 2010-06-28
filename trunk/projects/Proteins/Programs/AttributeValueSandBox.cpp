/*-----------------------------------------.---------------------------------.
| Filename: AttributeValueSandBox.cpp      | Attribute Value Representation  |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 11:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Protein/ProteinObject.h"
#include "Protein/Data/Protein.h"
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

  virtual Variable getVariable(size_t index) const
  {
    switch (index)
    {
    case anintVariable: return 51;
    case adoubleVariable: return 16.64;
    case astringVariable: return T("Hello World");
    case anobjectVariable: return Variable(const_cast<A* >(this));
    };
  }

  virtual void setVariable(size_t index, const Variable& value)
    {jassert(false);}
};

typedef ReferenceCountedObjectPtr<A> APtr;

class AClass : public DefaultClass_<A>
{
public:
  AClass() : DefaultClass_<A>(objectClass())
  {
    addVariable(integerClass(), T("anint"));
    addVariable(doubleClass(), T("adouble"));
    addVariable(stringClass(), T("astring"));
    addVariable(objectClass(), T("anobject"));
  }
};

////////////////////////////

class PrintObjectVisitor : public ObjectVisitor
{
public:
  PrintObjectVisitor(ObjectPtr object, std::ostream& ostr, int maxDepth = -1)
    : currentClasses(1, object->getClass()), ostr(ostr), maxDepth(maxDepth) {jassert(currentClasses[0]);}
  
  virtual void visit(size_t variableNumber, const Variable& value)
  {
    printVariable(variableNumber, value.toString());
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
      << currentClass->getStaticVariableType(variableNumber)->getName() << " "
      << currentClass->getStaticVariableName(variableNumber) << " = "
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
    ProteinObjectPtr protein = res->getAndCast<ProteinObject>(i);
    jassert(protein);
    protein->computeMissingFields();
  }
  return res;
}



void createResidues(ProteinObjectPtr protein, std::vector<ResiduePtr>& res)
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
    ResiduePtr residue = new Residue(protein->getName() + T("(") + lbcpp::toString(i) + T(")"), aminoAcid);
    if (i > 0)
      res.back()->setNext(residue);
    res.push_back(residue);
  }
}

/////////////////////////////////////////

class ProteinResidueInputAttributes : public Object
{
public:
  ProteinResidueInputAttributes(ResiduePtr residue, size_t windowSize)
    : residue(residue), windowSize(windowSize) {}

  virtual Variable getVariable(size_t index) const
  {
    // FIXM
    return Variable();
  }

  virtual void accept(ObjectVisitorPtr visitor)
  {
    size_t index = 0;
    visitor->visit(index++, Variable(residue->getAminoAcidType(), aminoAcidTypeEnumeration()));

    ResiduePtr prev = residue->getPrevious();
    for (size_t i = 0; prev && i < windowSize; ++i, prev = prev->getPrevious())
      visitor->visit(1 + i, Variable(prev->getAminoAcidType(), aminoAcidTypeEnumeration()));

    ResiduePtr next = residue->getNext();
    for (size_t i = 0; next && i < windowSize; ++i, next = next->getNext())
      visitor->visit(1 + windowSize + i, Variable(next->getAminoAcidType(), aminoAcidTypeEnumeration()));
  }

private:
  ResiduePtr residue;
  size_t windowSize;
};

class ProteinResidueInputAttributesClass : public ObjectClass
{
public:
  ProteinResidueInputAttributesClass(size_t windowSize)
    : ObjectClass(T("ProteinResidueInputAttributes"), objectClass())
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

  Variable myEnumValue(asparticAcid, aminoAcidTypeEnumeration());
  std::cout << myEnumValue << std::endl;

  Variable container = Variable::pair(16.64, 51);
  std::cout << "pair: " << container << " size: " << container.size() << std::endl;
  for (size_t i = 0; i < container.size(); ++i)
    std::cout << "  elt " << i << " = " << container[i].toString() << " (type = " << container[i].getType()->getName() << ")" << std::endl;
  Variable containerCopy = container;
  std::cout << "container copy: " << containerCopy << " (type = "
    << containerCopy.getType()->getName() << " equals: " << Variable(container == containerCopy) << std::endl;
  
  return 0;
  
  File workingDirectory(T("C:\\Projets\\LBC++\\projects\\temp"));
  ObjectContainerPtr proteins = loadProteins(workingDirectory.getChildFile(T("L50DB")));
  
  VectorObjectContainerPtr secondaryStructureExamples = new VectorObjectContainer();
  for (size_t i = 0; i < proteins->size(); ++i)
  {
    ProteinObjectPtr protein = proteins->getAndCast<ProteinObject>(i);
    jassert(protein);
    LabelSequencePtr secondaryStructure = protein->getSecondaryStructureSequence();
    jassert(secondaryStructure);

    std::vector<ResiduePtr> residues;
    createResidues(protein, residues);
    for (size_t position = 0; position < residues.size(); ++position)
    {
      ResiduePtr residue = residues[position];

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

  InferencePtr inference = multiClassExtraTreeInference(T("SS3"));
  InferenceContextPtr context = singleThreadedInferenceContext();

  ObjectContainerPtr trainingData = secondaryStructureExamples->fold(0, 2);
  ObjectContainerPtr testingData = secondaryStructureExamples->fold(1, 2);

  context->train(inference, trainingData);
  
  // todo: evaluate on train and on test
  return 0;
}
