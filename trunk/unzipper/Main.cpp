/*-----------------------------------------.---------------------------------.
 | Filename: RunWorkUnit.cpp                | A program to launch work units  |
 | Author  : Arnaud Schoofs                 |                                 |
 | Started : 30/10/2010 12:00               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
																`--------------------------------------------*/

#include <lbcpp/library.h>
using namespace lbcpp;

int mainImpl(int argc, char** argv) {
	File file(argv[1]);
	if (!file.existsAsFile()) {
		std::cerr << "ZipFile not found:" << argv[1] << std::endl;
		return -1;
	}
	
	ZipFile zippy(file);
	File target(argv[2]);
	zippy.uncompressTo(target, false);
	
	return 0;
}


int main(int argc, char** argv) {
	lbcpp::initialize(argv[0]);
  int exitCode = mainImpl(argc, argv);
  lbcpp::deinitialize();
  return exitCode;
}

