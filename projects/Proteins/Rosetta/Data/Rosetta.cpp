/*-----------------------------------------.---------------------------------.
| Filename: Rosetta.cpp                    | Rosetta source                  |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 2, 2011  10:55:21 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "Rosetta.h"

# ifdef LBCPP_PROTEIN_ROSETTA
#  undef T
#  include <core/chemical/ChemicalManager.hh>
#  include <core/chemical/util.hh>
#  include <core/init.hh>
#  include <core/pose/Pose.hh>
#  include <utility/vector0.hh>
#  define T JUCE_T
# endif // LBCPP_PROTEIN_ROSETTA

using namespace lbcpp;

Rosetta::Rosetta()
  : context(NULL), ownLock(new CriticalSection()) {}

Rosetta::~Rosetta()
  {delete ownLock;}

void Rosetta::setContext(ExecutionContext& context)
  {this->context = &context;}

void Rosetta::getLock()
  {ownLock->enter();}
void Rosetta::releaseLock()
  {ownLock->exit();}

# ifdef LBCPP_PROTEIN_ROSETTA

void Rosetta::init(ExecutionContext& eContext, bool verbose, size_t id, size_t delay)
{
  setContext(eContext);

  context->informationCallback(T("Rosetta Id : ") + String((int)id) + T(" initializing..."));

  jassert(context.get() != NULL);

  // init
  utility::vector0<std::string> args;
  args.add_back(std::string("./main"));

  // rosetta db
  args.add_back(std::string("-database"));
  juce::File rootDirectoryToSearch = context->getProjectDirectory();
  juce::File dbDirectory = context->getFile(T("rosetta_database"));
  while (!dbDirectory.exists())
  {
    rootDirectoryToSearch = rootDirectoryToSearch.getParentDirectory();
    dbDirectory = rootDirectoryToSearch.getChildFile(T("rosetta_database"));
  }
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
  args.add_back(std::string((const char*)String((int)id)));

  // processes
  args.add_back(std::string("-run:interactive"));
  args.add_back(std::string("true"));
  args.add_back(std::string("-run:nproc"));
  args.add_back(std::string((const char*)String((int)1)));
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
  core::chemical::make_pose_from_sequence(*pose, ("A"),
      core::chemical::ChemicalManager::get_instance()->nonconst_residue_type_set("fa_standard"));

  context->informationCallback(T("Rosetta Id : ") + String((int)id) + T(" initialized."));
}

# else

void Rosetta::init(ExecutionContext& context, bool verbose, size_t id, size_t delay)
  {jassert(false);}

# endif //! LBCPP_PROTEIN_ROSETTA
