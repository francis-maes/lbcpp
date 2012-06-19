/*-----------------------------------------.---------------------------------.
| Filename: RTreeFunction.cpp              | Wrapper of ExtraTrees           |
| Author  : Julien Becker                  | implemented by Pierre Geurts    |
| Started : 16/02/2011 13:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "RTreeFunction.h"
#include "RTreeBatchLearner.h"
#include <lbcpp/Core/ReferenceCountedObject.h>
#include <lbcpp/Core/DynamicObject.h>
#include <lbcpp/Data/DoubleVector.h>

#define LOAD_MULTIREGR
#define LOAD_OK3

static void rtree_update_progression(size_t);
static void context_result(const String&, double);

#include "RTree/tree-model.h"
#include "RTree/tree-model.c"
#include "RTree/tree-density.c"
#include "RTree/tree-kernel.c"
#include "RTree/f-table.c"
#include "RTree/tree-ok3.c"
#include "RTree/tree-multiregr.c"


CORETABLE_TYPE *core_table_y = NULL;

float getobjy_multiregr_learn_matlab(int obj, int att) {
  return (float)core_table_y[goal_multiregr[att]*nb_obj_in_core_table+object_mapping[obj]];
}

using namespace lbcpp;

ExecutionContext* rtree_context;
ProgressionStatePtr rtree_progress;

static CriticalSection learnerLock;

void rtree_update_progression(size_t value)
{
  rtree_progress->setValue(value);
  rtree_context->progressCallback(rtree_progress);
}

void context_result(const String& name, double data)
  {rtree_context->resultCallback(name, data);}

namespace lbcpp
{

class RTree : public Object
{
public:
  RTree(size_t maxNumNodes, size_t numLeaves, size_t numPredictions)
  {
    treesState.maxNumNodes = maxNumNodes;
    treesState.numLeaves = numLeaves;
    treesState.numPredictions = numPredictions;
  }

  Variable makePrediction(ExecutionContext& context, const Variable& input, const TypePtr& outputType) const
  {
    ScopedLock _(learnerLock);
    restoreTreesState();
    float *saved_core_table = core_table;
    int saved_nb_obj_in_core_table = nb_obj_in_core_table;

    /* on change la core table */
    nb_obj_in_core_table = 1;
    core_table = (CORETABLE_TYPE *)MyMalloc((size_t)nb_attributes * sizeof(CORETABLE_TYPE));
    jassert(core_table);
    ContainerPtr obj = input.getObjectAndCast<Container>(context);
    //jassert(obj->getNumElements() == (size_t)nb_attributes);
    for (size_t j = 0; j < (size_t)nb_attributes; ++j)
    {
      Variable objVariable = obj->getElement(j);
      TypePtr objType = objVariable.getType();
      CORETABLE_TYPE value;
      if (objType->inheritsFrom(booleanType))
        value = (CORETABLE_TYPE)(objVariable.getBoolean() ? 1 : 0);
      else if (objType->inheritsFrom(enumValueType))
        value = (CORETABLE_TYPE)objVariable.getInteger();
      else if (objType->inheritsFrom(doubleType))
        value = (CORETABLE_TYPE)objVariable.getDouble();
      else if (objType->inheritsFrom(integerType))
        value = (CORETABLE_TYPE)objVariable.getInteger();
      else {
        jassertfalse;
      }
      core_table[j] = value;
    }

    /* on cree la table */
    float* output = (float*)MyMalloc((size_t)nb_goal_multiregr * sizeof(float));
    jassert(output);

    /* on teste */
    get_multiregr_ens_pred_savepred(0, output);

    Variable res;
    if (outputType->inheritsFrom(booleanType) || outputType->inheritsFrom(probabilityType))
      res = Variable((double)*output, probabilityType);
    else if (outputType->inheritsFrom(doubleType))
      res = Variable((double)*output, doubleType);
    else if (outputType->inheritsFrom(enumValueType))
    {
      DenseDoubleVectorPtr resVector = new DenseDoubleVector(outputType, probabilityType);
      for (size_t i = 0; i < (size_t)nb_goal_multiregr; ++i)
        resVector->getValueReference(i) = output[i];
      res = resVector;
    }
    else if (outputType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
    {
      DenseDoubleVectorPtr resVector = new DenseDoubleVector(outputType->getTemplateArgument(0).staticCast<Enumeration>(), probabilityType);
      for (size_t i = 0; i < (size_t)nb_goal_multiregr; ++i)
        resVector->getValueReference(i) = output[i];
      res = resVector;
    }
    else
    {
      jassertfalse;
    }

    /* Liberation de la mémoire */
    MyFree(core_table);
    MyFree(output);

    /* on remet la coretable (pour les variables importances) */
    core_table = saved_core_table;
    nb_obj_in_core_table = saved_nb_obj_in_core_table;

    return res;
  }
  
  ~RTree()
  {
    /* on libere la memoire */
    MyFreeAndNull(treesState.ltrees);
    MyFreeAndNull(treesState.ltrees_weight);
    MyFreeAndNull(treesState.left_successor);
    MyFreeAndNull(treesState.right_successor);
    MyFreeAndNull(treesState.tested_attribute);
    free_table_float(treesState.prediction_values, treesState.size_current_tree_table_pred);
    MyFreeAndNull(treesState.prediction);
    MyFreeAndNull(treesState.threshold);
  }

  void saveTreesState()
  {
    treesState.nb_goal_multiregr = nb_goal_multiregr;
    treesState.current_nb_of_ensemble_terms = current_nb_of_ensemble_terms;
    treesState.ltrees_weight = (float*)MyMalloc(current_nb_of_ensemble_terms * sizeof(float));
    treesState.ltrees = (int*)MyMalloc(current_nb_of_ensemble_terms * sizeof(int));
    for (size_t i = 0; i < (size_t)current_nb_of_ensemble_terms; ++i)
    {
      treesState.ltrees[i] = ltrees[i];
      treesState.ltrees_weight[i] = ltrees_weight[i];
    }
    treesState.average_predictions_ltrees = average_predictions_ltrees;
    treesState.get_tree_prediction_vector = get_tree_prediction_vector;
    treesState.left_successor = left_successor;
    left_successor = NULL;
    treesState.right_successor = right_successor;
    right_successor = NULL;
    treesState.tested_attribute = tested_attribute;
    tested_attribute = NULL;
    treesState.prediction_values = prediction_values;
    prediction_values = NULL;
    treesState.size_current_tree_table_pred = size_current_tree_table_pred;
    treesState.prediction = prediction;
    prediction = NULL;
    treesState.threshold = threshold;
    threshold = NULL;
    treesState.attribute_descriptors = attribute_descriptors;
    attribute_descriptors = NULL;
    treesState.nb_attributes = nb_attributes;
    treesState.getattval = getattval;
  }

  void restoreTreesState() const
  {
    nb_goal_multiregr = treesState.nb_goal_multiregr;
    current_nb_of_ensemble_terms = treesState.current_nb_of_ensemble_terms;
    for (size_t i = 0; i < (size_t)current_nb_of_ensemble_terms; ++i)
    {
      ltrees[i] = treesState.ltrees[i];
      ltrees_weight[i] = treesState.ltrees_weight[i];
    }
    average_predictions_ltrees = treesState.average_predictions_ltrees;
    get_tree_prediction_vector = treesState.get_tree_prediction_vector;
    left_successor = treesState.left_successor;
    right_successor = treesState.right_successor;
    tested_attribute = treesState.tested_attribute;
    prediction_values = treesState.prediction_values;
    size_current_tree_table_pred = treesState.size_current_tree_table_pred;
    prediction = treesState.prediction;
    threshold = treesState.threshold;
    attribute_descriptors = treesState.attribute_descriptors;
    nb_attributes = treesState.nb_attributes;
    getattval = treesState.getattval;
  }

  void saveToXml(XmlExporter& exporter) const
  {
    exporter.enter(T("maxNumNodes"));
    exporter.writeVariable(Variable(treesState.maxNumNodes, integerType), integerType);
    exporter.leave();
    
    exporter.enter(T("numLeaves"));
    exporter.writeVariable(Variable(treesState.numLeaves, integerType), integerType);
    exporter.leave();
    
    exporter.enter(T("numPredictions"));
    exporter.writeVariable(Variable(treesState.numPredictions, integerType), integerType);
    exporter.leave();
    
    exporter.enter(T("nb_goal_multiregr"));
    exporter.writeVariable(Variable(treesState.nb_goal_multiregr, integerType), integerType);
    exporter.leave();
    
    exporter.enter(T("current_nb_of_ensemble_terms"));
    exporter.writeVariable(Variable(treesState.current_nb_of_ensemble_terms, integerType), integerType);
    exporter.leave();

    exporter.enter(T("average_predictions_ltrees"));
    exporter.writeVariable(Variable(treesState.average_predictions_ltrees, integerType), integerType);
    exporter.leave();

    exporter.enter(T("size_current_tree_table_pred"));
    exporter.writeVariable(Variable(treesState.size_current_tree_table_pred, integerType), integerType);
    exporter.leave();

    exporter.enter(T("nb_attributes"));
    exporter.writeVariable(Variable(treesState.nb_attributes, integerType), integerType);
    exporter.leave();

    File outputFile = exporter.getContext().getFile(T("model.bin"));
    OutputStream* o = outputFile.createOutputStream();
    // ltrees
    for (size_t i = 0; i < (size_t)treesState.current_nb_of_ensemble_terms; ++i)
      o->writeInt(treesState.ltrees[i]);
    // ltrees_weight
    for (size_t i = 0; i < (size_t)treesState.current_nb_of_ensemble_terms; ++i)
      o->writeFloat(treesState.ltrees_weight[i]);
    // left_successor
    for (size_t i = 0; i < (size_t)treesState.maxNumNodes; ++i)
      o->writeInt(treesState.left_successor[i]);
    // right_successor
    for (size_t i = 0; i < (size_t)treesState.maxNumNodes; ++i)
      o->writeInt(treesState.right_successor[i]);
    // tested_attribute
    for (size_t i = 0; i < (size_t)treesState.maxNumNodes; ++i)
      o->writeInt(treesState.tested_attribute[i]);
    // threshold
    for (size_t i = 0; i < treesState.maxNumNodes; ++i)
      for (size_t j = 0; j < MAX_NUMBER_OF_SYMBOLIC_VALUES_DIV_32; ++j)
        o->writeInt(treesState.threshold[i].i[j]);
    // prediction
    for (size_t i = 0; i < treesState.maxNumNodes; ++i)
      o->writeInt(treesState.prediction[i]);
    // prediction_values
    for (size_t i = 0; i < treesState.numLeaves; ++i)
      for (size_t j = 0; j < treesState.numPredictions; ++j)
        o->writeFloat(treesState.prediction_values[i][j]);
    // attribute_descriptors
    for (size_t i = 0; i < (size_t)treesState.nb_attributes; ++i)
      o->writeInt(treesState.attribute_descriptors[i]);

    delete o;
  }
  
  bool loadFromXml(XmlImporter& importer)
  {
    if (importer.enter(T("maxNumNodes")))
    {
      treesState.maxNumNodes = (size_t)importer.getAllSubText().getIntValue();
      importer.leave();
    }
    if (importer.enter(T("numLeaves")))
    {
      treesState.numLeaves = (size_t)importer.getAllSubText().getIntValue();
      importer.leave();
    }
    if (importer.enter(T("numPredictions")))
    {
      treesState.numPredictions = (size_t)importer.getAllSubText().getIntValue();
      importer.leave();
    }
    if (importer.enter(T("nb_goal_multiregr")))
    {
      treesState.nb_goal_multiregr = importer.getAllSubText().getIntValue();
      importer.leave();
    }
    if (importer.enter(T("current_nb_of_ensemble_terms")))
    {
      treesState.current_nb_of_ensemble_terms = importer.getAllSubText().getIntValue();
      importer.leave();
    }
    if (importer.enter(T("average_predictions_ltrees")))
    {
      treesState.average_predictions_ltrees = importer.getAllSubText().getIntValue();
      importer.leave();
    }
    if (importer.enter(T("size_current_tree_table_pred")))
    {
      treesState.size_current_tree_table_pred = importer.getAllSubText().getIntValue();
      importer.leave();
    }
    if (importer.enter(T("nb_attributes")))
    {
      treesState.nb_attributes = importer.getAllSubText().getIntValue();
      importer.leave();
    }

    File inputFile = importer.getContext().getFile(T("model.bin"));
    InputStream* o = inputFile.createInputStream();
    // ltrees
    treesState.ltrees = (int*)MyMalloc(treesState.current_nb_of_ensemble_terms * sizeof(int));
    if (treesState.ltrees == NULL)
    {
      importer.getContext().errorCallback(T("RTree::loadFromXml"), T("Malloc failed (ltrees) !"));
      return false;
    }
    for (size_t i = 0; i < (size_t)treesState.current_nb_of_ensemble_terms; ++i)
      treesState.ltrees[i] = o->readInt();
    // ltrees_weight
    treesState.ltrees_weight = (float*)MyMalloc(treesState.current_nb_of_ensemble_terms * sizeof(float));
    if (treesState.ltrees_weight == NULL)
    {
      importer.getContext().errorCallback(T("RTree::loadFromXml"), T("Malloc failed (ltrees_weight) !"));
      return false;
    }
    for (size_t i = 0; i < (size_t)treesState.current_nb_of_ensemble_terms; ++i)
        treesState.ltrees_weight[i] = o->readFloat();
    // left_successor
    treesState.left_successor = (int*)MyMalloc(treesState.maxNumNodes * sizeof(int));
    if (treesState.left_successor == NULL)
    {
      importer.getContext().errorCallback(T("RTree::loadFromXml"), T("Malloc failed (left_successor) !"));
      return false;
    }
    for (size_t i = 0; i < (size_t)treesState.maxNumNodes; ++i)
      treesState.left_successor[i] = o->readInt();
    // right_successor
    treesState.right_successor = (int*)MyMalloc(treesState.maxNumNodes * sizeof(int));
    if (treesState.right_successor == NULL)
    {
      importer.getContext().errorCallback(T("RTree::loadFromXml"), T("Malloc failed (right_successor) !"));
      return false;
    }
    for (size_t i = 0; i < (size_t)treesState.maxNumNodes; ++i)
      treesState.right_successor[i] = o->readInt();
    // tested_attribute
    treesState.tested_attribute = (int*)MyMalloc(treesState.maxNumNodes * sizeof(int));
    if (treesState.tested_attribute == NULL)
    {
      importer.getContext().errorCallback(T("RTree::loadFromXml"), T("Malloc failed (tested_attribute) !"));
      return false;
    }
    for (size_t i = 0; i < (size_t)treesState.maxNumNodes; ++i)
      treesState.tested_attribute[i] = o->readInt();
    // threshold
    treesState.threshold = (union threshold_type*)MyMalloc(treesState.maxNumNodes * sizeof(union threshold_type));
    if (treesState.threshold == NULL)
    {
      importer.getContext().errorCallback(T("RTree::loadFromXml"), T("Malloc failed (threshold) !"));
      return false;
    }
    for (size_t i = 0; i < (size_t)treesState.maxNumNodes; ++i)
      for (size_t j = 0; j < MAX_NUMBER_OF_SYMBOLIC_VALUES_DIV_32; ++j)
        treesState.threshold[i].i[j] = o->readInt();
    // prediction
    treesState.prediction = (int*)MyMalloc(treesState.maxNumNodes * sizeof(int));
    if (treesState.prediction == NULL)
    {
      importer.getContext().errorCallback(T("RTree::loadFromXml"), T("Malloc failed (prediction) !"));
      return false;
    }
    for (size_t i = 0; i < (size_t)treesState.maxNumNodes; ++i)
      treesState.prediction[i] = o->readInt();
    // prediction_values
    treesState.prediction_values = (float**)MyMalloc(treesState.numLeaves * sizeof(float*));
    if (treesState.prediction_values == NULL)
    {
      importer.getContext().errorCallback(T("RTree::loadFromXml"), T("Malloc failed (prediction_values) !"));
      return false;
    }
    for (size_t i = 0; i < treesState.numLeaves; ++i)
    {
      treesState.prediction_values[i] = (float*)MyMalloc(treesState.numPredictions * sizeof(float));
      if (treesState.prediction_values[i] == NULL)
      {
        importer.getContext().errorCallback(T("RTree::loadFromXml"), T("Malloc failed (prediction_values[") + String((int)i) + T("]) !"));
        return false;
      }
      for (size_t j = 0; j < treesState.numPredictions; ++j)
        treesState.prediction_values[i][j] = o->readFloat();
    }
    // attribute_descriptors
    treesState.attribute_descriptors = (int*)MyMalloc(treesState.nb_attributes * sizeof(int));
    if (treesState.attribute_descriptors == NULL)
    {
      importer.getContext().errorCallback(T("RTree::loadFromXml"), T("Malloc failed (attribute_descriptors) !"));
      return false;
    }
    for (size_t i = 0; i < (size_t)treesState.nb_attributes; ++i)
      treesState.attribute_descriptors[i] = o->readInt();

    treesState.get_tree_prediction_vector = get_tree_prediction_vector_classical;
    treesState.getattval = getattval_normal;
    
    set_print_result(0, 0);
    goal_type = MULTIREGR;
    goal = MULTIREGR;
    length_attribute_descriptors = treesState.nb_attributes;
    nb_classes = 0;
    
    return true;
  }
  
