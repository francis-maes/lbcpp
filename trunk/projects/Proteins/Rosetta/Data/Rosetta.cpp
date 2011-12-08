/*-----------------------------------------.---------------------------------.
| Filename: Rosetta.cpp                    | Rosetta source                  |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 2, 2011  10:55:21 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "Rosetta.h"
#include "precompiled.h"

namespace lbcpp
{

Rosetta::Rosetta()
  : context(NULL), ownLock(new CriticalSection()), poolLock(NULL), nProc(1), id(0), isInPool(false) {}
Rosetta::~Rosetta()
{
  delete ownLock;
  if (isInPool && (id == 0))
    delete poolLock;
}

void Rosetta::setContext(ExecutionContext& context)
  {this->context = &context;}

void Rosetta::getLock()
  {ownLock->enter();}
void Rosetta::releaseLock()
  {ownLock->exit();}
void Rosetta::getPoolLock()
  {poolLock->enter();}
void Rosetta::releasePoolLock()
  {poolLock->exit();}

VariableVectorPtr Rosetta::createRosettaPool(ExecutionContext& context, size_t size)
{
  VariableVectorPtr pool = new VariableVector(size);
  CriticalSection* pl = new CriticalSection();

  context.enterScope(T("Creating Rosetta cores"));
  for (size_t i = 0; i < size; i++)
  {
    // initialize rosetta object in a pool
    RosettaPtr r = new Rosetta();
    r->poolLock = pl;
    r->id = i;
    r->nProc = size;
    r->isInPool = true;

    pool->setElement(i, r);
    context.progressCallback(new ProgressionState(i, size, T("cores")));
  }
  context.leaveScope();

  return pool;
}

void Rosetta::init(ExecutionContext& eContext, bool verbose, int seed)
{
  # ifdef LBCPP_PROTEIN_ROSETTA
  if (isInPool)
    getPoolLock();

  setContext(eContext);

  jassert(context.get() != NULL);
  jassert((id >= 0) && (id < nProc));

  // init
  utility::vector0<std::string> args;
  args.add_back(std::string("./main"));

  // rosetta db
  args.add_back(std::string("-database"));
  juce::File dbDirectory = context->getFile(T("rosetta_database"));
  jassert(dbDirectory != File::nonexistent);
  args.add_back(std::string((const char*)dbDirectory.getFullPathName()));

  // seeds of random devices
  args.add_back(std::string("-run:use_time_as_seed"));
  args.add_back(std::string("false"));
  args.add_back(std::string("-run:constant_seed"));
  args.add_back(std::string("true"));
  args.add_back(std::string("-run:jran"));
  if (!isInPool && (seed >= 0))
    args.add_back(std::string((const char*)String(seed)));
  else
    args.add_back(std::string((const char*)String((int)id)));

  // processes
  args.add_back(std::string("-run:interactive"));
  args.add_back(std::string("true"));
  args.add_back(std::string("-run:nproc"));
  args.add_back(std::string((const char*)String((int)nProc)));
  args.add_back(std::string("-run:proc_id"));
  args.add_back(std::string((const char*)String((int)id)));

  // out paths
  args.add_back(std::string("-out:file"));
  args.add_back(std::string("true"));
  args.add_back(std::string("-out:file:o"));
  args.add_back(std::string((const char*)String(T("tmpRos_o_") + String(
      juce::Time::currentTimeMillis()) + T("_") + String((int)id))));

  // in paths
  args.add_back(std::string("-in:path"));
  args.add_back(std::string((const char*)String(T("tmpRos_i_") + String(
      juce::Time::currentTimeMillis()) + T("_") + String((int)id))));

  // verbosity
  if (!verbose)
  {
    args.add_back(std::string("-mute"));
    args.add_back(std::string("all"));
  }

  // initialize rosetta
  core::init(args);
  context->informationCallback(T("Rosetta initialized. Id : ") + String((int)id));

//  for (size_t i = 0; i <= args.u(); i++)
//    std::cout << args[i] << std::endl;

  if (isInPool)
    releasePoolLock();
  # endif //! LBCPP_PROTEIN_ROSETTA
}

}; /* namespace lbcpp */
