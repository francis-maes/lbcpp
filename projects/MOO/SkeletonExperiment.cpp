/* 
* Copyright (C) 2008, Brian Tanner

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include <stdio.h>	/* for printf */
#include <rlglue/RL_glue.h> /* RL_ function prototypes and RL-Glue types */
	
int whichEpisode=0;

/* Run One Episode of length maximum cutOff*/
void runEpisode(int stepLimit) {        
    int terminal=RL_episode(stepLimit);
	printf("Episode %d\t %d steps \t%f total reward\t %d natural end \n",whichEpisode,RL_num_steps(),RL_return(), terminal);
	whichEpisode++;
}

//int main(int argc, char *argv[]) {
//	const char* task_spec;
//	const char* responsemessage;
//	const reward_observation_action_terminal_t *stepresponse;
//	const observation_action_t *startresponse;
//
//	printf("\n\nexperiment starting up!\n");
//
//
//	task_spec=rl_init();
//	printf("rl_init called, the environment sent task spec: %s\n",task_spec);
//
//	printf("\n\n----------sending some sample messages----------\n");
//	/*talk to the agent and environment a bit...*/
//	responsemessage=rl_agent_message("what is your name?");
//	printf("agent responded to \"what is your name?\" with: %s\n",responsemessage);
//	responsemessage=rl_agent_message("if at first you don't succeed; call it version 1.0");
//	printf("agent responded to \"if at first you don't succeed; call it version 1.0\" with: %s\n\n",responsemessage);
//
//	responsemessage=rl_env_message("what is your name?");
//	printf("environment responded to \"what is your name?\" with: %s\n",responsemessage);
//	responsemessage=rl_env_message("if at first you don't succeed; call it version 1.0");
//	printf("environment responded to \"if at first you don't succeed; call it version 1.0\" with: %s\n",responsemessage);
//
//	printf("\n\n----------running a few episodes----------\n");
//	runepisode(100);
//	runepisode(100);
//	runepisode(100);
//	runepisode(100);
//	runepisode(100);
//	runepisode(1);
//	/* remember that steplimit of 0 means there is no limit at all!*/
//	runepisode(0);
//	rl_cleanup();
//
//	printf("\n\n----------stepping through an episode----------\n");
//	/*we could also start over and do another experiment */
//	task_spec=rl_init();
//
//	/*we could run one step at a time instead of one episode at a time */
//	/*start the episode */
//	startresponse=rl_start();
//	printf("first observation and action were: %d %d\n",startresponse->observation->intarray[0],startresponse->action->intarray[0]);
//
//	/*run one step */
//	stepresponse=rl_step();
//	
//	/*run until the episode ends*/
//	while(stepresponse->terminal!=1){
//		stepresponse=rl_step();
//		if(stepresponse->terminal!=1){
//			/*could optionally print state,action pairs */
//			/*printf("(%d,%d) ",stepresponse.o.intarray[0],stepresponse.a.intarray[0]);*/
//		}
//	}
//	
//	printf("\n\n----------summary----------\n");
//	
//
//	printf("it ran for %d steps, total reward was: %f\n",rl_num_steps(),rl_return());
//	rl_cleanup();
//
//
//	return 0;
//}
