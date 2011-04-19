/*-----------------------------------------.---------------------------------.
| Filename: unzipper.cpp                   | A program to unzip zip files    |
| Author  : Arnaud Schoofs                 |                                 |
| Started : 16/12/2010 12:25               |                                 |
`------------------------------------------/                                 |
															 |                                             |
                               `--------------------------------------------*/


#include <lbcpp/library.h>
using namespace lbcpp;

void usage()
{
  std::cerr << "Usage: unzipper /absolute/path/to/source.zip /absolute/path/to/destination/directory/" << std::endl;
}

int mainImpl(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "Invalid number of arguments!" << std::endl;
    usage();
    return 1;
  }
  
  File file(argv[1]);
  if (!file.existsAsFile()) {
    std::cerr << "ZipFile not found:" << std::endl;
	std::cerr << argv[1] << std::endl;
    usage();
    return 1;
  }
  ZipFile zippy(file);
  
  File target(argv[2]);
  if (!file.exists()) {
    std::cerr << "Destination doesn't exist:" << std::endl;
	std::cerr << argv[2] << std::endl;
    usage();
    return 1;
  }
  
  zippy.uncompressTo(target, false);   // don't overwrite files that already exist
  return 0;
}


int main(int argc, char** argv) {
  int exitCode = mainImpl(argc, argv);
  return exitCode;
}

