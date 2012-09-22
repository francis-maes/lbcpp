/*-----------------------------------------.---------------------------------.
| Filename: LBCppAssimilator.cpp           | BOINC assimilator               |
| Author  : Arnaud Schoofs                 | Implementation based on :       |
| Started : 23/02/2011 12:00               | sample_assimilator.cpp          |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

// LBCpp include
#include <lbcpp/common.h>

// Common include
#include <vector>

// BOINC includes
#include <boinc_db.h>
#include <error_numbers.h>
#include <filesys.h>
#include <sched_msgs.h>
#include <validate_util.h>
#include <sched_config.h>

int write_error(char* p)
{
    static FILE* f = 0;
    if (!f)
    {
        f = fopen(config.project_path("Network/.WorkUnit/errors"), "a");
        if (!f)
          return ERR_FOPEN;
    }
    fprintf(f, "%s", p);
    fflush(f);
    return 0;
}

int assimilate_handler(WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result)
{
  int retval;

  // Check directories
  retval = boinc_mkdir(config.project_path("Network/.WorkUnit/Finished"));
  if (retval)
  {
    log_messages.printf(MSG_CRITICAL, "Can't create Network/.WorkUnit/Finished directory\n");
    return retval;
  }
  retval = boinc_mkdir(config.project_path("Network/.WorkUnit/Traces"));
  if (retval)
  {
    log_messages.printf(MSG_CRITICAL, "Network/.WorkUnit/Traces directory\n");
    return retval;
  }
  retval = boinc_mkdir(config.project_path("Network/.WorkUnit/Errors"));
  if (retval)
  {
    log_messages.printf(MSG_CRITICAL, "Network/.WorkUnit/Errors directory\n");
    return retval;
  }

  if (wu.canonical_resultid)
  {
      std::vector<FILE_INFO> output_files;
      get_output_file_infos(canonical_result, output_files);

      for (unsigned int i = 0; i < output_files.size(); ++i)
      {
        FILE_INFO& fi = output_files[i];

        // if (!fi.no_validate) --> output.trace file (only one trace file!)
        if (!fi.no_validate)
        {
          // copy output.trace file
          retval = boinc_copy(fi.path.c_str(), config.project_path("Network/.WorkUnit/Traces/%s.trace", wu.name));
          if (retval)
          {
            log_messages.printf(MSG_CRITICAL, "[WORKUNIT#%d %s] Can't copy output.trace file : %s -> %s\n",
                                wu.id, wu.name, fi.path.c_str(), config.project_path("Network/.WorkUnit/Traces/%s.trace", wu.name));
            return 1;
          }

          // update WU status
          File fileToMove(config.project_path("Network/.WorkUnit/InProgress/%s.workUnit", wu.name));
          if (!fileToMove.moveFileTo(File(config.project_path("Network/.WorkUnit/Finished/%s.workUnit", wu.name))))
          {
          	log_messages.printf(MSG_CRITICAL, "[WORKUNIT#%d %s] Can't move workunit file : %s -> %s\n", wu.id, wu.name, fileToMove.getFullPathName().toUTF8() , File(config.project_path("Network/.WorkUnit/Finished/%s.workUnit", wu.name)).getFullPathName().toUTF8());
            return 1;
          }
          
          break;  // only one file to assimilate (trace)
        }
      }
    }
    else
    {
      File traceToCopy(config.project_path("Network/.WorkUnit/fakeTrace.trace"));
      File fileToCopy(config.project_path("Network/.WorkUnit/InProgress/%s.workUnit", wu.name));
      if (!traceToCopy.copyFileTo(File(config.project_path("Network/.WorkUnit/Traces/%s.trace", wu.name))))
        log_messages.printf(MSG_CRITICAL, "[WORKUNIT#%d %s] Can't copy fakeTrace file : %s -> %s\n", wu.id, wu.name, traceToCopy.getFullPathName().toUTF8() , config.project_path("Network/.WorkUnit/Traces/%s.trace", wu.name));
      else
      {
        if (!fileToCopy.copyFileTo(File(config.project_path("Network/.WorkUnit/Finished/%s.workUnit", wu.name))))
          log_messages.printf(MSG_CRITICAL, "[WORKUNIT#%d %s] Can't copy workunit file : %s -> %s\n", wu.id, wu.name, fileToCopy.getFullPathName().toUTF8() , File(config.project_path("Network/.WorkUnit/Finished/%s.workUnit", wu.name)).getFullPathName().toUTF8());
      }
      if (!fileToCopy.moveFileTo(File(config.project_path("Network/.WorkUnit/Errors/%s.workUnit", wu.name))))
        log_messages.printf(MSG_CRITICAL, "[WORKUNIT#%d %s] Can't move workunit file : %s -> %s\n", wu.id, wu.name, fileToCopy.getFullPathName().toUTF8() , config.project_path("Network/.WorkUnit/Errors/%s.workUnit", wu.name));

      char buf[1024];
      sprintf(buf, "%s: 0x%x\n", wu.name, wu.error_mask);
      return write_error(buf);
    }
    return 0;
}
