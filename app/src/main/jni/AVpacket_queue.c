//
// Created by saisai on 2018/4/12 0012.
//
#include <pthread.h>
#import "AVpacket_queue.h"
#include "include/libavcodec/avcodec.h"

AVPacketQueue* queue_init(int size){
    AVPacketQueue* queue=(AVPacketQueue*)malloc(sizeof(AVPacketQueue));
    queue->size=size;
    queue->next_to_read=0;
    queue->next_to_write=0;
    int i=0;
    queue->packets=malloc(sizeof(*queue->packets)*size);
    for(i=0;i<size;i++){
        queue->packets[i]=malloc(sizeof(AVPacket));
    }
    return queue;
}

int queue_next(AVPacketQueue* queue,int current){
    return (current+1)%queue->size;
}

void* queue_push(AVPacketQueue* queue,pthread_mutex_t* mutex,pthread_cond_t* cond){
    int current=queue->next_to_write;
    int next_to_write;
    for(;;){
        next_to_write=queue_next(queue,current);
        //写的不等于读的，跳出循环
        if(next_to_write!=queue->next_to_read){
            break;
        }

        pthread_cond_wait(cond,mutex);
    }
    queue->next_to_write=next_to_write;
    pthread_cond_broadcast(cond);
    return queue->packets[current];
}

void* queue_pop(AVPacketQueue* queue,pthread_mutex_t* mutex,pthread_cond_t* cond){
    int current=queue->next_to_read;
    for(;;){
        //写的不等于读的跳出循环
        if(queue->next_to_write!=queue->next_to_read){
            break;
        }
        pthread_cond_wait(cond,mutex);
    }

    queue->next_to_read=queue_next(queue,current);
    pthread_cond_broadcast(cond);
    return queue->packets[current];
}

void queue_free(AVPacketQueue* queue){
    int i;
    for(i=0;i<queue->size;i++){
        free(queue->packets[i]);
    }
    free(queue->packets);
    free(queue);
}
