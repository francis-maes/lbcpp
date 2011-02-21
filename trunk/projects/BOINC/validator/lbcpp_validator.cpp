#include <lbcpp/Execution/ExecutionTrace.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Core/Variable.h>
#include <lbcpp/library.h>
using namespace lbcpp;


int mainImpl(int argc, char** argv) {
  File file = File::getCurrentWorkingDirectory().getChildFile("output.trace");
  XmlImporter importer(defaultExecutionContext(), file);
  if (!importer.isOpened()) {
    return 1;
  }

  if (!importer.enter("variable") || !importer.enter("node") || !importer.enter("return")) {
    return 1;
  }

  Variable var = importer.loadVariable(TypePtr());

  /*
  Variable var2 = importer.load();
  //if (!var2.inheritsFrom(executionTraceClass)) {
  //  return 1;
  //}
  ExecutionTracePtr trace = var2.getObjectAndCast<ExecutionTrace>();
  Variable var = trace->getRootNode()->getReturnValue();
  if (!var.exists() || !var.isDouble()) {
    return 1;
  }
  */

  std::cout << "Score is : " << var.getDouble() << std::endl;

  return 0;
}

int main(int argc, char** argv) {
  lbcpp::initialize(argv[0]);
  int exitCode = mainImpl(argc, argv);
  lbcpp::deinitialize();
  return exitCode;
}

