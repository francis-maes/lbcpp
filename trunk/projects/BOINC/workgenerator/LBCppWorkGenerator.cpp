/*-----------------------------------------.---------------------------------.
| Filename: LBCppValidator.cpp             | BOINC workgenerator             |
| Author  : Arnaud Schoofs                 | Implementation based on :       |
| Started : 23/02/2011 21:44               | sample_work_generator           |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

// LBCpp include
#include <lbcpp/common.h>

// BOINC includes
#include <boinc_db.h>
#include <error_numbers.h>
#include <backend_lib.h>
#include <sched_config.h>
#include <sched_util.h>
#include <sched_msgs.h>

#define REPLICATION_FACTOR  2

// Default values
char* app_name = (char*) "LBCppBeta";
char* in_template_file = (char*) "LBCPP_wu";
char* out_template_file = (char*) "LBCPP_result";

char* in_template;
DB_APP app;

// create one new job
int make_job(File* file) {
  DB_WORKUNIT wu;
  char name[256], path[256];
  const char* infiles[1];
  int retval;

  // get WU's name
  sprintf(name, "%s", file->getFileNameWithoutExtension().toUTF8());

  // Put the WU's file in the right place in the download dir hierarchy
  retval = config.download_path(name, path);
  if (retval)
  {
    log_messages.printf(MSG_CRITICAL, "can't find right place in the download dir hierarchy for file: %s\n", name);
    return retval;
  }
  if (!file->copyFileTo(File(path)))
  {
    log_messages.printf(MSG_CRITICAL, "can't copy file: %s ---> %s\n", file->getFullPathName().toUTF8(), path);
    return 1;
  }

  // Update LBCpp WU's status
  String newLocation("Network/.WorkUnit/WorkUnits/");
  newLocation += file->getFileName();
  if (!file->moveFileTo(File(config.project_path(newLocation.toUTF8()))))
  {
    log_messages.printf(MSG_CRITICAL, "can't move file: %s ---> %s\n", file->getFullPathName().toUTF8(), config.project_path(newLocation.toUTF8()));
    return 1;
  }

  // Fill in the job parameters
  // TODO : use that kind of parameters in LBCpp for the request
  wu.clear();
  wu.appid = app.id;
  strcpy(wu.name, name);
  wu.rsc_fpops_est = 1e12;
  wu.rsc_fpops_bound = 1e14;
  wu.rsc_memory_bound = 1e8;
  wu.rsc_disk_bound = 1e8;
  wu.delay_bound = 3*86400;
  wu.min_quorum = REPLICATION_FACTOR;
  wu.target_nresults = REPLICATION_FACTOR;
  wu.max_error_results = REPLICATION_FACTOR*4;
  wu.max_total_results = REPLICATION_FACTOR*8;
  wu.max_success_results = REPLICATION_FACTOR*4;
  infiles[0] = name;

  // Register the job with BOINC
  sprintf(path, "templates/%s", out_template_file);
  return create_work(
        wu,
        in_template,
        path,
        config.project_path(path),
        infiles,
        1,
        config
    );
}

void main_loop()
{
  int retval;
  juce::OwnedArray<File> foundFiles;
  File directory(config.project_path("Network/.WorkUnit/PreProcessing"));

  while (1) {
    check_stop_daemons();

    directory.findChildFiles(foundFiles, File::findFiles, false);
    unsigned int njobs = foundFiles.size();
    if (njobs > 0)
    {
      log_messages.printf(MSG_DEBUG, "Making %d jobs\n", njobs);
      for (unsigned int i = 0; i < njobs; ++i)
      {
        retval = make_job(foundFiles[i]);
        if (retval)
        {
          log_messages.printf(MSG_CRITICAL, "can't make job (%s): %s\n", foundFiles[i]->getFileName().toUTF8(), boincerror(retval));
          exit(retval);
        }
      }
      foundFiles.clear();
    }
    sleep(10);
  }
}

void usage(char *name)
{
  fprintf(stderr, "This is the LBCpp work generator.\n"
          "This work generator has the following properties:\n"
          "- Runs as a daemon\n"
          "- Creates a job for each file (WU) found in Network/.WorkUnit/PreProcessing/\n\n"
          "Usage: %s [OPTION]...\n\n"
          "Options:\n"
          "  [ --app X                Application name (default: LBCppBeta)\n"
          "  [ --in_template_file     Input template (default: LBCPP_wu)\n"
          "  [ --out_template_file    Output template (default: LBCPP_result)\n"
          "  [ -d X ]                 Sets debug level to X.\n"
          "  [ -h | --help ]          Shows this help text.\n",
          name
  );
}

int main(int argc, char** argv)
{
  int i, retval;
  char buf[256];

  for (i=1; i<argc; i++)
  {
    if (is_arg(argv[i], "d"))
    {
      if (!argv[++i])
      {
        log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
        usage(argv[0]);
        exit(1);
      }
      int dl = atoi(argv[i]);
      log_messages.set_debug_level(dl);
      if (dl == 4) g_print_queries = true;
    }
    else if (!strcmp(argv[i], "--app"))
      app_name = argv[++i];
    else if (!strcmp(argv[i], "--in_template_file"))
      in_template_file = argv[++i];
    else if (!strcmp(argv[i], "--out_template_file"))
      out_template_file = argv[++i];
    else if (is_arg(argv[i], "h") || is_arg(argv[i], "help"))
    {
      usage(argv[0]);
      exit(0);
    }
    else
    {
      log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
      usage(argv[0]);
      exit(1);
    }
  }

  retval = config.parse_file();
  if (retval)
  {
    log_messages.printf(MSG_CRITICAL, "Can't parse config.xml: %s\n", boincerror(retval));
    exit(1);
  }

  retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
  if (retval)
  {
    log_messages.printf(MSG_CRITICAL, "can't open db\n");
    exit(1);
  }

  sprintf(buf, "where name='%s'", app_name);
  if (app.lookup(buf))
  {
    log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name);
    exit(1);
  }

  sprintf(buf, "templates/%s", in_template_file);
  if (read_file_malloc(config.project_path(buf), in_template))
  {
    log_messages.printf(MSG_CRITICAL, "can't read input template %s\n", buf);
    exit(1);
  }

  log_messages.printf(MSG_NORMAL, "Starting\n");
  main_loop();
}