protected:
  struct
  {
    size_t maxNumNodes;
    size_t numLeaves;
    size_t numPredictions;
    int nb_goal_multiregr;
    int current_nb_of_ensemble_terms;
    int* ltrees; // need to be copied form 0 to current_nb_of_ensemble_terms
    float* ltrees_weight; // need to be copied form 0 to current_nb_of_ensemble_terms
    int average_predictions_ltrees;
    float *(*get_tree_prediction_vector)(int tree, int obj); // normaly, this fonction is constant
                                                             // get_tree_prediction_vector_classical
    int* left_successor;
    int* right_successor;
    int* tested_attribute;
    float** prediction_values;
    int size_current_tree_table_pred;
    int* prediction;
    union threshold_type *threshold;
    int* attribute_descriptors;
    int nb_attributes;
    float (*getattval)(int obj, int att); // normaly, this fonction is constant
                                          // getattval_normal
  } treesState;
};

typedef ReferenceCountedObjectPtr<RTreeFunction> RTreeFunctionPtr;
typedef ReferenceCountedObjectPtr<RTree> RTreePtr;

extern BatchLearnerPtr rTreeBatchLearner(bool verbose = false);

}; /* namespace lbcpp */

/*
** RTreeFunction
*/

RTreeFunction::RTreeFunction(size_t numTrees,
                             size_t numAttributeSamplesPerSplit,
                             size_t minimumSizeForSplitting,
                             bool verbose)
  : numTrees(numTrees), 
    numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
    minimumSizeForSplitting(minimumSizeForSplitting)
  {setBatchLearner(filterUnsupervisedExamplesBatchLearner(rTreeBatchLearner(verbose)));}

