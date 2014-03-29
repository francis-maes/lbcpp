/*-------------------------------------------.-----------------------------------------.
 | Filename: PerceptronTest.h                | Testing Perceptron Code                 |
 | Author  : Denny Verbeeck                  |                                         |
 | Started : 10/12/2013 15:51                |                                         |
 `-------------------------------------------/                                         |
                                  |                                                    |
                                  `---------------------------------------------------*/

#ifndef MOO_REWARDLEARNER_TEST_H_
# define MOO_REWARDLEARNER_TEST_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Solver.h>
# include <ml/IncrementalLearner.h>
# include <ml/SelectionCriterion.h>
# include <ml/Domain.h>
# include <ml/IncrementalLearner.h>

#include <stdio.h>  /* for printf */
#include <string.h> /* for strcmp */
#include <time.h> /*for time()*/
#include <rlglue/utils/C/RLStruct_util.h> /* helpful functions for allocating structs and cleaning them up */
#include <rlglue/network/RL_network.h>

// TODO: make attribute limits parameters

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class RLExperimentWorkUnit : public WorkUnit
{
public:

  int theExperimentConnection;

observation_t clientexp_observation;
action_t clientexp_action;
rlBuffer clientexp_rlbuffer;

char* clientexp_message;
unsigned int clientexp_messagecapacity;

  virtual ObjectPtr run(ExecutionContext& context)
  {
    char* host = kLocalHost;
	  short port = kDefaultPort;

    clientexp_messagecapacity = 0;
    clientexp_message = 0;

    allocateRLStruct(&clientexp_observation, 0, 0, 0);
    allocateRLStruct(&clientexp_action, 0, 0, 0);
    

    rlBufferCreate(&clientexp_rlbuffer, 4096);
    theExperimentConnection = rlWaitForConnection(host, port, kRetryTimeout);
    rlBufferClear(&clientexp_rlbuffer);
    rlSendBufferData(theExperimentConnection, &clientexp_rlbuffer, kExperimentConnection);
    
    const char* theTaskSpec = RL_init();
    context.enterScope("Experiment: Learning episodes");
    for (size_t i = 0; i < 100; ++i)
    {
      context.progressCallback(new ProgressionState(i, 100, "Episodes"));
      RL_episode(1000);
    }
    context.leaveScope();

    //rlClose(theExperimentConnection);
    

    const char* task_spec;
    
    return new Boolean(true);
  }

  void cleanupExperimentAtExit(void)
{
  rlBufferDestroy(&clientexp_rlbuffer);
}

const char* RL_init() {
  unsigned int offset=0;
  unsigned int messageLength=0;
  int experimentState = kRLInit;

  /* Remote call RL_init */
  rlBufferClear(&clientexp_rlbuffer);
  rlSendBufferData(theExperimentConnection, &clientexp_rlbuffer, experimentState);

  /* Recv back a reply from RL_init */
  rlBufferClear(&clientexp_rlbuffer);
  rlRecvBufferData(theExperimentConnection, &clientexp_rlbuffer, &experimentState);
  assert(experimentState == kRLInit);

 /* Brian added Sept 8 so that RL_init returns the task spec */
 /* We'll reuse messageLength and clientexp_message from Agent_message*/
  offset = rlBufferRead(&clientexp_rlbuffer, offset, &messageLength, 1, sizeof(unsigned int));
  if (messageLength >= clientexp_messagecapacity) {
	if(clientexp_message!=0){
    	free(clientexp_message);
		clientexp_message=0;
	}	

    clientexp_message = (char*)calloc(messageLength+1, sizeof(char));
    clientexp_messagecapacity = messageLength;
  }

  if (messageLength > 0) {
    offset = rlBufferRead(&clientexp_rlbuffer, offset, clientexp_message, messageLength, sizeof(char));
  }
  /*Need to move this outside of the if statement, so that we get null termination for empty messages*/
  clientexp_message[messageLength] = '\0';

  return clientexp_message;
}

const observation_action_t *RL_start() {
  int experimentState = kRLStart;
  static observation_action_t oa = { 0,0};
  unsigned int offset = 0;

  assert(theExperimentConnection != 0);

  rlBufferClear(&clientexp_rlbuffer);
  rlSendBufferData(theExperimentConnection, &clientexp_rlbuffer, experimentState);

  rlBufferClear(&clientexp_rlbuffer);
  rlRecvBufferData(theExperimentConnection, &clientexp_rlbuffer, &experimentState); 
  assert(experimentState == kRLStart);

  offset = rlCopyBufferToADT(&clientexp_rlbuffer, offset, &clientexp_observation);
  offset = rlCopyBufferToADT(&clientexp_rlbuffer, offset, &clientexp_action);
	__RL_CHECK_STRUCT(&clientexp_observation)
	__RL_CHECK_STRUCT(&clientexp_action)

  oa.observation = &clientexp_observation;
  oa.action = &clientexp_action;

  return &oa;
}

const reward_observation_action_terminal_t* RL_step() {
  int experimentState = kRLStep;
  static reward_observation_action_terminal_t roat = {0, 0,0, 0};
  unsigned int offset = 0;
  
  assert(theExperimentConnection != 0);

  rlBufferClear(&clientexp_rlbuffer);
  rlSendBufferData(theExperimentConnection, &clientexp_rlbuffer, experimentState);

  /* Recv Data from Server */
  rlBufferClear(&clientexp_rlbuffer);
  rlRecvBufferData(theExperimentConnection, &clientexp_rlbuffer, &experimentState);
  assert(experimentState == kRLStep);

  offset = rlBufferRead(&clientexp_rlbuffer, offset, &roat.terminal, 1, sizeof(int));
  offset = rlBufferRead(&clientexp_rlbuffer, offset, &roat.reward, 1, sizeof(double));
  offset = rlCopyBufferToADT(&clientexp_rlbuffer, offset, &clientexp_observation);
  offset = rlCopyBufferToADT(&clientexp_rlbuffer, offset, &clientexp_action);
	__RL_CHECK_STRUCT(&clientexp_observation)
	__RL_CHECK_STRUCT(&clientexp_action)

  roat.observation = &clientexp_observation;
  roat.action = &clientexp_action;

  return &roat;
}

void RL_cleanup() {
	int experimentState = kRLCleanup;

	assert(theExperimentConnection != 0);

	rlBufferClear(&clientexp_rlbuffer);
	rlSendBufferData(theExperimentConnection, &clientexp_rlbuffer, experimentState);

	rlBufferClear(&clientexp_rlbuffer);
	rlRecvBufferData(theExperimentConnection, &clientexp_rlbuffer, &experimentState);
	assert(experimentState == kRLCleanup);

	clearRLStruct(&clientexp_observation);
	clearRLStruct(&clientexp_action);

	/*safe even if it is null */
	free(clientexp_message);
	clientexp_message = 0;

	clientexp_messagecapacity = 0;
}

double RL_return() {
  int experimentState = kRLReturn;
  double theReward = 0;
  unsigned int offset = 0;

  assert(theExperimentConnection != 0);

  rlBufferClear(&clientexp_rlbuffer);
  rlSendBufferData(theExperimentConnection, &clientexp_rlbuffer, experimentState);

  rlBufferClear(&clientexp_rlbuffer);
  rlRecvBufferData(theExperimentConnection, &clientexp_rlbuffer, &experimentState);
  assert(experimentState == kRLReturn);

  offset = rlBufferRead(&clientexp_rlbuffer, offset, &theReward, 1, sizeof(double));

  return theReward;
}

int RL_num_steps() {
  int experimentState = kRLNumSteps;
  int numSteps = 0;
  unsigned int offset = 0;

  assert(theExperimentConnection != 0);

  rlBufferClear(&clientexp_rlbuffer);
  rlSendBufferData(theExperimentConnection, &clientexp_rlbuffer, experimentState);

  rlBufferClear(&clientexp_rlbuffer);
  rlRecvBufferData(theExperimentConnection, &clientexp_rlbuffer, &experimentState);
  assert(experimentState == kRLNumSteps);

  offset = rlBufferRead(&clientexp_rlbuffer, offset, &numSteps, 1, sizeof(int));

  return numSteps;
}


const char* RL_agent_message(const char* message) {
  int experimentState = kRLAgentMessage;
  unsigned int messageLength = 0;
  unsigned int offset = 0;

  if (message != 0)
    messageLength = strlen(message);
  
  rlBufferClear(&clientexp_rlbuffer);
  offset = 0;
  offset = rlBufferWrite(&clientexp_rlbuffer, offset, &messageLength, 1, sizeof(int));
  if (messageLength > 0) {
    offset = rlBufferWrite(&clientexp_rlbuffer, offset, message, messageLength, sizeof(char));
  }
  rlSendBufferData(theExperimentConnection, &clientexp_rlbuffer, experimentState);
  
  offset = 0;
  rlBufferClear(&clientexp_rlbuffer);
  rlRecvBufferData(theExperimentConnection, &clientexp_rlbuffer, &experimentState);
  assert(experimentState == kRLAgentMessage);

  offset = rlBufferRead(&clientexp_rlbuffer, offset, &messageLength, 1, sizeof(int));
  /* Sept 12 2008 made this >= instead of > so that we'd at least have size 1 */
  if (messageLength >= clientexp_messagecapacity) {
    if(clientexp_message!=0){
    	free(clientexp_message);
		clientexp_message=0;
	}	
    clientexp_message = (char*)calloc(messageLength+1, sizeof(char));
    clientexp_messagecapacity = messageLength;
  }

  if (messageLength > 0) {
    offset = rlBufferRead(&clientexp_rlbuffer, offset, clientexp_message, messageLength, sizeof(char));
  }
  /* Sept 12 2008 moved this out of the if statement so we actually null terminate at the right place if we get a "" message */
  clientexp_message[messageLength] = '\0';

  return clientexp_message;
}


const char* RL_env_message(const char *message) {
	int experimentState = kRLEnvMessage;
	unsigned int messageLength = 0;
	unsigned int offset = 0;

	if (message != 0){
		messageLength = strlen(message);
	}
	rlBufferClear(&clientexp_rlbuffer);
	offset = 0;
	offset = rlBufferWrite(&clientexp_rlbuffer, offset, &messageLength, 1, sizeof(int));
	if (messageLength > 0) {
		offset = rlBufferWrite(&clientexp_rlbuffer, offset, message, messageLength, sizeof(char));
	}
	rlSendBufferData(theExperimentConnection, &clientexp_rlbuffer, experimentState);

	rlBufferClear(&clientexp_rlbuffer);
	rlRecvBufferData(theExperimentConnection, &clientexp_rlbuffer, &experimentState);
	assert(experimentState == kRLEnvMessage);

	offset = 0;
	offset = rlBufferRead(&clientexp_rlbuffer, offset, &messageLength, 1, sizeof(int));

	/* Sept 12 2008 made this >= instead of > so that we'd at least have size 1 */
	if (messageLength >= clientexp_messagecapacity) {
		if(clientexp_message!=0){
			free(clientexp_message);
			clientexp_message=0;
		}	
		clientexp_message = (char*)calloc(messageLength+1, sizeof(char));
		clientexp_messagecapacity = messageLength;
	}

	if (messageLength > 0) {
		offset = rlBufferRead(&clientexp_rlbuffer, offset, clientexp_message, messageLength, sizeof(char));
	}
	/* Sept 12 2008 moved this out of the if statement so we actually null terminate at the right place if we get a "" message */
	clientexp_message[messageLength] = '\0';

	return clientexp_message;
}

int RL_num_episodes() {
	int experimentState = kRLNumEpisodes;
	int numEpisodes = 0;
	unsigned int offset = 0;

	assert(theExperimentConnection != 0);

	rlBufferClear(&clientexp_rlbuffer);
	rlSendBufferData(theExperimentConnection, &clientexp_rlbuffer, experimentState);

	rlBufferClear(&clientexp_rlbuffer);
	rlRecvBufferData(theExperimentConnection, &clientexp_rlbuffer, &experimentState);
	assert(experimentState == kRLNumEpisodes);

	offset = rlBufferRead(&clientexp_rlbuffer, offset, &numEpisodes, 1, sizeof(int));

	return numEpisodes;
}

int RL_episode(unsigned int numSteps) {
	int terminal=0;
	unsigned int offset = 0;
	int experimentState = kRLEpisode;

	assert(theExperimentConnection != 0);

	rlBufferClear(&clientexp_rlbuffer);
	offset = 0;
	offset = rlBufferWrite(&clientexp_rlbuffer, offset, &numSteps, 1, sizeof(int));
	rlSendBufferData(theExperimentConnection, &clientexp_rlbuffer, experimentState);

	rlBufferClear(&clientexp_rlbuffer);
	/*Brian Sept 8 2008 :: Not really sure if I should be resetting offset to 0 here.  Seems to work as is*/
	offset=0;
	rlRecvBufferData(theExperimentConnection, &clientexp_rlbuffer, &experimentState);
	offset = rlBufferRead(&clientexp_rlbuffer, offset, &terminal, 1, sizeof(int));
	assert(experimentState == kRLEpisode);
	return terminal;
}


};

