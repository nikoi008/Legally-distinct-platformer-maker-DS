#ifndef SOURCE_LEVEL_SHARING_H
#define SOURCE_LEVEL_SHARING_H
#include <stdbool.h>
#include <nf_lib.h>
#include <nds.h>
#include <dswifi9.h>
#include "defines.h"
#include "types.h"
#include "globals.h"

typedef struct __attribute__((packed))
{
    u16 x;
    u16 y;
    u8 blockID;
} tilePacket;

#define BATCH_SIZE 8
#define MAX_CLIENTS 1
#define AIR 0
#define END 9999

typedef struct __attribute__((packed))
{
    tilePacket tiles[BATCH_SIZE];
    u8 count;
    u8 isEnd;
    u16 batchNum;
} tileBatchPacket;

typedef struct __attribute__((packed))
{
    u16 batchNum;
} batchAckPacket;

void sendTiles(tileBatchPacket *batch);

void FromClientPacketHandler(Wifi_MPPacketType type, int aid, int base, int len);

void FromHostPacketHandler(Wifi_MPPacketType type, int base, int len);

void hostMode();

bool AccessPointSelectionMenu();

void ClientMode(gameContext *ctx);

#endif //SOURCE_LEVEL_SHARING_H