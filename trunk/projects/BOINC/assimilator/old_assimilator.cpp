#include <vector>
#include <string>
#include <cstdlib>
#include <algorithm>
#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "sched_config.h"

#include <juce_amalgamated.h>
#include <iostream>
#include <sstream>

using std::stringstream;
using std::max;

int write_error(char* p) {
    static FILE* f = 0;
    if (!f) {
        f = fopen(config.project_path("error_results/errors"), "a");
        if (!f) return ERR_FOPEN;
    }
    fprintf(f, "%s", p);
    fflush(f);
    return 0;
}

int assimilate_handler(WORKUNIT& wu, vector<RESULT>& results, RESULT& canonical_result) {
    int retval;
    char buf[1024];

    retval = boinc_mkdir(config.project_path("results"));
    if (retval) return retval;

    if (wu.canonical_resultid) {
        vector<FILE_INFO> output_files;
        get_output_file_infos(canonical_result, output_files);

        FILE_INFO& fi = output_files[0];
        const char *copy_path = config.project_path("results/%s", wu.name);

        retval = boinc_copy(fi.path.c_str() , copy_path);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "%s cannot be copied into %s\n\n", fi.path.c_str(), copy_path);
            return -1;
        }

        File file(copy_path);
        String name = file.getFileName();

        XmlDocument xmlDoc(file);
        XmlElement* mainElement = xmlDoc.getDocumentElement();
        if (mainElement == 0) {
            log_messages.printf(MSG_CRITICAL, "Cannot get root element of : %s\n", name.toUTF8());
            return -1;
        }

        if (mainElement->getTagName().compare(String("result")) != 0) {
            delete mainElement;
            log_messages.printf(MSG_CRITICAL,
                "Root tag is not <result> in: %s\n",
                name.toUTF8()
                );
                return -1;
        }

        XmlElement* parameters = mainElement->getChildByName(String("parameters"));
        bool includeBoundsProximity;
        bool includeGlobalHistograms;
        bool includeLocalAAWindow;
        bool includeLocalHistograms;
        double l1RegularizerWeight;
        double l2RegularizerWeight;
        double learningRate;
        double learningRateDecrease;
        String learningStepsFrequency;
        String multiClassInference;
        int numIntervalsPerEntropy;
        int numIntervalsPerPositiveInteger;
        int numIntervalsPerProbability;
        String randomizationFrequency;
        String regularizerFrequency;
        bool restoreBestParametersWhenFinished;

        forEachXmlChildElementWithTagName(*parameters, parameter, String("param")) {
            if ((parameter->getStringAttribute(String("name"))).compare(String("includeBoundsProximity")) == 0) {
                String test = parameter->getStringAttribute(String("value"));
                if (test.compare(String("yes")) == 0)
                    includeBoundsProximity = true;
                else
                    includeBoundsProximity = false;
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("includeGlobalHistograms")) == 0) {
                String test = parameter->getStringAttribute(String("value"));
                if (test.compare(String("yes")) == 0)
                    includeGlobalHistograms = true;
                else
                    includeGlobalHistograms = false;
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("includeLocalAAWindow")) == 0) {
                String test = parameter->getStringAttribute(String("value"));
                if (test.compare(String("yes")) == 0)
                    includeLocalAAWindow = true;
                else
                    includeLocalAAWindow = false;
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("includeLocalHistograms")) == 0) {
                String test = parameter->getStringAttribute(String("value"));
                if (test.compare(String("yes")) == 0)
                    includeLocalHistograms = true;
                else
                    includeLocalHistograms = false;
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("l1RegularizerWeight")) == 0) {
                l1RegularizerWeight = parameter->getDoubleAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("l2RegularizerWeight")) == 0) {
                l2RegularizerWeight = parameter->getDoubleAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("learningRate")) == 0) {
                learningRate = parameter->getDoubleAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("learningRateDecrease")) == 0) {
                learningRateDecrease = parameter->getDoubleAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("learningStepsFrequency")) == 0) {
                learningStepsFrequency = parameter->getStringAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("multiClassInference")) == 0) {
                multiClassInference = parameter->getStringAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("numIntervalsPerEntropy")) == 0) {
                numIntervalsPerEntropy = parameter->getIntAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("numIntervalsPerPositiveInteger")) == 0) {
                numIntervalsPerPositiveInteger = parameter->getIntAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("numIntervalsPerProbability")) == 0) {
                numIntervalsPerProbability = parameter->getIntAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("randomizationFrequency")) == 0) {
                randomizationFrequency = parameter->getStringAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("regularizerFrequency")) == 0) {
                regularizerFrequency = parameter->getStringAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("restoreBestParametersWhenFinished")) == 0) {
                String test = parameter->getStringAttribute(String("value"));
                if (test.compare(String("yes")) == 0)
                    restoreBestParametersWhenFinished = true;
                else
                    restoreBestParametersWhenFinished = false;
            }
        }

        // TODO check
        XmlElement* scores = mainElement->getChildByName(String("scores"));
        XmlElement* score = scores->getChildByName(String("score"));    // TODO gerer plusieurs scores
        double x = score->getDoubleAttribute(String("value"));
        delete mainElement;


        stringstream oss;
        oss << "INSERT INTO results "
            << "(name, app_version_num, score, multiClassInference, randomizationFrequency, learningStepsFrequency, learningRate, learningRateDecrease, regularizerFrequency, "
            << "l1RegularizerWeight, l2RegularizerWeight, restoreBestParametersWhenFinished, numIntervalsPerProbability, numIntervalsPerPositiveInteger, numIntervalsPerEntropy, "
            << "includeGlobalHistograms, includeLocalHistograms, includeLocalAAWindow, includeBoundsProximity)"
            <<  " VALUES ("
            << "'"<< name.toUTF8() << "', "
            << "'"<< canonical_result.app_version_num << "', "
            << "'"<< x << "', "
            << "'"<< multiClassInference.toUTF8() << "', "
            << "'"<< randomizationFrequency.toUTF8() << "', "
            << "'"<< learningStepsFrequency.toUTF8() << "', "
            << "'"<< learningRate << "', "
            << "'"<< learningRateDecrease << "', "
            << "'"<< regularizerFrequency.toUTF8() << "', "
            << "'"<< l1RegularizerWeight << "', "
            << "'"<< l2RegularizerWeight << "', "
            << "'"<< restoreBestParametersWhenFinished << "', "
            << "'"<< numIntervalsPerProbability << "', "
            << "'"<< numIntervalsPerPositiveInteger << "', "
            << "'"<< numIntervalsPerEntropy << "', "
            << "'"<< includeGlobalHistograms << "', "
            << "'"<< includeLocalHistograms << "', "
            << "'"<< includeLocalAAWindow << "', "
            << "'"<< includeBoundsProximity << "')";

        MYSQL *conn;
        conn = mysql_init(NULL);
        if (conn == NULL) {
            log_messages.printf(MSG_CRITICAL, "Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
            return -1;
        }

        if (mysql_real_connect(conn, "localhost", "boincadm", "002230", "BoincEvaluator", 0, NULL, 0) == NULL) {
            log_messages.printf(MSG_CRITICAL, "Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
            return -1;
        }

        if (mysql_query(conn, oss.str().c_str())) {
            log_messages.printf(MSG_CRITICAL, "Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
            mysql_close(conn);
            return -1;
        }
        mysql_close(conn);
        oss.clear();
        return 0;

    } else {
        log_messages.printf(MSG_CRITICAL, "Too many errors for : %s\n", wu.name);

        char path[255];
        retval = config.download_path(wu.name, path);
        if (retval) return retval;

        retval = boinc_mkdir(config.project_path("error_results"));
        if (retval) return retval;

        const char *copy_path = config.project_path("error_results/%s", wu.name);
        retval = boinc_copy(path , copy_path);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "%s cannot be copied into %s\n\n", path, copy_path);
            return -1;
        }

        File file(copy_path);
        String name = file.getFileName();

        XmlDocument xmlDoc(file);
        XmlElement* mainElement = xmlDoc.getDocumentElement();
        if (mainElement == 0) {
            log_messages.printf(MSG_CRITICAL, "Cannot get root element of : %s\n", name.toUTF8());
            return -1;
        }

        if (mainElement->getTagName().compare(String("result")) != 0) {
            delete mainElement;
            log_messages.printf(MSG_CRITICAL,
                "Root tag is not <result> in: %s\n",
                name.toUTF8()
                );
                return -1;
        }

        XmlElement* parameters = mainElement->getChildByName(String("parameters"));
        bool includeBoundsProximity;
        bool includeGlobalHistograms;
        bool includeLocalAAWindow;
        bool includeLocalHistograms;
        double l1RegularizerWeight;
        double l2RegularizerWeight;
        double learningRate;
        double learningRateDecrease;
        String learningStepsFrequency;
        String multiClassInference;
        int numIntervalsPerEntropy;
        int numIntervalsPerPositiveInteger;
        int numIntervalsPerProbability;
        String randomizationFrequency;
        String regularizerFrequency;
        bool restoreBestParametersWhenFinished;

        forEachXmlChildElementWithTagName(*parameters, parameter, String("param")) {
            if ((parameter->getStringAttribute(String("name"))).compare(String("includeBoundsProximity")) == 0) {
                String test = parameter->getStringAttribute(String("value"));
                if (test.compare(String("yes")) == 0)
                    includeBoundsProximity = true;
                else
                    includeBoundsProximity = false;
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("includeGlobalHistograms")) == 0) {
                String test = parameter->getStringAttribute(String("value"));
                if (test.compare(String("yes")) == 0)
                    includeGlobalHistograms = true;
                else
                    includeGlobalHistograms = false;
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("includeLocalAAWindow")) == 0) {
                String test = parameter->getStringAttribute(String("value"));
                if (test.compare(String("yes")) == 0)
                    includeLocalAAWindow = true;
                else
                    includeLocalAAWindow = false;
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("includeLocalHistograms")) == 0) {
                String test = parameter->getStringAttribute(String("value"));
                if (test.compare(String("yes")) == 0)
                    includeLocalHistograms = true;
                else
                    includeLocalHistograms = false;
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("l1RegularizerWeight")) == 0) {
                l1RegularizerWeight = parameter->getDoubleAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("l2RegularizerWeight")) == 0) {
                l2RegularizerWeight = parameter->getDoubleAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("learningRate")) == 0) {
                learningRate = parameter->getDoubleAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("learningRateDecrease")) == 0) {
                learningRateDecrease = parameter->getDoubleAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("learningStepsFrequency")) == 0) {
                learningStepsFrequency = parameter->getStringAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("multiClassInference")) == 0) {
                multiClassInference = parameter->getStringAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("numIntervalsPerEntropy")) == 0) {
                numIntervalsPerEntropy = parameter->getIntAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("numIntervalsPerPositiveInteger")) == 0) {
                numIntervalsPerPositiveInteger = parameter->getIntAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("numIntervalsPerProbability")) == 0) {
                numIntervalsPerProbability = parameter->getIntAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("randomizationFrequency")) == 0) {
                randomizationFrequency = parameter->getStringAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("regularizerFrequency")) == 0) {
                regularizerFrequency = parameter->getStringAttribute(String("value"));
            }
            else if ((parameter->getStringAttribute(String("name"))).compare(String("restoreBestParametersWhenFinished")) == 0) {
                String test = parameter->getStringAttribute(String("value"));
                if (test.compare(String("yes")) == 0)
                    restoreBestParametersWhenFinished = true;
                else
                    restoreBestParametersWhenFinished = false;
            }
        }

        delete mainElement;

        int maxAppVersion = 0;
        for (int x = 0; x < results.size(); x++) {
            maxAppVersion = max(maxAppVersion, results.at(x).app_version_num);
        }

        stringstream oss;
        oss << "INSERT INTO errors "
            << "(name, app_version_num, multiClassInference, randomizationFrequency, learningStepsFrequency, learningRate, learningRateDecrease, regularizerFrequency, "
            << "l1RegularizerWeight, l2RegularizerWeight, restoreBestParametersWhenFinished, numIntervalsPerProbability, numIntervalsPerPositiveInteger, numIntervalsPerEntropy, "
            << "includeGlobalHistograms, includeLocalHistograms, includeLocalAAWindow, includeBoundsProximity)"
            <<  " VALUES ("
            << "'"<< name.toUTF8() << "', "
            << "'"<< maxAppVersion << "', "
            << "'"<< multiClassInference.toUTF8() << "', "
            << "'"<< randomizationFrequency.toUTF8() << "', "
            << "'"<< learningStepsFrequency.toUTF8() << "', "
            << "'"<< learningRate << "', "
            << "'"<< learningRateDecrease << "', "
            << "'"<< regularizerFrequency.toUTF8() << "', "
            << "'"<< l1RegularizerWeight << "', "
            << "'"<< l2RegularizerWeight << "', "
            << "'"<< restoreBestParametersWhenFinished << "', "
            << "'"<< numIntervalsPerProbability << "', "
            << "'"<< numIntervalsPerPositiveInteger << "', "
            << "'"<< numIntervalsPerEntropy << "', "
            << "'"<< includeGlobalHistograms << "', "
            << "'"<< includeLocalHistograms << "', "
            << "'"<< includeLocalAAWindow << "', "
            << "'"<< includeBoundsProximity << "')";

        MYSQL *conn;
        conn = mysql_init(NULL);
        if (conn == NULL) {
            log_messages.printf(MSG_CRITICAL, "Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
            return -1;
        }

        if (mysql_real_connect(conn, "localhost", "boincadm", "002230", "BoincEvaluator", 0, NULL, 0) == NULL) {
            log_messages.printf(MSG_CRITICAL, "Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
            return -1;
        }

        if (mysql_query(conn, oss.str().c_str())) {
            log_messages.printf(MSG_CRITICAL, "Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
            mysql_close(conn);
            return -1;
        }
        mysql_close(conn);
        oss.clear();

        sprintf(buf, "%s: 0x%x\n", wu.name, wu.error_mask);
        return write_error(buf);
    }

}