RTreeFunction::RTreeFunction() : numTrees(100), numAttributeSamplesPerSplit(10), minimumSizeForSplitting(1)
  {setBatchLearner(filterUnsupervisedExamplesBatchLearner(rTreeBatchLearner()));}

Variable RTreeFunction::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  if (!trees)
    return Variable::missingValue(getOutputType());
  return trees.staticCast<RTree>()->makePrediction(context, inputs[0], getInputVariable(1)->getType());
}

void RTreeFunction::saveToXml(XmlExporter& exporter) const
{
  Object::saveToXml(exporter);
  exporter.enter(T("rTree"));
  trees->saveToXml(exporter);
  exporter.leave();
}

bool RTreeFunction::loadFromXml(XmlImporter& importer)
{
  if (!Object::loadFromXml(importer))
    return false;
  if (!importer.enter(T("rTree")))
    return false;
  trees = new RTree(0,0,0);
  if (!trees->loadFromXml(importer))
    return false;
  importer.leave();
  return true;
}


/*
** RTreeBatchLearner
*/
void exportData(ExecutionContext& context)
{
  File f = File::getCurrentWorkingDirectory().getChildFile(T("x.train"));
  if (f.exists())
    f.deleteFile();
  OutputStream* o = f.createOutputStream();
  for (size_t i = 0; i < (size_t)nb_obj_in_core_table; ++i)
  {
    for (size_t j = 0; j < (size_t)nb_attributes; ++j)
      *o << core_table[nb_obj_in_core_table * j + i] << " ";
    *o << "\n";
  }
  delete o;
  
  f = File::getCurrentWorkingDirectory().getChildFile(T("y.train"));
  if (f.exists())
    f.deleteFile();
  o = f.createOutputStream();
  for (size_t i = 0; i < (size_t)nb_obj_in_core_table; ++i)
    *o << core_table_y[i] << "\n";
  delete o;
  
  context.informationCallback(T("RTreeBatchLearner::exportData"), T("Training data saved: ") + f.getFullPathName());
}

