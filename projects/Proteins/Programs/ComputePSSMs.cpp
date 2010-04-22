/*-----------------------------------------.---------------------------------.
| Filename: ComputePSSMs.cpp               | Compute Protein PSSMs           |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 14:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../../juce/ConsoleProcess.h"
#include "../ProteinInference/Protein.h"
using namespace lbcpp;

extern void declareProteinClasses();

File psiBlastExecutable(T("C:\\Program Files (x86)\\NCBI\\blast-2.2.23+\\bin\\psiblast.exe"));

using juce::ThreadPoolJob;
using juce::ThreadPool;
using juce::OwnedArray;
using juce::Thread;
using juce::ConsoleProcess;

class ComputePSSMThreadPoolJob : public ThreadPoolJob
{
public:
  ComputePSSMThreadPoolJob(const File& inputFile, const File& outputFile)
    : ThreadPoolJob(inputFile.getFileNameWithoutExtension()), inputFile(inputFile), outputFile(outputFile) {}

  virtual JobStatus runJob()
  {
    ProteinPtr protein = Protein::createFromFile(inputFile);
    if (!protein)
      return jobHasFinished;
    
    File fastaFile = File::createTempFile(T("fasta"));
    protein->saveToFASTAFile(fastaFile);

    if (outputFile.exists())
      outputFile.deleteFile();

    String commandArguments = T(" -db nr -num_iterations 3 -query ") + fastaFile.getFullPathName().quoted() + T(" -out_ascii_pssm ") + outputFile.getFullPathName().quoted();
    std::cout << "psiblast " << commandArguments << std::endl;
    ConsoleProcess* process = ConsoleProcess::create(psiBlastExecutable.getFullPathName(), commandArguments, File::getCurrentWorkingDirectory().getFullPathName());
    if (!process)
      return jobHasFinished;
   
    int returnCode;
    while (!shouldExit())
    {
      String str;
      bool nothingToRead = !process->readStandardOutput(str);
      if (str.isNotEmpty())
        std::cout << "+" << std::flush;

      if (!process->isRunning(returnCode))
      {
        delete process;
        break;
      }
      if (!nothingToRead)
      {
        std::cout << "." << std::flush;
        Thread::sleep(500);
      }
    }

    std::cout << "psiblast " << commandArguments << " => finished, return code = " << returnCode << std::endl;
    fastaFile.deleteFile();
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
    Thread::sleep(5000);
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
 
  int numCpus = juce::SystemStats::getNumCpus();
  std::cout << numCpus << " CPUs" << std::endl;
  computePSSMs(inputDir, outputDir, juce::jmax(1, numCpus - 1));
  return 0;
}
