/*-----------------------------------------.---------------------------------.
| Filename: ConnectivityPatternClassifi.cpp| Connectivity Pattern Classifier |
| Author  : Julien Becker                  |                                 |
| Started : 16/06/2011 13:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ConnectivityPatternClassifier.h"

using namespace lbcpp;

TypePtr AddConnectivityPatternBiasLearnableFunction::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  outputName = T("biased");
  outputShortName = T("b");
  setBatchLearner(new AddConnectivityPatternBiasBatchLearner());
  return symmetricMatrixClass(doubleType);
}