bool RTreeBatchLearner::train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
{
  ScopedLock _(learnerLock);
  const RTreeFunctionPtr& rTreeFunction = function.staticCast<RTreeFunction>();

  if (!checkHasAtLeastOneExemples(trainingData))
  {
    context.errorCallback(T("No training examples"));
    return false;
  }

  size_t n = trainingData.size();
  nb_attributes = trainingData[0]->getVariable(0).getObjectAndCast<Container>()->getNumElements();
  const size_t numAttributeSamplesPerSplit = rTreeFunction->getNumAttributeSamplesPerSplit() ? juce::jmin(rTreeFunction->getNumAttributeSamplesPerSplit(), nb_attributes) : (size_t)sqrt((double)nb_attributes);
  context.resultCallback(T("Num Attributes"), nb_attributes);
  context.resultCallback(T("K"), numAttributeSamplesPerSplit);
  context.resultCallback(T("nmin"), rTreeFunction->getMinimumSizeForSplitting());
  context.resultCallback(T("Num Examples"), n);
  
  set_print_result(0, 0);
  goal_type = MULTIREGR;
  goal = MULTIREGR;
  jassert(nb_attributes);
  nb_obj_in_core_table = n;
  
  core_table = (CORETABLE_TYPE *)MyMalloc((size_t)nb_obj_in_core_table * (size_t)nb_attributes * sizeof(CORETABLE_TYPE));
  for (size_t i = 0; i < (size_t)nb_obj_in_core_table; ++i)
  {
    ContainerPtr obj = trainingData[i]->getVariable(0).getObjectAndCast<Container>(); // training inputs
    jassert(obj->getNumElements() == (size_t)nb_attributes);
    for (size_t j = 0; j < (size_t)nb_attributes; ++j)
    {
      Variable objVariable = obj->getElement(j);
      TypePtr objType =  objVariable.getType();
      CORETABLE_TYPE value;
      if (objType->inheritsFrom(booleanType))
        value = (CORETABLE_TYPE)(objVariable.getBoolean() ? 1 : 0);
      else if (objType->inheritsFrom(enumValueType))
        value = (CORETABLE_TYPE)objVariable.getInteger();
      else if (objType->inheritsFrom(doubleType))
        value = (CORETABLE_TYPE)objVariable.getDouble();
      else if (objType->inheritsFrom(integerType))
        value = (CORETABLE_TYPE)objVariable.getInteger();
      else
        jassertfalse;

      core_table[nb_obj_in_core_table * j + i] = value;
    }
  }

  length_attribute_descriptors = nb_attributes;
  attribute_descriptors = (int*)MyMalloc((size_t)nb_attributes * sizeof(int));
  ContainerPtr firstData = trainingData[0]->getVariable(0).getObjectAndCast<Container>(context);
  for (size_t i = 0; i < (size_t)nb_attributes; ++i)
  {
    TypePtr attrType = firstData->getElement(i).getType();
    int value;
    if (attrType->inheritsFrom(booleanType))
      value = 2;
    else if (attrType->inheritsFrom(enumValueType))
      value = attrType.dynamicCast<Enumeration>()->getNumElements() + 1;
    else if (attrType->inheritsFrom(doubleType))
      value = 0;
    else if (attrType->inheritsFrom(integerType))
      value = 0;
    else if (attrType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
      value = attrType->getTemplateArgument(0).dynamicCast<Enumeration>()->getNumElements() + 1;
    else
      jassertfalse;

    attribute_descriptors[i] = value;
  }
  
  nb_classes = 0;

  attribute_vector = (int*)MyMalloc((size_t)nb_attributes * sizeof(int));
  for (size_t i = 0; i < (size_t)nb_attributes; ++i)
    attribute_vector[i] = i;
  getattval = getattval_normal;

  global_learning_set_size = nb_obj_in_core_table;
  current_learning_set_size = nb_obj_in_core_table;
  
  object_mapping = (int *)MyMalloc((size_t)nb_obj_in_core_table * sizeof(int));
  current_learning_set = (int *)MyMalloc((size_t)nb_obj_in_core_table * sizeof(int));
  
  for (size_t i = 0; i < (size_t)nb_obj_in_core_table; ++i)
  {
    object_mapping[i] = i;
    current_learning_set[i] = i;
  }
  
  object_weight = (SCORE_TYPE *)MyMalloc((size_t)nb_obj_in_core_table * sizeof(SCORE_TYPE));
  for (size_t i = 0; i < (size_t)nb_obj_in_core_table; ++i)
    object_weight[i] = 1.0;
  
  TypePtr outputType = trainingData[0]->getVariable(1).getType();
  nb_goal_multiregr = outputType->inheritsFrom(enumValueType) ? outputType.dynamicCast<Enumeration>()->getNumElements() : 1;
  core_table_y = (CORETABLE_TYPE *)MyMalloc((size_t)nb_obj_in_core_table * (size_t)nb_goal_multiregr * sizeof(CORETABLE_TYPE));
  for (size_t i = 0; i < (size_t)nb_obj_in_core_table; ++i)
  {
    Variable objVariable = trainingData[i]->getVariable(1);
    if (nb_goal_multiregr == 1)
    {
      CORETABLE_TYPE value;
      if (outputType->inheritsFrom(booleanType))
        value = (CORETABLE_TYPE)(objVariable.getBoolean() ? 1 : 0);
      else if (outputType->inheritsFrom(doubleType))
        value = (CORETABLE_TYPE)objVariable.getDouble();
      else if (outputType->inheritsFrom(integerType))
        value = (CORETABLE_TYPE)objVariable.getInteger();
      else if (outputType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
        value = (CORETABLE_TYPE)objVariable.getObjectAndCast<DoubleVector>()->getIndexOfMaximumValue();
      else
        jassertfalse;
      core_table_y[i] = value;
    }
    else
    {
      for (size_t j = 0; j < (size_t)nb_goal_multiregr; ++j)
        core_table_y[j * nb_obj_in_core_table + i] = (CORETABLE_TYPE)0;
      core_table_y[nb_obj_in_core_table * objVariable.getInteger() + i] = (CORETABLE_TYPE)1;
    }
  }

  goal_multiregr = (int*)MyMalloc((size_t)nb_goal_multiregr * sizeof(int));
  for (size_t i = 0; i < (size_t)nb_goal_multiregr; ++i)
    goal_multiregr[i] = (int)i;
  
  getobjy_multiregr_learn = getobjy_multiregr_learn_matlab;

  init_multiregr_trees(rTreeFunction->getMinimumSizeForSplitting(), 0.0, 1.0,/*savepred*/1);
  set_test_classical();
  set_best_first(/*bestfirst*/0, 0, /*maxnbsplits*/5);
  
  set_find_a_threshold_num_function_multiregr(3,1);
  set_find_a_threshold_symb_function_multiregr(1);
  set_find_a_test_function(6, 10.0, numAttributeSamplesPerSplit, numAttributeSamplesPerSplit);
  
  set_ensemble_method_parameters(/*method*/0, rTreeFunction->getNumTrees(), 1, 0, 1, 0, 0, 1.0, z_max);

  set_sorting_variant(LOCAL_SORT);
  init_sorting_method();
  init_save_ensemble_ls(1);
  
  int maxnbnodes = number_of_ensemble_terms
  * (best_first * (2 * best_first_max_nb_tests + 1)
     + (1 - best_first) * (2 * nb_obj_in_core_table - 1));
  int nb_leaves = (int)ceil((double)(maxnbnodes + number_of_ensemble_terms) / 2);
  int nb_pred = multiregr_savepred * nb_goal_multiregr;
  allocate_tree_tables(maxnbnodes, nb_leaves, nb_pred, 0);
  allocate_multiregr_table_score(nb_goal_multiregr);
  
  clean_all_trees();
  /* construction de l'ensemble d'arbres */
  rtree_context = &context;
  rtree_progress = new ProgressionState(0, rTreeFunction->getNumTrees(), "Trees");
  build_one_tree_ensemble(NULL, 0);

  /* Calcul l'importance des variables */
  SCORE_TYPE* varimp = (SCORE_TYPE*)MyMalloc(nb_attributes * sizeof(SCORE_TYPE));
  average_predictions_ltrees = 1;
  compute_ltrees_variable_importance_multiregr(varimp, -1);
  
  std::vector<double> variableImportances(nb_attributes);
  for (size_t i = 0; i < (size_t)nb_attributes; ++i)
    for (size_t j = 0; j < (size_t)nb_attributes; ++j)
      variableImportances[j] = varimp[j];

  rTreeFunction->setVariableImportances(variableImportances);

  std::vector<size_t> ranks;
  rTreeFunction->getRankedMapping(false, ranks);

  const EnumerationPtr attributesEnum = trainingData[0]->getVariable(0).getObjectAndCast<Container>()->getElementsEnumeration();
  if (verbose && attributesEnum)
  {
    for (size_t j = 0; j < (size_t)nb_attributes; ++j)
    {
      context.enterScope(T("Importance of ") + attributesEnum->getElementName(j));
      context.resultCallback(T("Rank"), ranks[j]);
      context.resultCallback(T("Score"), variableImportances[j]);
      context.resultCallback(T("Index"), j);
      context.resultCallback(T("Name"), attributesEnum->getElementName(j));
      context.leaveScope(variableImportances[j]);
    }
    MyFreeAndNull(varimp);
  }

  /* Sauvegarde de l'arbre */
  RTreePtr trees = new RTree(maxnbnodes, nb_leaves, nb_pred);
  trees->saveTreesState();
  rTreeFunction->setTrees(trees);

  /* Libération de la mémoire */
  MyFreeAndNull(attribute_vector);
  MyFreeAndNull(goal_multiregr);
  MyFreeAndNull(object_mapping);
  MyFreeAndNull(current_learning_set);
  MyFreeAndNull(object_weight);
  MyFreeAndNull(core_table_y);
  MyFreeAndNull(node_weight);
  MyFreeAndNull(node_size);
  free_table_score_type(table_score, 3);
  free_table_score_type(table_score_symb, MAX_NUMBER_OF_SYMBOLIC_VALUES);
  free_multiregr_table_score();
  MyFreeAndNull(save_ensemble_ls_vector);
  MyFreeAndNull(save_ensemble_ls_weight);
  MyFreeAndNull(core_table);

  return true;
}
