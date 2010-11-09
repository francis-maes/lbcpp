/*-----------------------------------------.---------------------------------.
| Filename: DigiBox.cpp                    | Digi Box                        |
| Author  : Julien Becker                  |                                 |
| Started : 08/11/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "MNISTProgram.h"
#include "MNISTImage.h"
#include "MatlabFileParser.h"

using namespace lbcpp;

class MNISTImage : public Perception
{
  virtual TypePtr getInputType() const
    {return mnistImageClass;}
  
  virtual void computeOutputType()
  {
    /*
    for (size_t i = 0; i < MNISTImage::numPixels; ++i)
      addOutputVariable(String(i), doubleType);
    Perception::*/
  }
};

ContainerPtr MNISTProgram::loadDataFromFile(const File& file)
{
  ObjectVectorPtr res = new ObjectVector(mnistImageClass, 0);
  
  ReferenceCountedObjectPtr<MatlabFileParser> parser(new MatlabFileParser(file));

  while (!parser->isExhausted())
  {
    Variable v = parser->next();
    if (v.isNil())
      break;
    res->append(v);
  }
  
  return res;
}

bool MNISTProgram::loadData()
{
  if (learningFile == File::nonexistent)
  {
    std::cerr << "Error - No learning file found !" << std::endl;
    return false;
  }
  
  learningData = loadDataFromFile(learningFile);
  if (!learningData->getNumElements())
  {
    std::cerr << "Error - No training data found in " << learningFile.getFullPathName().quoted() << std::endl;
    return false;
  }
  
  if (testingFile == File::nonexistent)
  {
    testingData = learningData->fold(0, 10);
    learningData = learningData->invFold(0, 10);
    return true;
  }
  
  testingData = loadDataFromFile(testingFile);
  if (!testingData->getNumElements())
  {
    std::cerr << "Error - No testing data found in " << testingFile.getFullPathName().quoted() << std::endl;
    return false;
  }
  
  return true;
}

int MNISTProgram::runProgram(MessageCallback& callback)
{
  if (!loadData())
    return -1;
  
  std::cout << "Learning images : " << learningData->getNumElements() << std::endl;
  std::cout << "Testing images  : " << testingData->getNumElements() << std::endl;
  
  InferenceContextPtr context = singleThreadedInferenceContext();
  /*
  PerceptionPtr perception = ...;
  
  InferencePtr inference = multiClassLinearSVMInference(T("Digit"), perception, digitTypeEnumeration, false);

  context->train(inference, trainingData);
  */
  return 0;
}

extern void declareMNISTClasses();

namespace lbcpp
{
  extern ClassPtr mnistProgramClass;
}

int main(int argc, char* argv[])
{
  lbcpp::initialize();
  declareMNISTClasses();
  return ProgramPtr(new MNISTProgram())->main(argc, argv);
}
