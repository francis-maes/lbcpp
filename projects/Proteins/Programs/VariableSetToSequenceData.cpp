/*-----------------------------------------.---------------------------------.
| Filename: VariableSetToSequenceData.cpp  | Convert variable sets to a      |
| Author  : Francis Maes                   |  old school sequence datafile   |
| Started : 30/03/2010 19:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "GeneratedCode/Data/Bio/Protein.lh"
#include "../VariableSetModel.h"
using namespace lbcpp;

extern void declareProteinsClasses();
extern void declareInterdependantVariableSetClasses();

class ProteinToVariableSetExample : public ObjectFunction
{
public:
  virtual String getOutputClassName(const String& inputClassName) const
    {return T("VariableSetExample");}

  virtual ObjectPtr function(ObjectPtr object) const
  {
    ProteinPtr protein = object.dynamicCast<Protein>();
    jassert(protein);
    return ObjectPtr(); // FIXME new SecondaryStructureVariableSetExample(protein);
  }
};

class SequenceExamplesPrinter : public LearningDataObjectPrinter
{
public:
  SequenceExamplesPrinter(const File& file)
    : LearningDataObjectPrinter(file), sequenceNumber(0) {}
  
  virtual void consume(ObjectPtr object)
  {
    VariableSetExamplePtr example = object.dynamicCast<VariableSetExample>();
    jassert(example);
    VariableSetPtr variables = example->getTargetVariables();

    print("# Sequence " + lbcpp::toString(++sequenceNumber) + "\n");
    for (size_t i = 0; i < variables->getNumVariables(); ++i)
    {
      size_t label;
      variables->getVariable(i, label);

      print(variables->getVariablesDictionary()->getString(label) + T(" "));
      printFeatureList(example->getVariableFeatures(i));
      printNewLine();
    }
    printNewLine();
  }

  size_t sequenceNumber;
};

int main()
{
  File cb513Directory = 
    File(T("C:/Projets/Proteins/data/CB513cool"));

  File outputFile(T("C:/Projets/Proteins/data/cb513SecondaryStructure3.sequences"));
  outputFile.deleteFile();

  declareProteinsClasses();
  ObjectStreamPtr proteinsStream = directoryObjectStream(cb513Directory, T("*.protein"));
  ObjectConsumerPtr printer = new SequenceExamplesPrinter(outputFile);
  printer->consume(proteinsStream->apply(new ProteinToVariableSetExample()));
  return 0;
};

#if 0
int main()
{
  File cb513Directory = 
    //File(T("/Users/francis/Projets/Proteins/Data/CB513"));
    File(T("C:/Projets/Proteins/data/CB513cool"));

  declareProteinsClasses();
  ObjectStreamPtr proteinsStream = directoryObjectStream(cb513Directory, T("*.protein"));
  ObjectStreamPtr examplesStream = proteinsStream->apply(new ProteinToVariableSetExample());
  ObjectContainerPtr examples = examplesStream->load()->randomize();
  StringDictionaryPtr labels = examples->getAndCast<VariableSetExample>(0)->getTargetVariables()->getVariablesDictionary();

  size_t numFolds = 7;
  double cvResult = 0.0;
  for (size_t i = 0; i < numFolds; ++i)
  {
    VariableSetModelPtr model =
      //new IndependantClassificationVariableSetModel(createMaxentClassifier(labels));
      //iterativeClassificationVariableSetModel(createMaxentClassifier(labels), createMaxentClassifier(labels));
      simulatedIterativeClassificationVariableSetModel(createMaxentClassifier(labels, false), createLearningStoppingCriterion());

    std::cout << std::endl << std::endl << "FOLD " << (i+1) << " / " << numFolds << "...." << std::endl;
    model->trainBatch(examples->invFold(i, numFolds), consoleProgressCallback());
    double trainAccuracy = model->evaluate(examples->invFold(i, numFolds));
    double testAccuracy = model->evaluate(examples->fold(i, numFolds));
    std::cout << "Train Score: " << trainAccuracy << std::endl;
    std::cout << "Test Score: " << testAccuracy << std::endl;
    cvResult += testAccuracy;
  }
  
  std::cout << std::endl << std::endl << "Cross Validation Result: " << (cvResult / numFolds) << std::endl;
 
  // Results: Prediction of SS3 / 7 folds CV
  
  // ---CO---
  //
  // AA(7)+PSSM(7) with 1/1 train iter => 71.30
  // AA(7)+PSSM(7) with 2/2 train iter => 72.01
  // AA(7)+PSSM(7) with 2/10 train iter => 72.29
  
  // ---ICA---
  // AA(7)+PSSM(7) + PR(1) with 2/2 train iter => 70.96
  // AA(7)+PSSM(7) + PR(2) with 2/2 train iter => 71.67
  // AA(7)+PSSM(7) + PR(5) with 2/2 train iter => 71.63
  
  // ---SICA---
  // AA(7)+PSSM(7) + PR(2) with 2/2 train iter, reg 0, maxpass 5 => 70.61
  // AA(7)+PSSM(7) + PR(2) with 2/10 train iter, reg 0.0001, maxpass 10
  
  // Results: Predition of SS3:
  // AA(15) => 62.2
  // PSSM(15) => 72.1
  // AA(15)+PSSM(15) => 73.9
  // AA(15)+PSSM(15)+OPT(1) => 88.3
  // AA(15)+PSSM(15)+OPT(10) => 89.1
  // AA(15)+PSSM(15)+OPT_8(10) => 90.5
  // PSSM(15)+OPT(1) => 87.8
  // AA(15)+OPT(1) => 87.6
  // OPT(1) => 82.8
  // OPT(10) => 84.7
  return 0;
}
#endif // 0
