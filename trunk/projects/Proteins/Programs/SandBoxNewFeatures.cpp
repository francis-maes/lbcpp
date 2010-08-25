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


int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();
  
  File workingDirectory(T("C:\\Projets\\LBC++\\projects\\temp"));
  Variable protein = Variable::createFromFile(workingDirectory.getChildFile(T("PDB30Small/xml/1A7M.xml")));
  if (!protein)
    return 1;

  
  ProteinInferenceFactory factory;
  PerceptionPtr perception = factory.createResiduePerception(String::empty);
  perception = perceptionToFeatures(perception);
  Variable input = Variable::pair(protein, 3);

  Variable parameters = Variable::create(perception->getOutputType());
  std::cout << "Parameters: " << std::endl;
  parameters.printRecursively(std::cout, -1, false);

  Variable output = perception->compute(input);
  if (!output)
    return 1;
  std::cout << "Features: " << std::endl;
  output.printRecursively(std::cout, -1, false);

  std::cout << "L0: " << l0norm(perception, input) << " " << l0norm(output) << std::endl;
  std::cout << "L1: " << l1norm(perception, input) << " " << l1norm(output) << std::endl;
  std::cout << "L2: " << l2norm(perception, input) << " " << l2norm(output) << std::endl;
  std::cout << "Dot Product: " << dotProduct(output, perception, input) << std::endl;


  addWeighted(parameters, perception, input, 0.1);
  std::cout << "Parameters: " << std::endl;
  parameters.printRecursively(std::cout, -1, false);

  addWeighted(parameters, perception, input, -0.1);
  std::cout << "Parameters: " << std::endl;
  parameters.printRecursively(std::cout, -1, false);

  return 0;
}
