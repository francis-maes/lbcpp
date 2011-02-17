#include <iostream>
#include <fstream>
#include <lbcpp/Data/RandomGenerator.h>
#include <time.h>
#include <map>
#include <algorithm>

#include "Category.h"

#include <unistd.h>
#include <cstdlib>
#include <string>
#include <cstring>

#include "boinc_db.h"
#include "error_numbers.h"
#include "backend_lib.h"
#include "parse.h"
#include "util.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "str_util.h"

#define CUSHION 50
    // maintain at least this many unsent results
#define REPLICATION_FACTOR  2

using namespace lbcpp;
using namespace juce;
using namespace std;

// globals
//
char* wu_template;
DB_APP app;
int start_time;
int seqno;
String appname;
XmlElement* mainElement;
XmlElement* parameters;
map<String, Category*>  categoriesMap;  // TODO delete

void parseApplicationFile(const char* applicationDescripton) {
    const File file(applicationDescripton);

	if (!file.exists()) {
		cerr << "File not found!" << endl;
		exit(EXIT_FAILURE);
	}

	XmlDocument xmlFile(file);
	mainElement = xmlFile.getDocumentElement();
	if (mainElement == 0) {
		String myerror = xmlFile.getLastParseError();
		delete mainElement;
		cerr << myerror.toUTF8() << endl;
		exit(EXIT_FAILURE);
	}

	if (mainElement->getTagName().compare(String("application")) != 0) {
		cerr << "Root tag is not <application>" << endl;
		exit(EXIT_FAILURE);
	}

	XmlElement* worker = mainElement->getChildByName(String("worker"));
	appname = worker->getStringAttribute(String("name"));

	//XmlElement* archive = mainElement->getChildByName(String("dependencies"));
	//String archivename = archive->getStringAttribute(String("archive"));
	//cout << "archivename: " << archivename.toUTF8() << endl;

	//XmlElement* checkpoint = mainElement->getChildByName(String("checkpoint"));
	//String statefile = checkpoint->getStringAttribute(String("statefile"));
	//String progressfile = checkpoint->getStringAttribute(String("progressfile"));
	//cout << "statefile: " << statefile.toUTF8() << endl;
	//cout << "progressfile: " << progressfile.toUTF8() << endl;

	parameters = mainElement->getChildByName(String("parameters"));

	XmlElement* categories = parameters->getChildByName(String("categories"));
    forEachXmlChildElementWithTagName(*categories, category, String("category")) {
        String name = category->getStringAttribute(String("name"));
		categoriesMap.insert(pair<String, Category*>(name, new Category(name, category)));
	}

    RandomGeneratorPtr rg = RandomGenerator::getInstance();
	rg->setSeed((int) time(NULL));
}

void createInputFile(const char* infile) {
	ofstream outfile(infile);
	outfile << "<parameters>" << endl;
	forEachXmlChildElementWithTagName(*parameters, parameter, String("param")) {
		if ((parameter->getStringAttribute(String("type"))).compare(String("numeric")) == 0 ) {	// TODO éviter de refaire ça à chaque fois !!
			double min = parameter->getDoubleAttribute(String("min"));
			double max = parameter->getDoubleAttribute(String("max"));
			double mean;
			if (parameter->hasAttribute(String("mean"))) {
				mean = parameter->getDoubleAttribute(String("mean"));
			} else {
				mean = (min+max)/2;
			}
			double stddev;
			if (parameter->hasAttribute(String("stddev"))) {
				stddev = parameter->getDoubleAttribute(String("stddev"));
			} else {
				stddev = std::max(abs(mean-min), abs(mean-max))/3;
			}
			double value = RandomGenerator::getInstance()->sampleDoubleFromGaussian(mean, stddev);
			value = std::max(min, value);
			value = std::min(max, value);
			outfile << "\t";
			outfile << "<param name=\"" << parameter->getStringAttribute(String("name")).toUTF8() << "\" value=\"" << value << "\"/>" << endl;
		} else if ((parameter->getStringAttribute(String("type"))).compare(String("integer")) == 0 ) {	// TODO regrouper avec numeric
			int min = parameter->getIntAttribute(String("min"));
			int max = parameter->getIntAttribute(String("max"));
			double mean;
			if (parameter->hasAttribute(String("mean"))) {
				mean = parameter->getDoubleAttribute(String("mean"));
			} else {
				mean = (min+max)/2.0;
			}
			double stddev;
			if (parameter->hasAttribute(String("stddev"))) {
				stddev = parameter->getDoubleAttribute(String("stddev"));
			} else {
				stddev = std::max(abs(mean-min), abs(mean-max))/3.0;
			}
			int value = RandomGenerator::getInstance()->sampleDoubleFromGaussian(mean, stddev);
			value = std::max(min, value);
			value = std::min(max, value);
			outfile << "\t";
			outfile << "<param name=\"" << parameter->getStringAttribute(String("name")).toUTF8() << "\" value=\"" << value << "\"/>" << endl;
		}
		else if (categoriesMap.find(parameter->getStringAttribute(String("type"))) != categoriesMap.end()) {
			outfile << "\t";
			outfile << "<param name=\"" << parameter->getStringAttribute(String("name")).toUTF8() << "\" value=\"" << categoriesMap[parameter->getStringAttribute(String("type"))]->getRandomValue() << "\"/>" << endl;
		}
	}
	outfile << "</parameters>" << endl;

	outfile.close();
}

