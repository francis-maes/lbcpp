INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${LBCpp_SOURCE_DIR}/shark/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(LBCppLibrary.xml LBCppLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Core/Type/CoreTypeLibrary.xml CoreTypeLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Core/CoreLibrary.xml CoreLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Data/Stream/StreamLibrary.xml StreamLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Data/Container/ContainerLibrary.xml ContainerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Data/Matrix/MatrixLibrary.xml MatrixLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Data/DataLibrary.xml DataLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Execution/ExecutionLibrary.xml ExecutionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Callback/ExecutionCallbackLibrary.xml ExecutionCallbackLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Context/ExecutionContextLibrary.xml ExecutionContextLibrary.cpp)

GENERATE_INTROSPECTION_CODE(LossFunction/LossFunctionLibrary.xml LossFunctionLibrary.cpp)

GENERATE_INTROSPECTION_CODE(DecisionProblem/DecisionProblemLibrary.xml DecisionProblemLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Optimizer/OptimizerLibrary.xml OptimizerLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Luape/LuapeLibrary.xml LuapeLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Luape/NodeBuilder/ExpressionBuilderLibrary.xml ExpressionBuilderLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Luape/Learner/LuapeLearnerLibrary.xml LuapeLearnerLibrary.cpp)

IF(LBCPP_USER_INTERFACE)
  GENERATE_INTROSPECTION_CODE(UserInterface/Component/UserInterfaceComponentLibrary.xml UserInterfaceComponentLibrary.cpp)
ENDIF(LBCPP_USER_INTERFACE)

GENERATE_INTROSPECTION_CODE(Lua/LuaLibrary.xml LuaLibrary.cpp)

#### Core ####

SET(LBCPP_CORE_SOURCES
  ../include/lbcpp/Core/Utilities.h  
  Core/Utilities.cpp
  ../include/lbcpp/Core/ReferenceCountedObject.h
  ../include/lbcpp/Core/predeclarations.h
  ../include/lbcpp/Core/Object.h
  Core/Object.cpp
  ../include/lbcpp/Core/TypeManager.h
  Core/TypeManager.cpp
  ../include/lbcpp/Core/Signature.h
  ../include/lbcpp/Core/Type.h
  Core/Type.cpp
  ../include/lbcpp/Core/Enumeration.h
  Core/Enumeration.cpp
  ../include/lbcpp/Core/Class.h
  Core/Class.cpp
  ../include/lbcpp/Core/TemplateType.h
  Core/TemplateType.cpp
  ../include/lbcpp/Core/Library.h
  ../include/lbcpp/Core/Variable.h
  Core/Variable.cpp
  ../include/lbcpp/Core/Pair.h
  ../include/lbcpp/Core/Container.h
  Core/Container.cpp
  ../include/lbcpp/Core/Vector.h
  Core/Vector.cpp
  ../include/lbcpp/Core/Map.h
  Core/Map.cpp
  ../include/lbcpp/Core/OrderedMap.h
  Core/OrderedMap.cpp
  ../include/lbcpp/Core/DynamicObject.h
  Core/DynamicObject.cpp
  ../include/lbcpp/Core/XmlSerialisation.h
  Core/XmlSerialisation.cpp
  Core/CoreLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/CoreLibrary.cpp
)

SET(LBCPP_CORE_IMPL_HEADERS
  ../include/lbcpp/Core/impl/VariableValue.hpp
  ../include/lbcpp/Core/impl/Variable.hpp
)

SET(LBCPP_CORE_TYPE_SOURCES
  Core/Type/TopLevelType.h
  Core/Type/NilType.h
  Core/Type/BuiltinType.h
  Core/Type/BooleanType.h
  Core/Type/IntegerType.h
  Core/Type/DoubleType.h
  Core/Type/StringType.h
  Core/Type/FileType.h
  Core/Type/CoreTypeLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/CoreTypeLibrary.cpp
)

SET(LBCPP_CORE_OBJECT_SOURCES
  Core/Object/SparseVectorHelper.h
  Core/Object/SparseGenericObject.h
  Core/Object/SparseDoubleObject.h
  Core/Object/DenseGenericObject.h
  Core/Object/DenseDoubleObject.h
  Core/Object/DenseObjectObject.h
)

