#include "GeneratedCode/Test.h"
using namespace lbcpp;

int main(int argc, char* argv[])
{
  CRAlgorithmPtr lbcpp = toto();

  VariablePtr choice;
  while (true)
  {
	  static int one = 1;

	  double reward;
	  ChoosePtr choose = lbcpp->runUntilNextChoose(choice, &reward);
	  std::cout << "Step reward: " << reward << " first var value = " << lbcpp->getVariableValue(0) << std::endl;
	  if (!choose)
		  break;
	  choice = Variable::create(one);
  }
  return 0;
}
