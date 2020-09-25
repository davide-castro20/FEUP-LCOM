#include <stdio.h> 
#include <stdlib.h> 
#include "queue.h"

// A utility function to create a new linked list node. 
QNode* newNode(uint8_t k) 
{ 
    QNode* temp = (QNode*)malloc(sizeof(QNode)); 
    temp->key = k; 
    temp->next = NULL; 
    return temp; 
} 
  
// A utility function to create an empty queue 
Queue* createQueue() 
{ 
    Queue* q = (Queue*)malloc(sizeof(Queue)); 
    q->front = q->rear = NULL; 
    return q; 
} 
  
// The function to add a key k to q 
void enQueue(Queue* q, uint8_t k) 
{ 
    // Create a new LL node 
    QNode* temp = newNode(k); 
  
    // If queue is empty, then new node is front and rear both 
    if (q->rear == NULL) { 
        q->front = q->rear = temp; 
        return; 
    } 
  
    // Add the new node at the end of queue and change rear 
    q->rear->next = temp; 
    q->rear = temp; 
} 

// Function to remove a key from given queue q 
uint8_t deQueue(Queue* q) 
{ 
    // If queue is empty, return NULL. 
    if (q->front == NULL) 
        return 'N'; 
  
    // Store previous front and move front one node ahead 
    QNode* temp = q->front; 
    //free(temp); 
  
    q->front = q->front->next; 
  
    // If front becomes NULL, then change rear also as NULL 
    if (q->front == NULL) 
        q->rear = NULL; 
    return temp->key; 
} 

bool queue_empty(Queue* q)
{
    if(q->front == NULL && q->rear == NULL)
    {
        return true;
    }
    return false;
}
