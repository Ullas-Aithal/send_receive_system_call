#include "mod_msg_passing_library.h"
#include "pm.h"
#include "mproc.h"
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#define MAX_NO_OF_RECEIVERS 10
#define MAX_NO_OF_GROUPS 10
#define MAX_MQ_GRP_CNT 10
#define MAX_MSG_CNT 10
#define MAX_REC_CNT 10

#define MARKED 1
#define UNMARKED 0

#define MARK 1
#define UNMARK 0

#define SUCCESS 1
#define FAIL 0


static STR_GROUPS Stat_groups;

 STR_MESSAGE* getMessage(int sender, int message_value) {

	 STR_MESSAGE* msg;
	 msg = (STR_MESSAGE*)calloc(1, sizeof(STR_MESSAGE));
	 msg->mMessage = message_value;
	 msg->next = NULL;
	 msg->mSender = sender;
	 msg->mReceivers = (int*)calloc(MAX_REC_CNT + 1, sizeof(int));

	 return msg;

 }
 STR_RECOVERRECEIVER* getRecoverReciver(int receiverId) {

	STR_RECOVERRECEIVER* newRecoverRec;

	newRecoverRec = (STR_RECOVERRECEIVER*)calloc(1, sizeof(STR_RECOVERRECEIVER));
	newRecoverRec->mReceiverId = receiverId;
	return newRecoverRec;
    	
}
void resetGroupContent(int groupId) 
{

	int i = groupId;

	Stat_groups.mMq_group[i].mMessageQueueHead = NULL;
	Stat_groups.mMq_group[i].mMQReceivers = NULL;
	Stat_groups.mMq_group[i].mMaxMessageReceivers = MAX_REC_CNT + 1;
	Stat_groups.mMq_group[i].mCurrentMesQueuCount = 0;
	Stat_groups.mMq_group[i].mReceiversCount = 0;
	Stat_groups.mMq_group[i].mRecvRecoverHead = NULL;
	

}
int initializeGroups() {

	Stat_groups.mGroup_hash = (int*)calloc(MAX_MQ_GRP_CNT + 1, sizeof(int));
	Stat_groups.mGroupCount = 0;
	Stat_groups.mMaxNoOfGroups = MAX_MQ_GRP_CNT;
	Stat_groups.mMq_group = (STR_MQ_GROUP*)malloc(sizeof(STR_MQ_GROUP)*(MAX_MQ_GRP_CNT + 1));
	for (int i = 1; i < MAX_MQ_GRP_CNT + 1; i++) {
		
		resetGroupContent(i);
		
	}

	

	return 1;

}

void cleanMessage(STR_MESSAGE *msgPtr) {

	


	if (msgPtr != NULL) {

	
		if (msgPtr->mReceivers != NULL)
			if(msgPtr->mReceivers != NULL)
			free(msgPtr->mReceivers);
			msgPtr->mReceivers = NULL;


	}

}

void cleanMQ_group(STR_MQ_GROUP* ptr) {

	STR_MESSAGE *temp_msg_ptr= NULL;
	STR_RECOVERRECEIVER *temp_RecvRecoverHead_ptr = NULL;
	STR_RECOVERRECEIVER *recvRecoverHead_ptr = NULL;

	if (ptr->mMessageQueueHead != NULL) {

		STR_MESSAGE *msg_ptr = NULL;
		msg_ptr = ptr->mMessageQueueHead;
		while (msg_ptr != NULL) {
			cleanMessage(msg_ptr);

			
			temp_msg_ptr = msg_ptr;
			msg_ptr = msg_ptr->next;
			if(temp_msg_ptr!=NULL)
			free(temp_msg_ptr);		
			temp_msg_ptr = NULL;
		}

		recvRecoverHead_ptr = ptr->mRecvRecoverHead;
		while (recvRecoverHead_ptr != NULL) {

			temp_RecvRecoverHead_ptr = recvRecoverHead_ptr;
			recvRecoverHead_ptr = recvRecoverHead_ptr->mNext;
			if(temp_RecvRecoverHead_ptr != NULL)
			free(temp_RecvRecoverHead_ptr);
			temp_RecvRecoverHead_ptr = NULL;
		}
		
	}

}

