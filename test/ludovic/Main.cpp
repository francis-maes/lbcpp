#include "GeneratedCode/Test.h"
using namespace lcpp;

int main(int argc, char* argv[])
{
  CRAlgorithmPtr lcpp = toto();

  VariablePtr choice;
  while (true)
  {
	  static int one = 1;

	  double reward;
	  ChoosePtr choose = lcpp->runUntilNextChoose(choice, &reward);
	  std::cout << "Step reward: " << reward << " first var value = " << lcpp->getVariableValue(0) << std::endl;
	  if (!choose)
		  break;
	  choice = Variable::create(one);
  }
  return 0;
}
