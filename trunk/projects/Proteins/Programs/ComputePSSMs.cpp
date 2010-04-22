/*-----------------------------------------.---------------------------------.
| Filename: ComputePSSMs.cpp               | Compute Protein PSSMs           |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 14:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../ProteinInference/Protein.h"
using namespace lbcpp;

extern void declareProteinClasses();

File psiBlastExecutable(T("C:\\Program Files (x86)\\NCBI\\blast-2.2.23+\\bin\\psiblast.exe"));

using juce::ThreadPoolJob;
using juce::ThreadPool;
using juce::OwnedArray;
using juce::Thread;

class ComputePSSMThreadPoolJob : public ThreadPoolJob
{
public:
  ComputePSSMThreadPoolJob(const File& inputFile, const File& outputFile)
    : ThreadPoolJob(inputFile.getFileNameWithoutExtension()), inputFile(inputFile), outputFile(outputFile) {}

  virtual JobStatus runJob()
  {
    ProteinPtr protein = Object::createFromFileAndCast<Protein>(inputFile);
    if (!protein)
      return jobHasFinished;
    
    File fastaFile = File::createTempFile(T("fasta"));
    protein->saveToFASTAFile(fastaFile);

    String commandArguments = T("-db nr -out_ascii_pssm ") + outputFile.getFullPathName() + T(" -num_iterations 3 -query ") + fastaFile.getFullPathName();
    std::cout << "psiblast " << commandArguments << std::endl;
    psiBlastExecutable.startAsProcess(commandArguments);
    std::cout << "psiblast " << commandArguments << " => finished" << std::endl;
    fastaFile.deleteFile();
    exit(1);
    return jobHasFinished;
  }

protected:
  File inputFile;
  File outputFile;
};

void computePSSMs(const File& inputDirectory, const File& outputDirectory, int numCpus)
{
  OwnedArray<File> inputFiles;
  inputDirectory.findChildFiles(inputFiles, File::findFiles, false, T("*.protein"));
  
  ThreadPool threadPool(numCpus);
  for (int i = 0; i < inputFiles.size(); ++i)
  {
    File inputFile = *inputFiles[i];
    File outputFile = outputDirectory.getChildFile(inputFile.getFileNameWithoutExtension() + T(".pssm"));
    threadPool.addJob(new ComputePSSMThreadPoolJob(inputFile, outputFile));
  }
  while (threadPool.getNumJobs() > 0)
  {
    std::cout << "Num jobs running or queued: " << threadPool.getNumJobs() << std::endl;
    Thread::sleep(1000);
  }
}

int main(int argc, char* argv[])
{
  declareProteinClasses();
  juce::initialiseJuce_NonGUI();

  if (argc < 3)
  {
    std::cout << "Usage: " << argv[0] << " inputdir outputdir" << std::endl;
    return 1;
  }

  File cwd = File::getCurrentWorkingDirectory();
  File inputDir = cwd.getChildFile(argv[1]);
  if (!inputDir.isDirectory())
  {
    std::cerr << argv[1] << " is not a directory" << std::endl;
    return 1;
  }

  File outputDir = cwd.getChildFile(argv[2]);
  if (!outputDir.isDirectory())
  {
    std::cerr << argv[2] << " is not a directory" << std::endl;
    return 1;
  }
 
  int numCpus = 1;//juce::SystemStats::getNumCpus();
  std::cout << numCpus << " CPUs" << std::endl;
  computePSSMs(inputDir, outputDir, numCpus);
  return 0;
}
