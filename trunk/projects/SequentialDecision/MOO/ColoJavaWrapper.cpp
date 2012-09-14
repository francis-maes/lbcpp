/*-----------------------------------------.---------------------------------.
| Filename: ColoJavaWrapper.cpp            | COLO Wrapper to Java            |
| Author  : Francis Maes                   |                                 |
| Started : 14/09/2012 19:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"

extern "C" {
#include <jni.h>
};

#include "ColoSandBox.h"
using namespace lbcpp;

class JavaJVM
{
public:
  JavaJVM() : jvm(NULL) {}
  ~JavaJVM()  {if (jvm) jvm->DestroyJavaVM();}

  bool initialize(ExecutionContext& context, const File& javaDirectory, const File& modelDirectory)
  {
    JavaVMInitArgs args;
    args.version = JNI_VERSION_1_6;
	  args.ignoreUnrecognized = JNI_FALSE;


#ifdef JUCE_WIN32
    String option = "-Djava.class.path=" + javaDirectory.getFullPathName() + ";" + javaDirectory.getChildFile("weka.jar").getFullPathName();
#else
    String option = "-Djava.class.path=" + javaDirectory.getFullPathName() + ":" + javaDirectory.getChildFile("weka.jar").getFullPathName();
#endif // JUCE_WIN32
    
    //String option =  "-Djava.class.path=C:\\Projets\\LBCpp\\workspace\\MOO;C:\\Projets\\LBCpp\\workspace\\MOO\\weka.jar";
    JavaVMOption options[1];
    options[0].optionString = const_cast<char* >((const char* )option);
	  args.options = options;
	  args.nOptions = 1;
        
    int ret = JNI_CreateJavaVM(&jvm, (void**)&env, &args);
    if (ret < 0)
    {
      context.errorCallback("Could not create JVM");
      return false;
    }
    context.informationCallback("JVM Created");

    // find constructor and getValue method
   	evaluatorClass = env->FindClass("ColoEvaluator");
    if (!evaluatorClass)
    {
      env->ExceptionDescribe();
      context.errorCallback("Could not find ColoEvaluator class");
      return false;
    }
    constructor = env->GetMethodID(evaluatorClass, "<init>", "(Ljava/lang/String;)V");
    getValuesMethod = env->GetMethodID(evaluatorClass, "getValue", "(Ljava/lang/String;)[D");
    jstring modelpath = env->NewStringUTF(modelDirectory.getFullPathName());
    context.enterScope("Loading Weka Models");
    instance = env->NewObject(evaluatorClass, constructor, modelpath);
    context.leaveScope();
    return true;
  }

  std::vector<double> evaluate(const ColoObjectPtr& colo)
  {
    jstring sequence = env->NewStringUTF(colo->toShortString());
    jdoubleArray vals = (jdoubleArray)env->CallObjectMethod(instance, getValuesMethod, sequence);
    std::vector<double> values(2);
    if (vals)
      env->GetDoubleArrayRegion(vals,0,2,&values[0]);
    else
    {
      values[0] = 10.0;
      values[1] = 0.0;
    }
    return values;
  }
  JavaVM* jvm;
  JNIEnv* env;
  
  jclass evaluatorClass;
  jmethodID constructor;
  jmethodID getValuesMethod;
  jobject instance;
} jvm;

class ColoJavaWrapper
{
public:
  bool initialize(ExecutionContext& context, const File& javaDirectory, const File& modelDirectory)
    {return jvm.jvm || jvm.initialize(context, javaDirectory, modelDirectory);}

  std::vector<double> evaluate(const ColoObjectPtr& colo)
    {return jvm.evaluate(colo);}

};

void* lbcpp::createColoJavaWrapper(ExecutionContext& context, const File& javaDirectory, const File& modelDirectory)
{
  ColoJavaWrapper* res = new ColoJavaWrapper();
  if (res->initialize(context, javaDirectory, modelDirectory))
    return res;
  delete res;
  return NULL;
}
  
std::vector<double> lbcpp::evaluateColoJavaWrapper(void* wrapper, const ColoObjectPtr& colo)
  {return ((ColoJavaWrapper* )wrapper)->evaluate(colo);}

void lbcpp::freeColoJavaWrapper(void* wrapper)
  {delete (ColoJavaWrapper* )wrapper;}