#### Data ####

SET(LBCPP_DATA_SOURCES
  ../include/lbcpp/Data/predeclarations.h
  ../include/lbcpp/Data/Stream.h
  Data/Stream.cpp
  ../include/lbcpp/Data/DoubleVector.h
  Data/FeatureGeneratorCallbacks.hpp
  Data/DoubleVector.cpp
  ../include/lbcpp/Data/Matrix.h
  ../include/lbcpp/Data/SymmetricMatrix.h
  ../include/lbcpp/Data/RandomGenerator.h
  Data/RandomGenerator.cpp  
  ../include/lbcpp/Data/RandomVariable.h  
  Data/RandomVariable.cpp
  ../include/lbcpp/Data/Consumer.h
  Data/Consumer.cpp
  ../include/lbcpp/Data/Cache.h
  Data/Cache.cpp
  ../include/lbcpp/Data/IndexSet.h
  Data/IndexSet.cpp
  ../include/lbcpp/Data/BinaryKey.h
  ../include/lbcpp/Data/BinaryConfusionMatrix.h
  Data/BinaryConfusionMatrix.cpp
  ../include/lbcpp/Data/IterationFunction.h
  Data/IterationFunction.hpp

  Data/DataLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DataLibrary.cpp
)

SET(LBCPP_DATA_MATRIX_SOURCES
  Data/Matrix/Matrix.cpp
  Data/Matrix/UpperLowerSquareMatrix.h
  Data/Matrix/MatrixLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MatrixLibrary.cpp
)

SET(LBCPP_DATA_STREAM_SOURCES
  Data/Stream/DirectoryFileStream.h
  Data/Stream/DirectoryPairFileStream.h
  Data/Stream/LibSVMDataParser.h
  Data/Stream/LibSVMDataParser.cpp
  Data/Stream/ARFFDataParser.h
  Data/Stream/ARFFDataParser.cpp
  Data/Stream/SaveToFileConsumer.h
  Data/Stream/BuiltinTypeStream.h
  Data/Stream/StreamLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/StreamLibrary.cpp
)

SET(LBCPP_DATA_CONTAINER_SOURCES
  Data/Container/SubsetContainer.h
  Data/Container/DuplicatedContainer.h
  Data/Container/RangeContainer.h
  Data/Container/ExcludeRangeContainer.h
  Data/Container/ContainerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ContainerLibrary.cpp
)

#### Execution ####

SET(LBCPP_EXECUTION_SOURCES
  ../include/lbcpp/Execution/predeclarations.h
  ../include/lbcpp/Execution/WorkUnit.h
  Execution/WorkUnit.cpp
  ../include/lbcpp/Execution/TestUnit.h
  ../include/lbcpp/Execution/ExecutionStack.h
  ../include/lbcpp/Execution/ExecutionCallback.h
  Execution/ExecutionCallback.cpp
  ../include/lbcpp/Execution/ExecutionContext.h
  Execution/ExecutionContext.cpp
  ../include/lbcpp/Execution/ExecutionContextCallback.h
  ../include/lbcpp/Execution/ExecutionTrace.h
  Execution/ExecutionTrace.cpp
  ../include/lbcpp/Execution/Notification.h
  Execution/Notification.cpp
  Execution/ExecutionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionLibrary.cpp
)

SET(LBCPP_EXECUTION_CALLBACK_SOURCES
  Execution/Callback/ExecutionNotifications.h
  Execution/Callback/ConsoleExecutionCallback.h
  Execution/Callback/MakeTraceExecutionCallback.h
  Execution/Callback/MakeAndAutoSaveTraceExecutionCallback.h
  Execution/Callback/ExecutionCallbackLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionCallbackLibrary.cpp
)
IF(LBCPP_USER_INTERFACE)
  SET(LBCPP_EXECUTION_CALLBACK_SOURCES ${LBCPP_EXECUTION_CALLBACK_SOURCES} Execution/Callback/UserInterfaceExecutionCallback.h)
ENDIF(LBCPP_USER_INTERFACE)


