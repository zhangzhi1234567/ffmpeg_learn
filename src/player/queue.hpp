//
// Created by zz on 2021/3/25.
//

#ifndef BUILD_FFMPEG_LEARN_QUEUE_HPP
#define BUILD_FFMPEG_LEARN_QUEUE_HPP

#include <SDL/SDL.h>
#include <libavformat/avformat.h>
#include <libavcodec/packet.h>
typedef AVPacketList QueueNode;

struct PacketQueue_t {
    QueueNode *first_node, *last_node; //AVPacketList实际就是node, 起名有点奇怪
    int packer_num;
    int total_size;
    SDL_mutex *mutex;
    SDL_cond *cond;//条件变量 上锁后用，当资源不能满足时等待，等待时，会先解锁，然后线程休眠，被唤醒后，继续加锁获取资源。
};



void packet_queue_init(PacketQueue_t *queue) {
    memset(queue, 0, sizeof(PacketQueue_t));
    queue->mutex = SDL_CreateMutex();
    queue->cond = SDL_CreateCond();
}

int packet_queue_put(PacketQueue_t *queue, AVPacket *pkt) {
    if (NULL == queue || NULL == pkt) return -1;
    if (av_dup_packet(pkt) < 0) return -2;

    QueueNode *node = static_cast<QueueNode *>(av_malloc(sizeof(QueueNode)));
    if (!node) return -3;

    node->pkt = *pkt;
    node->next = NULL;//准备好node， 插入队列

    SDL_LockMutex(queue->mutex);

    if (queue->first_node == NULL) { //头为空时，给头部和尾部都赋值，头不为空时，直接在尾部插入节点，并且尾部节点指向当前插入的节点
        queue->first_node = node;
    }else {
        queue->last_node->next = node;
    }
    queue->last_node = node;

    queue->packer_num++;
    queue->total_size += node->pkt.size;

    SDL_CondSignal(queue->cond);
    SDL_UnlockMutex(queue->mutex);

    return 0;
}

/// @brief
/// @param queue
/// @param block 0 非阻塞，当队列无资源时，直接返回。1 阻塞，等队列无资源等，等待队列中的资源
/// @return

int packet_queue_get(PacketQueue_t *queue, AVPacket *pkt, int block) {
    int ret = 0;
    SDL_LockMutex(queue->mutex);
    while(1) {
        QueueNode *node = queue->first_node;
        if (node) {
            memcpy(pkt, &node->pkt, sizeof(AVPacket));
            queue->first_node = queue->first_node->next;
            if (queue->first_node == NULL) {
                queue->last_node = NULL;
            }
            queue->total_size -= pkt->size;
            queue->packer_num--;
            av_free(node);
            break;
        }else {
            if (block) {
                SDL_CondWait(queue->cond, queue->mutex);
            }else {
                break;
            }
        }
    }

    SDL_UnlockMutex(queue->mutex);
    return ret;
}

#endif //BUILD_FFMPEG_LEARN_QUEUE_HPP
















