#include "GeneratedCode/Test.h"
using namespace cralgo;

int main(int argc, char* argv[])
{
  CRAlgorithmPtr cralgo = toto();

  const void* choice = NULL;
  while (true)
  {
	  static int one = 1;

	  double reward;
	  ChoosePtr choose = cralgo->runUntilNextChoose(choice, &reward);
	  std::cout << "Step reward: " << reward << " first var value = " << cralgo->getVariableValue(0) << std::endl;
	  if (!choose)
		  break;
	  choice = &one;
  }
  return 0;
}