SET(LBCPP_EXECUTION_CONTEXT_SOURCES
  Execution/Context/SingleThreadedExecutionContext.h
  Execution/Context/SubExecutionContext.h
  Execution/Context/MultiThreadedExecutionContext.h
  Execution/Context/ExecutionContextLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionContextLibrary.cpp
)

#### Learning ####

SET(LBCPP_LEARNING_LOSS_FUNCTION_SOURCES
  ../include/lbcpp/Learning/LossFunction.h
  LossFunction/LossFunction.cpp
  LossFunction/LossFunctionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LossFunctionLibrary.cpp
)

SET(LBCPP_LEARNING_LOSS_FUNCTION_REGRESSION_SOURCES
  LossFunction/SquareRegressionLossFunction.h
)

SET(LBCPP_LEARNING_LOSS_FUNCTION_DISCRIMINATIVE_SOURCES
  LossFunction/HingeDiscriminativeLossFunction.h
  LossFunction/LogBinomialDiscriminativeLossFunction.h
  LossFunction/ExponentialDiscriminativeLossFunction.h
)

SET(LBCPP_LEARNING_LOSS_FUNCTION_MULTI_CLASS_SOURCES
  LossFunction/OneAgainstAllMultiClassLossFunction.h
  LossFunction/MostViolatedMultiClassLossFunction.h
  LossFunction/LogBinomialMultiClassLossFunction.h
)

SET(LBCPP_LEARNING_LOSS_FUNCTION_RANKING_SOURCES
  LossFunction/AdditiveRankingLossFunction.h
  LossFunction/AllPairsRankingLossFunction.h
  LossFunction/MostViolatedPairRankingLossFunction.h
  LossFunction/BestAgainstAllRankingLossFunction.h
)

#### Decision Problem ####

SET(LBCPP_DECISION_PROBLEM_SOURCES
  ../include/lbcpp/DecisionProblem/DecisionProblem.h
  DecisionProblem/DecisionProblem.cpp
  ../include/lbcpp/DecisionProblem/Policy.h
  DecisionProblem/DecisionProblemLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DecisionProblemLibrary.cpp
)

#### Optimizer ####

SET(LBCPP_OPTIMIZER_SOURCES
  ../include/lbcpp/Optimizer/BanditPool.h
  Optimizer/BanditPool.cpp
  Optimizer/OptimizerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OptimizerLibrary.cpp
)

#### Luape ####

SET(LBCPP_LUAPE_SOURCES
  ../include/lbcpp/Luape/predeclarations.h
  ../include/lbcpp/Luape/LuapeCache.h
  Luape/LuapeCache.cpp
  ../include/lbcpp/Luape/LuapeInference.h
  Luape/LuapeInference.cpp
  ../include/lbcpp/Luape/ExpressionBuilder.h
  Luape/ExpressionBuilder.cpp
  ../include/lbcpp/Luape/LearningObjective.h
  Luape/LearningObjective.cpp
  ../include/lbcpp/Luape/LuapeLearner.h
  Luape/LuapeLearner.cpp
  Luape/LuapeLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LuapeLibrary.cpp
)

SET(LBCPP_LUAPE_NODE_BUILDER_SOURCES
  Luape/NodeBuilder/NodeBuilderDecisionProblem.h
  Luape/NodeBuilder/NodeBuilderTypeSearchSpace.h
  Luape/NodeBuilder/SingletonNodeBuilder.h
  Luape/NodeBuilder/CompositeNodeBuilder.h
  Luape/NodeBuilder/InputsNodeBuilder.h
  Luape/NodeBuilder/ExhaustiveSequentialNodeBuilder.h
  Luape/NodeBuilder/PolicyBasedNodeBuilder.h
  Luape/NodeBuilder/RandomSequentialNodeBuilder.h
  Luape/NodeBuilder/AdaptativeSamplingNodeBuilder.h  
  Luape/NodeBuilder/ExpressionBuilderLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExpressionBuilderLibrary.cpp
)

SET(LBCPP_LUAPE_LEARNER_SOURCES
  Luape/Learner/GradientDescentLearner.h
  Luape/Learner/GenerateTestNodesLearner.h
  Luape/Learner/LuapeLearnerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LuapeLearnerLibrary.cpp
)