int make_job() {
    DB_WORKUNIT wu;
    char name[256], path[256];
    const char* infiles[1];
    int retval;

    // make a unique name (for the job and its input file)
    //
    sprintf(name, "BoincEvaluator_%d_%d", start_time, seqno++); // TODO app name

    // Create the input file.
    // Put it at the right place in the download dir hierarchy
    //
    retval = config.download_path(name, path);
    if (retval) return retval;
    //if (out.fail()) return ERR_FOPEN;
    //fprintf(f, "This is the input file for job %s", name);  // TODO input.xml
    createInputFile(path);
    //out.close();

    // Fill in the job parameters
    //
    wu.clear();
    wu.appid = app.id;
    strcpy(wu.name, name);
    wu.rsc_fpops_est = 5e12;
    wu.rsc_fpops_bound = 1e13;
    wu.rsc_memory_bound = 1e9;
    wu.rsc_disk_bound = 500e6;
    wu.delay_bound = 604800;
    wu.min_quorum = REPLICATION_FACTOR;
    wu.target_nresults = REPLICATION_FACTOR;
    wu.max_error_results = REPLICATION_FACTOR*4;
    wu.max_total_results = REPLICATION_FACTOR*8;
    wu.max_success_results = REPLICATION_FACTOR*4;
    infiles[0] = name;

    // Register the job with BOINC
    //
    return create_work(     // TODO checker le reste des paramètres
        wu,
        wu_template,
        "templates/BoincEvaluator_result",
        config.project_path("templates/BoincEvaluator_result"),
        infiles,
        1,
        config
    );
}

void main_loop() {
    int retval;

    while (1) {
        check_stop_daemons();
        int n;
        retval = count_unsent_results(n, 0);    // TODO app_id
        if (n > CUSHION) {
            sleep(60);
        } else {
            int njobs = (CUSHION-n)/REPLICATION_FACTOR;
            log_messages.printf(MSG_DEBUG,
                "Making %d jobs\n", njobs
            );
            for (int i=0; i<njobs; i++) {
                retval = make_job();
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "can't make job: %d\n", retval
                    );
                    exit(retval);
                }
            }
            // Now sleep for a few seconds to let the transitioner
            // create instances for the jobs we just created.
            // Otherwise we could end up creating an excess of jobs.
            sleep(5);
        }
    }
}

void usage(char *name) {
    fprintf(stderr,
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  [ -d X ]                    Sets debug level to X.\n"
        "  [ -h | --help ]             Shows this help text.\n",
        name
    );
}

int main(int argc, char** argv) {
	SystemStats::initialiseStats(); // for juce

    int i, retval;
    const char* applicationDescription;

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "d")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]); // debug level
            log_messages.set_debug_level(dl);
            if (dl == 4)
                g_print_queries = true;
        } else if (is_arg(argv[i], "app")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            applicationDescription = argv[i]; // debug level
        }
        else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    retval = config.parse_file();   // parse config.xml
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open db\n");
        exit(1);
    }

    parseApplicationFile(applicationDescription);
    if (app.lookup("where name='BoincEvaluator'")) { // TODO change appname
        log_messages.printf(MSG_CRITICAL, "can't find app\n");
        exit(1);
    }
    if (read_file_malloc(config.project_path("templates/BoincEvaluator_wu"), wu_template)) {    // TODO appname_wu
        log_messages.printf(MSG_CRITICAL, "can't read WU template\n");
        exit(1);
    }

    start_time = time(0);
    seqno = 0;

    log_messages.printf(MSG_NORMAL, "Starting\n");

    main_loop();
}

