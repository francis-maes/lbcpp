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
File dsspExecutable(T("C:\\Projets\\LBC++\\projects\\temp\\DSSPCMBI.EXE"));

using juce::ThreadPoolJob;
using juce::ThreadPool;
using juce::OwnedArray;
using juce::Thread;
using juce::ConsoleProcess;

class ConsoleProcessThreadPoolJob : public ThreadPoolJob
{
public:
  ConsoleProcessThreadPoolJob(const String& name, const File& executable)
    : ThreadPoolJob(name), executable(executable){}

  virtual JobStatus runJob()
  {
    std::cout << executable.getFileName() << " " << commandArguments << std::endl;
    ConsoleProcess* process = ConsoleProcess::create(executable.getFullPathName(), commandArguments, File::getCurrentWorkingDirectory().getFullPathName());
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
    return jobHasFinished;
  }

protected:
  File executable;
  String commandArguments;
};

class ComputeDSSPThreadPoolJob : public ConsoleProcessThreadPoolJob
{
public:
  ComputeDSSPThreadPoolJob(const File& inputFile, const File& outputFile)
    : ConsoleProcessThreadPoolJob(inputFile.getFileNameWithoutExtension() + T(" dssp"), dsspExecutable),
        inputFile(inputFile), outputFile(outputFile) {}

  virtual JobStatus runJob()
  {
    ProteinPtr protein = Protein::createFromFile(inputFile);
    if (!protein)
      return jobHasFinished;
    
    File pdbFile = File::createTempFile(T("pdb"));
    protein->saveToPDBFile(pdbFile);
    if (outputFile.exists())
      outputFile.deleteFile();
    commandArguments = pdbFile.getFullPathName().quoted() + T(" ") + outputFile.getFullPathName().quoted();
    JobStatus res = ConsoleProcessThreadPoolJob::runJob();
    pdbFile.deleteFile();
    return res;
  }

private:
  File inputFile;
  File outputFile;
};

class ComputePSSMThreadPoolJob : public ConsoleProcessThreadPoolJob
{
public:
  ComputePSSMThreadPoolJob(const File& inputFile, const File& outputFile)
    : ConsoleProcessThreadPoolJob(inputFile.getFileNameWithoutExtension() + T(" pssm"), psiBlastExecutable),
        inputFile(inputFile), outputFile(outputFile) {}

  virtual JobStatus runJob()
  {
    ProteinPtr protein = Protein::createFromFile(inputFile);
    if (!protein)
      return jobHasFinished;
    
    File fastaFile = File::createTempFile(T("fasta"));
    protein->saveToFASTAFile(fastaFile);

    if (outputFile.exists())
      outputFile.deleteFile();

    commandArguments = T(" -db nr -num_iterations 3 -query ") + fastaFile.getFullPathName().quoted()
      + T(" -out_ascii_pssm ") + outputFile.getFullPathName().quoted();

    JobStatus res = ConsoleProcessThreadPoolJob::runJob();
    fastaFile.deleteFile();
    return res;
  }

private:
  File inputFile;
  File outputFile;
};

void computePSSMsAndDSSPs(const File& inputDirectory, const File& pssmOutputDirectory, const File& dsspOutputDirectory, int numCpus)
{
  OwnedArray<File> inputFiles;
  inputDirectory.findChildFiles(inputFiles, File::findFiles, false, T("*.protein"));
  
  ThreadPool threadPool(numCpus);
  for (int i = 0; i < inputFiles.size(); ++i)
  {
    File inputFile = *inputFiles[i];
    if (pssmOutputDirectory.exists())
    {
      File outputFile = pssmOutputDirectory.getChildFile(inputFile.getFileNameWithoutExtension() + T(".pssm"));
      threadPool.addJob(new ComputePSSMThreadPoolJob(inputFile, outputFile));
    }
    if (dsspOutputDirectory.exists())
    {
      File outputFile = dsspOutputDirectory.getChildFile(inputFile.getFileNameWithoutExtension() + T(".dssp"));
      threadPool.addJob(new ComputeDSSPThreadPoolJob(inputFile, outputFile));
    }
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

  if (argc < 4)
  {
    std::cout << "Usage: " << argv[0] << " inputdir outputPSSMDir outuptDSSPdir" << std::endl;
    return 1;
  }

  File cwd = File::getCurrentWorkingDirectory();
  File inputDir = cwd.getChildFile(argv[1]);
  if (!inputDir.isDirectory())
  {
    std::cerr << argv[1] << " is not a directory" << std::endl;
    return 1;
  }

  File outputPSSMDir = cwd.getChildFile(argv[2]);
  File outputDSSPDir = cwd.getChildFile(argv[3]);

  int numCpus = juce::SystemStats::getNumCpus();
  std::cout << numCpus << " CPUs" << std::endl;
  computePSSMsAndDSSPs(inputDir, outputPSSMDir, outputDSSPDir, juce::jmax(1, numCpus - 1));
  return 0;
}