void clean_STR_GROUPS() {

	if (Stat_groups.mMq_group != NULL) {

		STR_MQ_GROUP *ittr = Stat_groups.mMq_group;
		for (int i = 1; i < MAX_MQ_GRP_CNT + 1; i++) {

			cleanMQ_group(&Stat_groups.mMq_group[i]);
			resetGroupContent(i);
		}

		if(Stat_groups.mMq_group != NULL)
		free(Stat_groups.mMq_group);
		Stat_groups.mMq_group = NULL;
		
		if(Stat_groups.mGroup_hash != NULL)
		free(Stat_groups.mGroup_hash);
		Stat_groups.mGroup_hash = NULL;
		
	}

}


int openGroupBySender(int groupId, int senderId,STR_MESSAGE* message) {

	STR_MESSAGE* ittr = NULL;
	int status = 0;
	if (groupId > MAX_MQ_GRP_CNT || groupId < 1)
	{
		printf("\nMaximum Group number should be between 1-%d !\n", MAX_MQ_GRP_CNT);
		return status;
	}

	if (Stat_groups.mGroup_hash == NULL) {
		status = initializeGroups();
	}

	

	if (Stat_groups.mGroup_hash[groupId] != MARKED)
	{
		printf("\nGroup not found with gid %d. Creating new group\n", groupId);
		Stat_groups.mMq_group[groupId].mMQReceivers = (int*)calloc(MAX_REC_CNT + 1, sizeof(int));
		Stat_groups.mGroup_hash[groupId] = MARK;
		Stat_groups.mGroupCount++;
		status = 1;
		printf("\nGroup Created");
	}
	else {
		printf("\nGroup already present. Adding new message from sender %d \n",senderId);
	}

	message->mReceiverCount = 0;

	for (int i = 1; i < Stat_groups.mMq_group[groupId].mMaxMessageReceivers + 1; i++) {

		message->mReceivers[i] = Stat_groups.mMq_group[groupId].mMQReceivers[i];
		if (message->mReceivers[i] == MARKED) {
			message->mReceiverCount++;
		}
	}


	if (Stat_groups.mMq_group[groupId].mMessageQueueHead == NULL) {

		Stat_groups.mMq_group[groupId].mMessageQueueHead = message;

	}
	else {

		ittr = Stat_groups.mMq_group[groupId].mMessageQueueHead;
		while (ittr->next != NULL) {
			ittr = ittr->next;
		}
		ittr->next = message;
		printf("\nMessage sent to group %d" , groupId);
	}


	return status;


}


int do_mq_send()
{
	
	STR_MESSAGE* msg = getMessage(m_in.m2_i2,m_in.m2_i1);
	 STR_MESSAGE* ittr = NULL;
	 int groupId = m_in.m2_i3;
	 

	if (Stat_groups.mMq_group == NULL) {
		printf("No group created yet. Creating new Group \n");
		return openGroupBySender(groupId,msg->mSender,msg);
	}

	if (Stat_groups.mGroup_hash[groupId] == MARKED) {

		msg->mReceiverCount = 0;

		for (int i = 1; i < Stat_groups.mMq_group[groupId].mMaxMessageReceivers+1; i++) {

			msg->mReceivers[i] = Stat_groups.mMq_group[groupId].mMQReceivers[i];
			if (msg->mReceivers[i] == MARKED) {
				
				msg->mReceiverCount++;
			}
		}


		if (Stat_groups.mMq_group[groupId].mMessageQueueHead == NULL) {

			Stat_groups.mMq_group[groupId].mMessageQueueHead = msg;

		}
		else {

			ittr = Stat_groups.mMq_group[groupId].mMessageQueueHead;
			while (ittr->next != NULL) {
				ittr = ittr->next;
			}
			ittr->next = msg;

		}
		Stat_groups.mMq_group[groupId].mCurrentMesQueuCount++;
		printf("\nMessage %d sent to group %d from sender %d\n" , msg->mMessage,groupId,msg->mSender);
		return 1;
	}
	else {

		
		return openGroupBySender(groupId,msg->mSender,msg);
	} 
	
}

