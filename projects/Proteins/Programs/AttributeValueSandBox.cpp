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

int main(int argc, char** argv)
{
  declareProteinClasses();
  Class::declare(new ProteinResidueClass());
  Class::declare(new ProteinResidueInputAttributesClass(8));


  Variable container = Variable::pair(16.64, 51);
  std::cout << "pair: " << container.toString() << " size: " << container.size() << std::endl;
  for (size_t i = 0; i < container.size(); ++i)
    std::cout << "  elt " << i << " = " << container[0].toString() << "( type = " << container[0].getType()->getName() << ")" << std::endl;
  Variable containerCopy = container;
  std::cout << "container copy: " << containerCopy << " equals: " << (container == containerCopy) << std::endl;
  return 0;

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

  InferencePtr inference = multiClassExtraTreeInference(T("SS3"));
  InferenceContextPtr context = singleThreadedInferenceContext();

  ObjectContainerPtr trainingData = secondaryStructureExamples->fold(0, 2);
  ObjectContainerPtr testingData = secondaryStructureExamples->fold(1, 2);

  context->train(inference, trainingData);
  
  // todo: evaluate on train and on test
  return 0;
}