class RLAgentWorkUnit : public WorkUnit
{
public:
    char* kUnknownMessage;
  rlBuffer theBuffer;
  char* theTaskSpec;
  observation_t clientagent_observation;
  char *clientagent_inmessage;
  unsigned int clientagent_inmessagecapacity;

  action_t this_action;
action_t last_action;

observation_t *last_observation;

  virtual ObjectPtr run(ExecutionContext& context)
  {
    char* host = kLocalHost;
	  short port = kDefaultPort;
    
    kUnknownMessage = "Unknown Message: %d\n";

    theTaskSpec = 0;
    allocateRLStruct(&clientagent_observation, 0, 0, 0);
    allocateRLStruct(&this_action, 0, 0, 0);
    allocateRLStruct(&last_action, 0, 0, 0);
    last_observation = new rl_abstract_type_t();
    allocateRLStruct(last_observation, 0, 0, 0);

	rlBufferCreate(&theBuffer, 4096);
  int theConnection;
	theConnection = rlWaitForConnection(host, port, kRetryTimeout);
	rlBufferClear(&theBuffer);
	rlSendBufferData(theConnection, &theBuffer, kAgentConnection);
  context.enterScope("Agent");
  runAgentEventLoop(theConnection);
  context.leaveScope();
	//rlClose(theConnection);

    const char* task_spec;
    
    return new Boolean(true);
  }



