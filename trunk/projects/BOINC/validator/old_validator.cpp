#include "config.h"
#include "util.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "sched_config.h"
#include "validate_util.h"
#include "error_numbers.h"
#include "boinc_db.h"
#include "filesys.h"

#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <juce_amalgamated.h>

using std::vector;
using std::stringstream;

struct DATA {
    double x;
};

int init_result(RESULT& result, void*& data) {
    int retval;

    vector<FILE_INFO> files;
    retval = get_output_file_infos(result, files);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d %s] check_set: can't get output filenames\n",
            result.id, result.name
        );
        return retval;
    }

    FILE_INFO& fi = files[0];   // TODO peut gere plusieurs fichiers

    const File file(fi.path.c_str());
    XmlDocument xmlFile(file);
    XmlElement* mainElement = xmlFile.getDocumentElement();

    if (mainElement == 0) {
		String myerror = xmlFile.getLastParseError();
		log_messages.printf(MSG_CRITICAL, "[RESULT#%d %s]  cannot parse XML file : %s\n",
            result.id, result.name, myerror.toUTF8()
        );
        return ERR_XML_PARSE;
	}

    if (mainElement->getTagName().compare(String("result")) != 0) {
        delete mainElement;
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d %s] Root tag is not <result>\n",
            result.id, result.name
        );
		return ERR_XML_PARSE;
	}

	XmlElement* scores = mainElement->getChildByName(String("scores"));
	XmlElement* score = scores->getChildByName(String("score"));    // TODO gerer plusieurs scores
    double x = score->getDoubleAttribute(String("value"));
    delete mainElement;

    DATA* dp = new DATA;
    dp->x = x;
    data = (void*) dp;
    return 0;
}

int compare_results(RESULT& r1, void* _data1, const RESULT& r2, void* _data2, bool& match) {
    DATA* data1 = (DATA*)_data1;
    DATA* data2 = (DATA*)_data2;
    match = true;
    if (fabs(data1->x - data2->x) > 0.000000001) {
        match = false;

        int retval;

        retval = boinc_mkdir(config.project_path("invalid_results"));
        if (retval) return retval;

        const char *copy_path1;
        vector<FILE_INFO> files1;
        retval = get_output_file_infos(r1, files1);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "[RESULT#%d %s] check_set: can't get output filenames\n",
                r1.id, r1.name
            );
        } else {
            copy_path1 = config.project_path("invalid_results/%s", r1.name);
            retval = boinc_copy(files1[0].path.c_str() , copy_path1);
            if (retval) {
                log_messages.printf(MSG_CRITICAL, "%s cannot be copied into %s\n\n", files1[0].path.c_str(), copy_path1);
            }
        }

        const char *copy_path2;
        vector<FILE_INFO> files2;
        RESULT r2_ = r2;
        retval = get_output_file_infos(r2_, files2);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "[RESULT#%d %s] check_set: can't get output filenames\n",
                r2.id, r2.name
            );
        } else {
            copy_path2 = config.project_path("invalid_results/%s", r2.name);
            retval = boinc_copy(files2[0].path.c_str() , copy_path2);
            if (retval) {
                log_messages.printf(MSG_CRITICAL, "%s cannot be copied into %s\n\n", files2[0].path.c_str(), copy_path2);
            }
        }

        File file(files1[0].path.c_str());
        String name1 = file.getFileName();

        XmlDocument xmlDoc(file);
        XmlElement* mainElement = xmlDoc.getDocumentElement();
        if (mainElement == 0) {
            log_messages.printf(MSG_CRITICAL, "Cannot get root element of : %s\n", name1.toUTF8());
            return -1;
        }

        if (mainElement->getTagName().compare(String("result")) != 0) {
            delete mainElement;
            log_messages.printf(MSG_CRITICAL,
                "Root tag is not <result> in: %s\n",
                name1.toUTF8()
            );
            return -1;
        }

        // TODO gérer les erreurs:
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

        XmlElement* scores = mainElement->getChildByName(String("scores"));
        XmlElement* score = scores->getChildByName(String("score"));    // TODO gerer plusieurs scores
        double x1 = score->getDoubleAttribute(String("value"));
        delete mainElement;

        File file2(files2[0].path.c_str());
        String name2 = file2.getFileName();

        XmlDocument xmlDoc2(file2);
        mainElement = xmlDoc2.getDocumentElement();
        if (mainElement == 0) {
            log_messages.printf(MSG_CRITICAL, "Cannot get root element of : %s\n", name2.toUTF8());
            return -1;
        }

        if (mainElement->getTagName().compare(String("result")) != 0) {
            delete mainElement;
            log_messages.printf(MSG_CRITICAL,
                "Root tag is not <result> in: %s\n",
                name2.toUTF8()
            );
            return -1;
        }

        scores = mainElement->getChildByName(String("scores"));
        score = scores->getChildByName(String("score"));    // TODO gerer plusieurs scores
        double x2 = score->getDoubleAttribute(String("value"));
        delete mainElement;

        stringstream oss;
        oss << "INSERT INTO invalids "
            << "(name1, name2, score1, score2, hostid1, hostid2, app_version_num1, app_version_num2, multiClassInference, randomizationFrequency, learningStepsFrequency, learningRate, learningRateDecrease, regularizerFrequency, "
            << "l1RegularizerWeight, l2RegularizerWeight, restoreBestParametersWhenFinished, numIntervalsPerProbability, numIntervalsPerPositiveInteger, numIntervalsPerEntropy, "
            << "includeGlobalHistograms, includeLocalHistograms, includeLocalAAWindow, includeBoundsProximity)"
            <<  " VALUES ("
            << "'"<< name1.toUTF8() << "', "
            << "'"<< name2.toUTF8() << "', "
            << "'"<< x1 << "', "
            << "'"<< x2 << "', "
            << "'"<< r1.hostid << "', "
            << "'"<< r2.hostid << "', "
            << "'"<< r1.app_version_num << "', "
            << "'"<< r2.app_version_num << "', "
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
    }
    return 0;
}

int cleanup_result(RESULT const& r, void* data) {
    if (data) delete (DATA*) data;
    return 0;
}
