/*-----------------------------------------.---------------------------------.
| Filename: LBCppValidator.cpp             | BOINC validator                 |
| Author  : Arnaud Schoofs                 | Implementation based on :       |
| Started : 21/02/2011 14:00               | sample_bitwise_validator.cpp    |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

// LBCpp includes
#include <lbcpp/Execution/ExecutionTrace.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Core/Variable.h>
#include <lbcpp/library.h>

// BOINC includes
#include <config.h>
#include <util.h>
#include <sched_util.h>
#include <sched_msgs.h>
#include <validate_util.h>

using namespace lbcpp;

int getScoreImpl(const char* fileName, double& score)
{
  
  // TODO arnaud : throw catch ?
  
  ObjectPtr obj = Object::createFromFile(context, File(fileName));
  if (!obj) {
    log_messages.printf(MSG_CRITICAL, "Can't create an Object from file : %s\n", fileName);
    return 1;
  }
  
  ExecutionTracePtr trace = obj.dynamicCast<ExecutionTrace>();
  if (!trace) {
    log_messages.printf(MSG_CRITICAL, "This file is not a valide ExecutionTrace : %s\n", fileName);
    return 1;
  }
  
  ExecutionTraceNodePtr root = trace->getRootNode();
  if (!root) {
    log_messages.printf(MSG_CRITICAL, "Can't get root from ExecutionTrace : %s\n", fileName);
    return 1;
  }
  
  std::vector<ExecutionTraceItemPtr> vec = root->getSubItems();  
  if (vec.empty()) {
    log_messages.printf(MSG_CRITICAL, "Can't get subitems from ExecutionTrace : %s\n", fileName);
    return 1;
  }
  
  ExecutionTraceNodePtr traceNode = vec[0].dynamicCast<ExecutionTraceNode>();
  if (!traceNode) {
    log_messages.printf(MSG_CRITICAL, "Can't get node from ExecutionTrace : %s\n", fileName);
    return 1;
  }

  Variable returnValue = traceNode->getReturnValue();
  if (!returnValue) {
    log_messages.printf(MSG_CRITICAL, "No return value in ExecutionTrace : %s\n", fileName);
    return 1;
  }
  
  ScoreObjectPtr scoreObject = returnValue.dynamicCast<ScoreObject>();
  if (!scoreObject) {
    log_messages.printf(MSG_CRITICAL, "Return value in ExecutionTrace is not a ScoreObject : %s\n", fileName);
    return 1;
  }
  
  score = 1 - scoreObject->getScoreToMinimize();  
  
  return 0;
}

int getScore(const char* fileName, double& score)
{
  lbcpp::initialize("validator");
  int exitCode = getScoreImpl(fileName, score);
  lbcpp::deinitialize();
  return exitCode;
}

int init_result(RESULT& result, void*& data)
{
  int retval;
  std::vector<FILE_INFO> files;
  std::vector<double>* scores = new std::vector<double>();

  retval = get_output_file_infos(result, files);
  if (retval)
  {
    log_messages.printf(MSG_CRITICAL,
                        "[RESULT#%d %s] check_set: can't get output filenames\n",
                        result.id, result.name);
    return retval;
  }

  for (unsigned int i=0; i<files.size(); i++)
  {
    FILE_INFO& fi = files[i];
    if (fi.no_validate) continue;

    double score;
    retval = getScore(fi.path.c_str(), score);
    if (retval)
    {
      log_messages.printf(MSG_CRITICAL,
                          "[RESULT#%d %s] Couldn't get score in %s\n",
                          result.id, result.name, fi.path.c_str());
      return retval;
    }
    scores->push_back(score);
    data = (void*) scores;
    return 0;
  }
}

int compare_results(RESULT & r1, void* data1, RESULT const& r2, void* data2, bool& match)
{
  std::vector<double>* scores1 = (std::vector<double>*) data1;
  std::vector<double>* scores2 = (std::vector<double>*) data2;

  if (scores1->size() != scores2->size())
  {
    log_messages.printf(MSG_CRITICAL,
                        "[RESULT#%d %s && RESULT#%d %s] Not the same number of scores: %d vs %d\n",
                        r1.id, r1.name, r1.id, r2.name, scores1->size(), scores2->size());
    match = false;
    return 0;
  }

  for (unsigned i = 0; i < scores1->size(); ++i)
    if (fabs(scores1->at(i) - scores2->at(i)) > 0.000000001)
    {
      match = false;
      return 0;
    }

  match = true;
  return 0;
}

int cleanup_result(RESULT const& result, void* data)
{
  std::vector<double>* scores = (std::vector<double>*) data;
  scores->clear();
  delete scores;
  return 0;
}