  void runAgentEventLoop(int theConnection) {
  int agentState = 0;

  do {
    rlBufferClear(&theBuffer);
    rlRecvBufferData(theConnection, &theBuffer, &agentState);

    switch(agentState) {
    case kAgentInit:
      onAgentInit(theConnection);
      break;

    case kAgentStart:
      onAgentStart(theConnection);
      break;

    case kAgentStep:
      onAgentStep(theConnection);
      break;

    case kAgentEnd:
      onAgentEnd(theConnection);
      break;

    case kAgentCleanup:
      onAgentCleanup(theConnection);
      break;

    case kAgentMessage:
      onAgentMessage(theConnection);
      break;

    case kRLTerm:
      break;
    
    default:
      fprintf(stderr, kUnknownMessage, agentState);
      exit(0);
      break;
    };

    rlSendBufferData(theConnection, &theBuffer, agentState);
  } while (agentState != kRLTerm);
}

void onAgentInit(int theConnection) {
	unsigned int theTaskSpecLength = 0;
	unsigned int offset = 0;

	/* Read the data in the buffer (data from server) */
	offset = rlBufferRead(&theBuffer, offset, &theTaskSpecLength, 1, sizeof(int));
	if (theTaskSpecLength > 0) {
		theTaskSpec = (char*)calloc(theTaskSpecLength+1, sizeof(char));
		offset = rlBufferRead(&theBuffer, offset, theTaskSpec, theTaskSpecLength, sizeof(char));
		/*Make sure the string is null terminated */
		theTaskSpec[theTaskSpecLength]='\0';
	}

	/* Call RL method on the recv'd data */
	agent_init(theTaskSpec);

	/* Prepare the buffer for sending data back to the server */
	rlBufferClear(&theBuffer);
}

void onAgentStart(int theConnection) {
	const action_t *theAction;
	unsigned int offset = 0;

	/* Read the data in the buffer (data from server) */
	offset = rlCopyBufferToADT(&theBuffer, offset, &clientagent_observation);
	__RL_CHECK_STRUCT(&clientagent_observation)

	/* Call RL method on the recv'd data */
	theAction = agent_start(&clientagent_observation);
	__RL_CHECK_STRUCT(theAction)

	/* Prepare the buffer for sending data back to the server */
	rlBufferClear(&theBuffer);
	offset = 0;
	offset = rlCopyADTToBuffer(theAction, &theBuffer, offset);

}

void onAgentStep(int theConnection) {
	double theReward = 0;
	const action_t *theAction;
	unsigned int offset = 0;

	/* Read the data in the buffer (data from server) */
	offset = rlBufferRead(&theBuffer, offset, &theReward, 1, sizeof(theReward));
	offset = rlCopyBufferToADT(&theBuffer, offset, &clientagent_observation);
	__RL_CHECK_STRUCT(&clientagent_observation)

	/* Call RL method on the recv'd data */
	theAction = agent_step(theReward, &clientagent_observation);
	__RL_CHECK_STRUCT(theAction)

	/* Prepare the buffer for sending data back to the server */
	rlBufferClear(&theBuffer);
	offset = 0;

	rlCopyADTToBuffer(theAction, &theBuffer, offset);
}

void onAgentEnd(int theConnection) {
	double theReward = 0;

	/* Read the data in the buffer (data from server) */
	rlBufferRead(&theBuffer, 0, &theReward, 1, sizeof(double));

	/* Call RL method on the recv'd data */
	agent_end(theReward);

	/* Prepare the buffer for sending data back to the server */
	rlBufferClear(&theBuffer);
}

void onAgentCleanup(int theConnection) {
	/* Read the data in the buffer (data from server) */
	/* No data sent for agent cleanup */

	/* Call RL method on the recv'd data */
	agent_cleanup();

	/* Prepare the buffer for sending data back to the server */
	rlBufferClear(&theBuffer);

	/* Cleanup our resources */
	clearRLStruct(&clientagent_observation);
	free(theTaskSpec);
	free(clientagent_inmessage);

	theTaskSpec = 0;
	clientagent_inmessage = 0;
	clientagent_inmessagecapacity = 0;
}


void onAgentMessage(int theConnection) {
	unsigned int inMessageLength = 0;
	unsigned int outMessageLength = 0;
	char* inMessage = 0;
	const char* outMessage = 0;
	unsigned int offset = 0;

	/* Read the data in the buffer (data from server) */
	offset = 0;
	offset = rlBufferRead(&theBuffer, offset, &inMessageLength, 1, sizeof(int));

	if (inMessageLength >= clientagent_inmessagecapacity) {
		inMessage = (char*)calloc(inMessageLength+1, sizeof(char));
		free(clientagent_inmessage);

		clientagent_inmessage = inMessage;
		clientagent_inmessagecapacity = inMessageLength;
	}

	if (inMessageLength > 0) {
		offset = rlBufferRead(&theBuffer, offset, clientagent_inmessage, inMessageLength, sizeof(char));
	}
	/*Make sure to null terminate the string */
	 clientagent_inmessage[inMessageLength]='\0';

	/* Call RL method on the recv'd data */
	outMessage = agent_message(clientagent_inmessage);
	if (outMessage != NULL) {
		outMessageLength = strlen(outMessage);
	}

	/* Prepare the buffer for sending data back to the server */
	/* we want to start sending, so we're going to reset the offset to 0 so we write to the beginning of the buffer */
	rlBufferClear(&theBuffer);
	offset = 0;

	offset = rlBufferWrite(&theBuffer, offset, &outMessageLength, 1, sizeof(int)); 
	if (outMessageLength > 0) {
		offset = rlBufferWrite(&theBuffer, offset, outMessage, outMessageLength, sizeof(char));
	}
}

int randInRange(int max){
	double r, x;
	r = ((double)rand() / ((double)(RAND_MAX)+(double)(1)));
   	x = (r * (max+1));
	return (int)x;
}

void agent_init(const char* task_spec)
{
	/*Seed the random number generator*/
	srand(time(0));
	/*Here is where you might allocate storage for parameters (value function or policy, last action, last observation, etc)*/
	
	/*Here you would parse the task spec if you felt like it*/
	
	/*Allocate memory for a one-dimensional integer action using utility functions from RLStruct_util*/
	allocateRLStruct(&this_action,1,0,0);
	last_observation=allocateRLStructPointer(0,0,0);
	
	/* That is equivalent to:
			 this_action.numInts     =  1;
			 this_action.intArray    = (int*)calloc(1,sizeof(int));
			 this_action.numDoubles  = 0;
			 this_action.doubleArray = 0;
			 this_action.numChars    = 0;
			 this_action.charArray   = 0;
	*/
}

const action_t *agent_start(const observation_t *this_observation) {
	/* This agent always returns a random number, either 0 or 1 for its action */
	int theIntAction=randInRange(1);
	this_action.intArray[0]=theIntAction;

	/* In a real action you might want to store the last observation and last action*/
	replaceRLStruct(&this_action, &last_action);
	replaceRLStruct(this_observation, last_observation);
	
	return &this_action;
}

const action_t *agent_step(double reward, const observation_t *this_observation) {
	/* This agent always returns a random number, either 0 or 1 for its action */
	int theIntAction=randInRange(1);
	this_action.intArray[0]=theIntAction;
	
	
	/* In a real action you might want to store the last observation and last action*/
	replaceRLStruct(&this_action, &last_action);
	replaceRLStruct(this_observation, last_observation);
	
	return &this_action;
}

void agent_end(double reward) {
	clearRLStruct(&last_action);
	clearRLStruct(last_observation);
}

void agent_cleanup() {
	clearRLStruct(&this_action);
	clearRLStruct(&last_action);
	freeRLStructPointer(last_observation);
}

const char* agent_message(const char* inMessage) {
	if(strcmp(inMessage,"what is your name?")==0)
		return "my name is skeleton_agent!";

	return "I don't know how to respond to your message";
}

};

class RewardLearningTest : public WorkUnit
{
public:
  RewardLearningTest() : numSamples(25), numInitialSamples(10), randomSeed(0), learningRate(0.1), learningRateDecay(0.005) {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    
    context.getRandomGenerator()->setSeed(randomSeed);
  
    CompositeWorkUnitPtr agentAndExperiment = new CompositeWorkUnit("Agent and Experiment", 2);
    agentAndExperiment->setWorkUnit(0, new RLAgentWorkUnit());
    agentAndExperiment->setWorkUnit(1, new RLExperimentWorkUnit());
    context.run(agentAndExperiment);
    return new Boolean(true);
  }
  
protected:
  friend class RewardLearningTestClass;
  
  size_t numSamples;
  size_t numInitialSamples;
  int randomSeed;
  double learningRate;
  double learningRateDecay;

};

}; /* namespace lbcpp */

#endif // MOO_PERCEPTRON_TEST_H_