int do_mq_receive()
{
	
	int groupId = m_in.m3_i1;
	int receiverId = m_in.m3_i2;
	
	STR_MESSAGE* ittr = NULL;
	STR_MESSAGE* preMsgIttr = NULL;

	if (Stat_groups.mMq_group == NULL) {
		printf("No group %d found. Exiting.. \n",groupId);
		return 0;
		
	}

	

	if (Stat_groups.mGroup_hash[groupId] == MARKED) {
		
		if(Stat_groups.mMq_group[groupId].mMQReceivers[receiverId] == UNMARKED)
		{
			printf("\nReceiver %d not part of the group %d. Register the receiver by calling opnGroup",receiverId,groupId );
			return 0;
		}

		if (Stat_groups.mMq_group[groupId].mMessageQueueHead == NULL) {
		
			
			

			printf("No messages in the group\n");
			STR_RECOVERRECEIVER *recHeadIttr = Stat_groups.mMq_group[groupId].mRecvRecoverHead;
			STR_RECOVERRECEIVER *prevRecHeadIttr = NULL;
			prevRecHeadIttr = recHeadIttr;
			if (recHeadIttr != NULL) {
				while (recHeadIttr != NULL) {

					if (recHeadIttr->mReceiverId == receiverId)
					{
						printf("Recoving %d recv from livelock\n", recHeadIttr->mReceiverId);
						
						if (Stat_groups.mMq_group[groupId].mRecvRecoverHead == recHeadIttr) {
							Stat_groups.mMq_group[groupId].mRecvRecoverHead = NULL;
						}
						else {
							prevRecHeadIttr->mNext = recHeadIttr->mNext;
						}
						if(recHeadIttr != NULL)
						free(recHeadIttr);
						recHeadIttr = NULL;
						prevRecHeadIttr = NULL;
						return 1;

					}
					prevRecHeadIttr = recHeadIttr;
					recHeadIttr = recHeadIttr->mNext;
				}
			} 
			
			return 2;

		}
		else {

			ittr = Stat_groups.mMq_group[groupId].mMessageQueueHead;
			while (ittr != NULL) {
					
					
				if (ittr->mReceivers[receiverId] == MARKED) {

					printf("\n\nMessage for receiverId %d from sender %d. Message: %d\n", receiverId, ittr->mSender, ittr->mMessage);
					ittr->mReceivers[receiverId] = UNMARK;
					ittr->mReceiverCount--;
					if (ittr->mReceiverCount == 0) {

						if (Stat_groups.mMq_group[groupId].mMessageQueueHead == ittr) {
							Stat_groups.mMq_group[groupId].mMessageQueueHead = ittr->next;
						}
						else {
							preMsgIttr->next = ittr->next;    // removing current message
						}
						if(ittr != NULL)
						free(ittr);
						ittr = NULL;
						Stat_groups.mMq_group[groupId].mCurrentMesQueuCount--;
					}
					return 1;

				}
				

				preMsgIttr = ittr;
				ittr = ittr->next;
			} // while end
			
			return 0;
		}

	}
	else 
	{

		printf("Group does not exist\n");
		return 0;
	}
}

