/*-----------------------------------------.---------------------------------.
| Filename: Rosetta.cpp                    | Rosetta source                  |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 2, 2011  10:55:21 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "Rosetta.h"

#include "../RosettaUtils.h"

using namespace lbcpp;

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
# if 0
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
# else
  jassert(false);
  return VariableVectorPtr();
# endif //! 0
}

# ifdef LBCPP_PROTEIN_ROSETTA

void Rosetta::init(ExecutionContext& eContext, bool verbose, int seed, size_t delay)
{
  //  if (isInPool)
  //    getPoolLock();

  setContext(eContext);

  context->informationCallback(T("Rosetta Id : ") + String((int)id) + T(" initializing..."));

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
  args.add_back(std::string("-in::file::obey_ENDMDL"));
  args.add_back(std::string("true"));

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
  args.add_back(std::string("-run:nodelay"));
  args.add_back(std::string("false"));
  args.add_back(std::string("-run:delay"));
  args.add_back(std::string((const char*)String((int)delay)));


  // out paths
  //  args.add_back(std::string("-out:file"));
  //  args.add_back(std::string("true"));
  //  args.add_back(std::string("-out:path:all"));
  //  args.add_back(std::string((const char*)String(T("/r_o_")
  //      + String(juce::Time::currentTimeMillis()) + T("_") + String((int)id))));
  //  args.add_back(std::string("-out:file:o"));
  //  args.add_back(std::string((const char*)String(T("/ros_o_") + String(
  //      juce::Time::currentTimeMillis()) + T("_") + String((int)id))));
  //  args.add_back(std::string("-out:sf"));
  //  args.add_back(std::string((const char*)String(T("r_score") + String((int)id) + T("-") + String(
  //      (int)nProc) + T("_") + String(juce::Time::currentTimeMillis()) + T(".fsc"))));
  //  args.add_back(std::string("-out:prefix"));
  //  args.add_back(std::string((const char*)String(T("r_pre") + String((int)id) + T("-") + String(
  //      (int)nProc) + T("_") + String(juce::Time::currentTimeMillis()))));
  //  args.add_back(std::string("-out:nooutput"));
  //  args.add_back(std::string("true"));
  //  args.add_back(std::string("-score:output_etables"));
  //  args.add_back(std::string((const char*)String(T("r_et") + String((int)id) + T("-") + String(
  //      (int)nProc) + T("_") + String(juce::Time::currentTimeMillis()))));
  //  args.add_back(std::string("-score:output_etables"));
  //  std::string prefix_et((const char*)String(T("r_et") + String((int)id) + T("-") + String(
  //      (int)nProc) + T("_") + String(juce::Time::currentTimeMillis())));
  //  args.add_back(prefix_et);
  //  args.add_back(std::string("-score:input_etables"));
  //  args.add_back(prefix_et);

  // verbosity
  if (!verbose)
  {
    args.add_back(std::string("-mute"));
    args.add_back(std::string("all"));
  }

  // initialize rosetta
  core::init(args);

  // ensures initialization of database
  core::pose::PoseOP pose = new core::pose::Pose();
  makePoseFromSequence(pose, T("A"));

  context->informationCallback(T("Rosetta Id : ") + String((int)id) + T(" initialized."));

  //  if (isInPool)
  //    releasePoolLock();
}

# else

void Rosetta::init(ExecutionContext& eContext, bool verbose, int seed)
  {jassert(false);}

# endif //! LBCPP_PROTEIN_ROSETTA
