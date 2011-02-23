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
        f = fopen(config.project_path("NetworkTest/.WorkUnit/errors"), "a");
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
  retval = boinc_mkdir(config.project_path("NetworkTest/.WorkUnit/Finished"));
  if (retval)
  {
    log_messages.printf(MSG_CRITICAL, "Can't create NetworkTest/.WorkUnit/Finished directory\n");
    return retval;
  }
  retval = boinc_mkdir(config.project_path("NetworkTest/.WorkUnit/Traces"));
  if (retval)
  {
    log_messages.printf(MSG_CRITICAL, "NetworkTest/.WorkUnit/Traces directory\n");
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
          retval = boinc_copy(fi.path.c_str(), config.project_path("NetworkTest/.WorkUnit/Traces/%s.trace", wu.name));
          if (retval)
            log_messages.printf(MSG_CRITICAL, "[WORKUNIT#%d %s] Can't copy output.trace file : %s -> %s\n",
                                wu.id, wu.name, fi.path.c_str(), config.project_path("NetworkTest/.WorkUnit/Traces/%s.trace", wu.name));

          // update WU status
          juce::OwnedArray<File> foundFiles;
          File directory(config.project_path("NetworkTest/.WorkUnit/InProgress"));
          directory.findChildFiles(foundFiles, File::findFiles, false, strcat(wu.name, ".*"));
          for (unsigned int u = 0 ; u < foundFiles.size(); ++u) {
            if (!foundFiles[u]->moveFileTo(File(config.project_path("NetworkTest/.WorkUnit/Finished"))))
              log_messages.printf(MSG_CRITICAL, "[WORKUNIT#%d %s] Can't copy workunit file : %s -> %s\n",
                                wu.id, wu.name, foundFiles[u]->getFullPathName().toUTF8() , config.project_path("NetworkTest/.WorkUnit/Finished"));
          }

          break;
        }
      }
    }
    else
    {
      char buf[1024];
      sprintf(buf, "%s: 0x%x\n", wu.name, wu.error_mask);
      return write_error(buf);
    }
    return 0;
}