int do_mq_open()
{
	//initializeGroups();
	
	int groupId = m_in.m1_i1;
	int receiverId = m_in.m1_i2;
	
	
	
	int status = 0;
	if (groupId > MAX_MQ_GRP_CNT || groupId < 1)
	{
		printf("\nMaximum Group number should be between 1-%d !\n", MAX_MQ_GRP_CNT);
		return status;
	}

	if (receiverId > MAX_REC_CNT || receiverId < 1)
	{
		printf("\nMaximum receiver number should be between 1-%d !\n", MAX_REC_CNT);
		return status;
	}

	

	if (Stat_groups.mGroup_hash == NULL) {
		status = initializeGroups();
	}
	
	


	

	if (Stat_groups.mGroup_hash[groupId] != MARKED)
	{
		printf("\nGroup not found with gid %d. Creating new group\n", groupId);
		Stat_groups.mMq_group[groupId].mMQReceivers = (int*)calloc(MAX_REC_CNT + 1, sizeof(int));

		
		
		Stat_groups.mGroup_hash[groupId] = MARK;

		Stat_groups.mGroupCount++;
		status = 1;

		printf("\nReceiver %d added to group %d\n",receiverId,groupId);
		
		
		

	}
	else {

		printf("\nGroup already present adding new receiver %d\n", receiverId);

		

	}
	if(Stat_groups.mMq_group[groupId].mMQReceivers[receiverId] == MARKED)
	{
		printf("\nReceiver %d already registered with group id %d!\n", receiverId,groupId);
		return status;		
	}

	Stat_groups.mMq_group[groupId].mMQReceivers[receiverId] = MARK;
	Stat_groups.mMq_group[groupId].mReceiversCount++;


	return status;
	
	
}
int do_mq_close()
{
	int groupId = m_in.m1_i1;
	int requesterId = m_in.m1_i2;
	
	int status = FAIL; 
	if (Stat_groups.mMq_group == NULL) {
		printf("\n No Groups exists\n");
		return status;
	}

	if (groupId > MAX_MQ_GRP_CNT || groupId < 1)
	{
		printf("\nMaximum Group number should be between 1-%d !\n", MAX_MQ_GRP_CNT);
		return status;
	}

	if (requesterId > MAX_REC_CNT || requesterId < 1)
	{
		printf("\nMaximum receiver number should be between 1-%d !\n", MAX_REC_CNT);
		return status;
	}
	if (Stat_groups.mMq_group[groupId].mMQReceivers[requesterId] != MARKED)
	{
		printf("\nReceiver %d doesn't have permission to close the group %d. Please check the permissions",requesterId,groupId);
		return FAIL;
	}


	if (Stat_groups.mGroup_hash[groupId] == MARKED)
	{
		printf("\nRemoving groupId %d.\n", groupId);
		if (Stat_groups.mMq_group[groupId].mCurrentMesQueuCount == 0) {

			STR_MQ_GROUP* temp = &(Stat_groups.mMq_group[groupId]);
		
			cleanMQ_group(temp);
			Stat_groups.mGroup_hash[groupId] = UNMARK;
			resetGroupContent(groupId);
			printf("\nRemoved groupId %d.\n", groupId);
			return SUCCESS;

		}
		else {
			printf("\nCannot remove groupId %d. %d message(s) stil pending.\n", groupId, Stat_groups.mMq_group[groupId].mCurrentMesQueuCount);
			return FAIL;
		}

	}
	else
	{
			printf("\nGroup ID %d not found.\n", groupId);
			return FAIL;
	}

	return status;
}
int do_mq_recover() {

	int groupId =  m_in.m3_i1;
	int receiverId =  m_in.m3_i2;
	int status = 0;

	if (Stat_groups.mMq_group == NULL) {
		printf("\nNo gropus at all. Should not be here !!\n");
		return 0;
	}

	if (groupId > MAX_MQ_GRP_CNT || groupId < 1)
	{
		printf("\nMaximum Group number should be between 1-%d !\n", MAX_MQ_GRP_CNT);
		return status;
	}


	if (receiverId > MAX_REC_CNT || receiverId < 1)
	{
		printf("\nMaximum receiver number should be between 1-%d !\n", MAX_REC_CNT);
		return status;
	}



	if (Stat_groups.mGroup_hash[groupId] == MARKED) {

		//printf(" %s", Stat_groups.mMq_group[groupId].mMessageQueueHead == NULL ? "NULL" : "NOT NULL");
		if (Stat_groups.mMq_group[groupId].mMessageQueueHead == NULL) {  // if there are no messages then only try to recover else it will rec will recover by reading message

			printf("Nothing to receive so recover the receiver %d \n",receiverId);
			STR_RECOVERRECEIVER *recoverHeadIttr = NULL;
			STR_RECOVERRECEIVER *newRecovRec = NULL;
			newRecovRec = getRecoverReciver(receiverId);
			if (newRecovRec == NULL) {   // error check
				printf("\n MI should not be here !\n");
				status = 0;
				return status;
			}

			recoverHeadIttr = Stat_groups.mMq_group[groupId].mRecvRecoverHead;
			if (recoverHeadIttr == NULL) {
				 
				
				Stat_groups.mMq_group[groupId].mRecvRecoverHead = newRecovRec;

			}
			else {

				while (recoverHeadIttr->mNext != NULL) {

					recoverHeadIttr = recoverHeadIttr->mNext;

				}

				
				recoverHeadIttr->mNext = newRecovRec;

			}

		}
		status = 1;

	}
	else {

		printf("without group a receiver should not be waiting : shouldnt be here !!\n\n");
		status = 0;
		
	}

	return status;

}
int do_mq_clean()
{
	clean_STR_GROUPS();
	return 1;
}

