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
          	log_messages.printf(MSG_CRITICAL, "[WORKUNIT#%d %s] Can't copy workunit file : %s -> %s\n", wu.id, wu.name, fileToMove.getFullPathName().toUTF8() , File(config.project_path("Network/.WorkUnit/Finished/%s.workUnit", wu.name)).getFullPathName().toUTF8());
            return 1;
          }

	/*
          juce::OwnedArray<File> foundFiles;
          File directory(config.project_path("Network/.WorkUnit/InProgress"));
          directory.findChildFiles(foundFiles, File::findFiles, false, strcat(wu.name, ".*"));
          if (foundFiles.size() == 0)
          {
            log_messages.printf(MSG_CRITICAL, "[WORKUNIT#%d %s] Can't find InProgress file\n", wu.id, wu.name);
            return 1;
          }
          if (foundFiles.size() > 1)
          {
            log_messages.printf(MSG_CRITICAL, "[WORKUNIT#%d %s] More than one InProgress file\n", wu.id, wu.name);
            return 1;
          }
          */


          // Only one file found
          /*
          String newLocation("NetworkTest/.WorkUnit/Finished/");
          newLocation += foundFiles[0]->getFileName();
          if (!foundFiles[0]->moveFileTo(File(config.project_path(newLocation.toUTF8()))))
          {
            log_messages.printf(MSG_CRITICAL, "[WORKUNIT#%d %s] Can't copy workunit file : %s -> %s\n", wu.id, wu.name, foundFiles[0]->getFullPathName().toUTF8() , config.project_path("NetworkTest/.WorkUnit/Finished"));
            return 1;
          }
          */

          break;  // only one file to assimilate (trace)
        }
      }
    }
    else
    {
      File fileToCopy(config.project_path("Network/.WorkUnit/InProgress/%s.workUnit", wu.name));
      if (!fileToCopy.copyFileTo(File(config.project_path("Network/.WorkUnit/Errors/%s.workUnit", wu.name))))
        log_messages.printf(MSG_CRITICAL, "[WORKUNIT#%d %s] Can't copy workunit file : %s -> %s\n", wu.id, wu.name, fileToCopy.getFullPathName().toUTF8() , config.project_path("Network/.WorkUnit/Errors/%s.workUnit", wu.name));

      // TODO : generate trace with socre = 0 in finished !!! 
      char buf[1024];
      sprintf(buf, "%s: 0x%x\n", wu.name, wu.error_mask);
      return write_error(buf);
    }
    return 0;
}
