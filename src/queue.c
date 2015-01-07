/* 
 * Copyright (c) 1999 Network Appliance, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */ 

#include <malloc.h>
#include "main.h"
#include "queue.h"
#include "ndmp_common.h"




/* enqueue():
 *   Description:
 *      Queues up particular NDMP client requests for processing at
 *      the top level.  If head is NULL the list is assumed to be
 *      empty and head is changed to point to the new data element.
 *      
 *   Inputs:
 *      MsgQueue *head  - head of the message queue to put the message on
 *      MsgData  *data  - message data to put on the queue.
 *
 *   Outputs:
 *      MsgQueue *head  - set to the head of a new queue if the queue was
 *                        previously empty (head == NULL).
 *
 *   Returns:
 *      void
 */


void
enqueue(MsgQueue *head, MsgData *data)
{

    MsgData *newdata;

    newdata = (MsgData *) malloc(sizeof(MsgData));
    if (newdata == NULL) {
	fprintf(stderr, "\n\nCould not allocate memory for NDMP message.\n"
			"message = %d, reason = %d.\n",
			data->message, data->reason);
	exit (1);
    }
    newdata->message = data->message;
    newdata->reason = data->reason;
    strcpy(newdata->text, data->text);
    newdata->connection = data->connection;
    newdata->next = NULL;

    if (*head == NULL) {
	*head = newdata;
    }
    else
    {
	newdata->next = *head;
	*head = newdata;
    }
    return;
}



/* dequeue():
 *   Description:
 *      Removes a data element from the queue.
 *      
 *   Inputs:
 *      MsgQueue *head  - head of the message queue to take the message from
 *
 *   Outputs:
 *      MsgData  *data  - data element to store information in
 *
 *   Returns:
 *      number of messages that were in the queue BEFORE this one was removed.
 */

int
dequeue(MsgQueue *head, MsgData *data)
{
    int element_count = 0;
    MsgData *cur_data;

    
    for (cur_data = *head;
	 cur_data && cur_data->next && cur_data->next->next;
	 cur_data = cur_data->next, element_count++);


    /* this only occurs when there are no elements in the list */
    if (cur_data == NULL)
	return 0;
    
    /* This only occurs if it's the only element in the list */
    if (cur_data->next == NULL) {
	memcpy(data, cur_data, sizeof(MsgData));
	*head = NULL;
	free(cur_data);
	return 1;
    }

    /* if there are two or more elements, cur_data points to the
     * second to last element.
     */
    if (cur_data->next->next == NULL) {
	memcpy(data, cur_data->next, sizeof(MsgData));
	free(cur_data->next);
	cur_data->next = NULL;
	return element_count+1;
    }

    /* this should never happen */
    return -1;
}


/* peek():
 *   Description:
 *      Same as dequeue but the message is left in the queue.  This is
 *      useful for determing if there are messages waiting to be processed
 *      and how many.
 *      
 *   Inputs:
 *      MsgQueue *head  - head of the message queue to take the message from
 *
 *   Outputs:
 *      MsgData  *data  - data element with head of the message queue
 *
 *   Returns:
 *      number of messages that were in the queue BEFORE this one was removed.
 */

int peek(MsgQueue *head, MsgData *data)
{
    MsgData *cur_data;
    int element_count = 0;

    for (cur_data = *head;
	 cur_data && cur_data->next; 
	 cur_data = cur_data->next, element_count++);

    if (cur_data == NULL)
	return 0;

    if (cur_data ->next == NULL) {
	memcpy(data, cur_data->next, sizeof(MsgData));
	return element_count;
    }

    /* this should never happen */
    return -1;
}




