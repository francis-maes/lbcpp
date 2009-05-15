#include "GeneratedCode/LCeB.lh"
#include "GeneratedCode/NQueens.lh"
#include <fstream>
using namespace lbcpp;

CRAlgorithmPtr backTrackingDepthFirstSearchRec(ChoosePtr choose)
{
  assert(choose);
  for (VariableIteratorPtr it = choose->newIterator(); it->exists(); it->next())
  {
    CRAlgorithmPtr nextState = choose->getCRAlgorithm()->clone();
    double reward;
    ChoosePtr nextChoose = nextState->runUntilNextChoose(it->get(), &reward);
    if (reward > 0)
      return nextState;
    else if (nextChoose)
    {
      CRAlgorithmPtr res = backTrackingDepthFirstSearchRec(nextChoose);
      if (res)
        return res;
    }
  }
  return CRAlgorithmPtr();  
}

CRAlgorithmPtr backTrackingDepthFirstSearch(CRAlgorithmPtr initialState)
{
  CRAlgorithmPtr state = initialState->clone();
  ChoosePtr choose = state->runUntilFirstChoose();
  assert(choose);
  return backTrackingDepthFirstSearchRec(choose);
}

int main(int argc, char* argv[])
{
  for (int i = 0; i < 100; ++i)
  {
    std::cout << "ITERATION " << i+1 << std::endl;

   // CRAlgorithmPtr instance = lceb::sampleInstance(6);
   CRAlgorithmPtr instance = nqueens::searchSpace(i + 1);
//    std::cout << "Instance: " << instance->toString() << std::endl;
    
    CRAlgorithmPtr solution = backTrackingDepthFirstSearch(instance);
    if (!solution)
      std::cout << "No solutions" << std::endl;
    else
      std::cout << "Solution: " << solution->toString() << std::endl;
  }
  return 0;
}