SET(LBCPP_LUAPE_LEARNER_BOOSTING_SOURCES
  Luape/Learner/BoostingLearner.h
  Luape/Learner/WeightBoostingLearner.h
  Luape/Learner/AdaBoostLearner.h
  Luape/Learner/AdaBoostMHLearner.h
  Luape/Learner/GradientBoostingLearner.h
)

SET(LBCPP_LUAPE_LEARNER_WEAK_SOURCES
  Luape/Learner/ExactWeakLearner.h
  Luape/Learner/LaminatingWeakLearner.h
  Luape/Learner/BanditBasedWeakLearner.h
  Luape/Learner/OptimizerBasedSequentialWeakLearner.h
)

SET(LBCPP_LUAPE_LEARNER_META_SOURCES
  Luape/Learner/TreeLearner.h
  Luape/Learner/EnsembleLearner.h
  Luape/Learner/CompositeLearner.h
  Luape/Learner/AddActiveVariablesLearner.h
)

#### UserInterface ####
IF(LBCPP_USER_INTERFACE)

  # Res input data.
  FILE(GLOB LBCPP_USER_INTERFACE_DATA_FILES
    UserInterface/Data/*.png
  )

  # Where is jucebinbuild?
  GET_TARGET_PROPERTY(JUCE_BIN_BUILD_COMMAND jucebinbuild LOCATION)

  # The generated files.
  SET(LBCPP_USER_INTERFACE_DATA_CPP ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceData.cpp)
  SET(LBCPP_USER_INTERFACE_DATA_H ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceData.h)

  # Main commmand for resources generation.
  ADD_CUSTOM_COMMAND(
    OUTPUT ${LBCPP_USER_INTERFACE_DATA_CPP}
    COMMAND ${JUCE_BIN_BUILD_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface/Data ${CMAKE_CURRENT_BINARY_DIR} UserInterfaceData precompiled.h
    DEPENDS ${LBCPP_USER_INTERFACE_DATA_FILES} ${JUCE_BIN_BUILD_COMMAND} 
    COMMENT "User Interface Data"
  )
  # Dummy custom command for second generated file.
  ADD_CUSTOM_COMMAND(
    OUTPUT ${LBCPP_USER_INTERFACE_DATA_H}
    COMMAND echo
    DEPENDS  ${LBCPP_USER_INTERFACE_DATA_FILES} ${JUCE_BIN_BUILD_COMMAND}
  )

  SET(LBCPP_USER_INTEFACE_GENERATED_FILES
    ${LBCPP_USER_INTERFACE_DATA_H}
    ${LBCPP_USER_INTERFACE_DATA_CPP}
  )

  #IDE_ONLY_SOURCES(LBCPP_USER_INTEFACE_GENERATED_FILES ${LBCPP_USER_INTEFACE_GENERATED_FILES})

  SET(LBCPP_USER_INTERFACE_SOURCES
    ../include/lbcpp/UserInterface/SimpleTreeViewItem.h
    ../include/lbcpp/UserInterface/VariableSelector.h
    ../include/lbcpp/UserInterface/ComponentWithPreferedSize.h
    ../include/lbcpp/UserInterface/UserInterfaceManager.h
    ../include/lbcpp/UserInterface/ObjectEditor.h
    ../include/lbcpp/UserInterface/MatrixComponent.h
    ../include/lbcpp/UserInterface/PieChartComponent.h
    ../include/lbcpp/UserInterface/HistogramComponent.h
    ../include/lbcpp/UserInterface/PlotAxis.h
    UserInterface/UserInterfaceManager.cpp
    ${LBCPP_USER_INTEFACE_GENERATED_FILES}
  )

  SET(LBCPP_USER_INTERFACE_COMPONENT_SOURCES
    UserInterface/Component/ObjectEditor.cpp
    UserInterface/Component/VariableTreeView.h
    UserInterface/Component/VariableTreeView.cpp
    UserInterface/Component/ContainerTableListBox.h
    UserInterface/Component/ContainerTableListBox.cpp
    UserInterface/Component/ContainerCurveEditor.h
    UserInterface/Component/ContainerCurveEditor.cpp
    UserInterface/Component/ExecutionTraceTreeViewItem.h
    UserInterface/Component/ExecutionTraceTreeViewItem.cpp
    UserInterface/Component/ExecutionTraceTreeView.h
    UserInterface/Component/ExecutionTraceTreeView.cpp
    UserInterface/Component/ConfusionMatrixComponent.h
    UserInterface/Component/DoubleMatrixComponent.h
    UserInterface/Component/UserInterfaceComponentLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceComponentLibrary.cpp
  )
  
  SET(LBCPP_USER_INTERFACE_PLOT_SOURCES
    UserInterface/Plot/TwoDimensionalPlotDrawable.h
    UserInterface/Plot/TwoDimensionalPlotDrawable.cpp
  )

ENDIF(LBCPP_USER_INTERFACE)

SET(LBCPP_LUA_SOURCES
  ../include/lbcpp/Lua/Lua.h
  Lua/Lua.cpp
  Lua/LuaLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LuaLibrary.cpp
)

##### MAIN ####

SET(LBCPP_MAIN_SOURCES
  ../include/lbcpp/common.h
  precompiled.h
  precompiled.cpp
  ../include/lbcpp/lbcpp.h
  ../include/lbcpp/library.h
  library.cpp
  LBCppLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LBCppLibrary.cpp
)

SET(LBCPP_SOURCES
  ${LBCPP_MAIN_SOURCES}

  # Core
  ${LBCPP_CORE_SOURCES}
    ${LBCPP_CORE_IMPL_HEADERS}
    ${LBCPP_CORE_TYPE_SOURCES}
    ${LBCPP_CORE_OBJECT_SOURCES}

  # Data
  ${LBCPP_DATA_SOURCES}
    ${LBCPP_DATA_STREAM_SOURCES}
    ${LBCPP_DATA_CONTAINER_SOURCES}
    ${LBCPP_DATA_MATRIX_SOURCES}
  
  # Execution
  ${LBCPP_EXECUTION_SOURCES}
    ${LBCPP_EXECUTION_CALLBACK_SOURCES}
    ${LBCPP_EXECUTION_CONTEXT_SOURCES}
  
  # Loss Function
  ${LBCPP_LEARNING_LOSS_FUNCTION_SOURCES}
    ${LBCPP_LEARNING_LOSS_FUNCTION_REGRESSION_SOURCES}
    ${LBCPP_LEARNING_LOSS_FUNCTION_DISCRIMINATIVE_SOURCES}
    ${LBCPP_LEARNING_LOSS_FUNCTION_MULTI_CLASS_SOURCES}
    ${LBCPP_LEARNING_LOSS_FUNCTION_RANKING_SOURCES}

  # Decision Problem
  ${LBCPP_DECISION_PROBLEM_SOURCES}
 
  # Optimizer
  ${LBCPP_OPTIMIZER_SOURCES}
 
  # Luape
  ${LBCPP_LUAPE_SOURCES}
    ${LBCPP_LUAPE_NODE_BUILDER_SOURCES}
    ${LBCPP_LUAPE_LEARNER_SOURCES}
      ${LBCPP_LUAPE_LEARNER_BOOSTING_SOURCES}
      ${LBCPP_LUAPE_LEARNER_WEAK_SOURCES}
      ${LBCPP_LUAPE_LEARNER_META_SOURCES}	  
 
  # Lua
  ${LBCPP_LUA_SOURCES}
    
  # User Interface
  ${LBCPP_USER_INTERFACE_SOURCES}
    ${LBCPP_USER_INTERFACE_COMPONENT_SOURCES}
    ${LBCPP_USER_INTERFACE_PLOT_SOURCES}
    ${LBCPP_USER_INTERFACE_DATA_FILES}
)

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_SOURCES)

## Main
SOURCE_GROUP(Main FILES ${LBCPP_MAIN_SOURCES})

## Core
SOURCE_GROUP(Core FILES ${LBCPP_CORE_SOURCES})
SOURCE_SUB_GROUP(Core impl FILES ${LBCPP_CORE_IMPL_HEADERS})
SOURCE_SUB_GROUP(Core Type FILES ${LBCPP_CORE_TYPE_SOURCES})
SOURCE_SUB_GROUP(Core Object FILES ${LBCPP_CORE_OBJECT_SOURCES})

## Data
SOURCE_GROUP(Data FILES ${LBCPP_DATA_SOURCES})
SOURCE_SUB_GROUP(Data Stream FILES ${LBCPP_DATA_STREAM_SOURCES})
SOURCE_SUB_GROUP(Data Container FILES ${LBCPP_DATA_CONTAINER_SOURCES})
SOURCE_SUB_GROUP(Data Matrix FILES ${LBCPP_DATA_MATRIX_SOURCES})

## Execution
SOURCE_GROUP(Execution FILES ${LBCPP_EXECUTION_SOURCES})
SOURCE_SUB_GROUP(Execution Callback FILES ${LBCPP_EXECUTION_CALLBACK_SOURCES})
SOURCE_SUB_GROUP(Execution Context FILES ${LBCPP_EXECUTION_CONTEXT_SOURCES})

## Learning
SOURCE_SUB_GROUP(CleanMe LossFunction FILES ${LBCPP_LEARNING_LOSS_FUNCTION_SOURCES})
SOURCE_SUB_GROUP2(CleanMe LossFunction Regression FILES ${LBCPP_LEARNING_LOSS_FUNCTION_REGRESSION_SOURCES})
SOURCE_SUB_GROUP2(CleanMe LossFunction Discriminative FILES ${LBCPP_LEARNING_LOSS_FUNCTION_DISCRIMINATIVE_SOURCES})
SOURCE_SUB_GROUP2(CleanMe LossFunction MultiClass FILES ${LBCPP_LEARNING_LOSS_FUNCTION_MULTI_CLASS_SOURCES})
SOURCE_SUB_GROUP2(CleanMe LossFunction Ranking FILES ${LBCPP_LEARNING_LOSS_FUNCTION_RANKING_SOURCES})

## DecisionProblem
SOURCE_SUB_GROUP(CleanMe DecisionProblem FILES ${LBCPP_DECISION_PROBLEM_SOURCES})

## Optimizer
SOURCE_SUB_GROUP(CleanMe Optimizer FILES ${LBCPP_OPTIMIZER_SOURCES})

## Luape
SOURCE_SUB_GROUP(CleanMe Luape FILES ${LBCPP_LUAPE_SOURCES})
SOURCE_SUB_GROUP2(CleanMe Luape NodeBuilder FILES ${LBCPP_LUAPE_NODE_BUILDER_SOURCES})
SOURCE_SUB_GROUP2(CleanMe Luape Learner FILES ${LBCPP_LUAPE_LEARNER_SOURCES})
SOURCE_SUB_GROUP3(CleanMe Luape Learner Boosting FILES ${LBCPP_LUAPE_LEARNER_BOOSTING_SOURCES})
SOURCE_SUB_GROUP3(CleanMe Luape Learner Weak FILES ${LBCPP_LUAPE_LEARNER_WEAK_SOURCES})
SOURCE_SUB_GROUP3(CleanMe Luape Learner Meta FILES ${LBCPP_LUAPE_LEARNER_META_SOURCES})

## Lua
SOURCE_GROUP(Lua FILES ${LBCPP_LUA_SOURCES})
	  
## UserInterface
SOURCE_GROUP(UserInterface FILES ${LBCPP_USER_INTERFACE_SOURCES})
SOURCE_SUB_GROUP(UserInterface Component FILES ${LBCPP_USER_INTERFACE_COMPONENT_SOURCES})
SOURCE_SUB_GROUP(UserInterface Plot FILES ${LBCPP_USER_INTERFACE_PLOT_SOURCES})
SOURCE_SUB_GROUP(UserInterface Data FILES ${LBCPP_USER_INTERFACE_DATA_FILES})

IF (UNIX)
  ADD_LIBRARY(lbcpp-core SHARED ${LBCPP_SOURCES})
ELSE (UNIX)
  ADD_LIBRARY(lbcpp-core ${LBCPP_SOURCES})
ENDIF (UNIX)
TARGET_LINK_LIBRARIES(lbcpp-core juce lua shark)
