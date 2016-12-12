#include <lib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct str_message_def {

	int mMessage;
	int mSender;
	int *mReceivers;
	int mReceiverCount;
	struct str_message_def *next;

}STR_MESSAGE;
typedef struct str_recoverReceiver {

	int mReceiverId;
	struct str_recoverReceiver *mNext;
	

}STR_RECOVERRECEIVER;

typedef struct str_messageQueue_group {

	STR_MESSAGE *mMessageQueueHead;
	int *mMQReceivers;
	int mMaxMessageReceivers;
	int mCurrentMesQueuCount;
	int mMaxMessageCount;
	int mReceiversCount;
	STR_RECOVERRECEIVER *mRecvRecoverHead;
	

}STR_MQ_GROUP;

typedef struct str_groups {

	STR_MQ_GROUP *mMq_group;
	int mGroupCount;
	int mMaxNoOfGroups;
	int *mGroup_hash;


}STR_GROUPS;

int openGroup(int groupId, int receiverId) {
	
	message m;
	m.m1_i1 = groupId;
	m.m1_i2 = receiverId;
	
	return(_syscall(PM_PROC_NR, 35, &m));
}

 int closeGroup(int groupId, int requesterId) {
	
	message m;
	m.m1_i1 = groupId;
	m.m1_i2 = requesterId;
	
	return(_syscall(PM_PROC_NR, 44, &m));
} 



int msend(int groupId, int sender, int message_value) {
	
	message m;
	m.m2_i1 = message_value;
	m.m2_i2 = sender;
	m.m2_i3 = groupId;
	
	
	return(_syscall(PM_PROC_NR, 69, &m));
	
}

int mreceive(int groupId, int receiverId)
{
	message m;
	int status = 1;
	int wait_time = 3;
	int i = 0;
	int wait_time_seconds = 5;
	m.m3_i1 = groupId;
	m.m3_i2 = receiverId;
	
	status = _syscall(PM_PROC_NR, 70, &m);
	
	if(status == 2)
	{
		
			i= 0;
			while(1)
			{	i++;
				//printf("\ni %d", i);
				m.m3_i1 = groupId;
				m.m3_i2 = receiverId;		
				printf("\nNo message in the group. Sleeping for %d seconds", wait_time_seconds);
				printf("\n");
				//fflush(stdout);
				sleep(wait_time_seconds);
				status = _syscall(PM_PROC_NR, 70, &m);
				
				
				if(i >= wait_time)
				{
					//printf("\ni inside loop %d", i);
					m.m3_i1 = groupId;
					m.m3_i2 = receiverId;
					status = _syscall(PM_PROC_NR, 49, &m);
					m.m3_i1 = groupId;
					m.m3_i2 = receiverId;
					status = _syscall(PM_PROC_NR, 70, &m);
					//printf("\nStatus inside %d", status);
					i=0;
				}
				if(status != 2) 
				{
					return 1;
				}
			//	printf("\nStatus outside %d", status);
				
			}
			
		
		
		//Call recover system call
		
		
		//Call receive system call one last time
		
		
		
		
		return 0;
	}
	else
	{
		return 1;
	}
	
	
}
int recoverReceiver(int groupId, int receiverId) {
			message m;
			m.m3_i1 = groupId;
			m.m3_i2 = receiverId;
			return(_syscall(PM_PROC_NR, 49,&m));
}
int cleanAll()
{
	message m;
	return(_syscall(PM_PROC_NR, 56,&m));
}