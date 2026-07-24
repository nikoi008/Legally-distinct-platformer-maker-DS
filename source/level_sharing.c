#include "level_sharing.h"

Wifi_AccessPoint AccessPoint;
volatile int ackBatch = -1;
volatile int pendingBatchAck = -1;
volatile int pendingBatchCount = 0;
volatile bool pendingBatchEnd = false;
bool levelDone = false;

void sendTiles(tileBatchPacket *batch)
{
    Wifi_MultiplayerHostCmdTxFrame(batch, sizeof(tileBatchPacket));
}

void FromClientPacketHandler(Wifi_MPPacketType type, int aid, int base, int len)
{
    if(len < sizeof(batchAckPacket))
        return;

    if(type != WIFI_MPTYPE_REPLY)
        return;

    batchAckPacket ack;
    Wifi_RxRawReadPacket(base, sizeof(ack), (void *)&ack);
    ackBatch = ack.batchNum;
}

void FromHostPacketHandler(Wifi_MPPacketType type, int base, int len)
{
    if(len < sizeof(tileBatchPacket))
        return;
    if(type != WIFI_MPTYPE_CMD)
        return;

    tileBatchPacket batch;
    Wifi_RxRawReadPacket(base, sizeof(batch), (void *)&batch);

    pendingBatchAck = batch.batchNum;
    pendingBatchCount = batch.count;
    pendingBatchEnd = batch.isEnd;

    if(!batch.isEnd)
    {
        for(int i = 0; i < batch.count; i++)
        {
            TILE_MAP[batch.tiles[i].y][batch.tiles[i].x] = batch.tiles[i].blockID;
        }
    }
}

void hostMode()
{
    char debugBuf[64];

    NF_ClearTextLayer(0, 0);
    NF_WriteText(0, 0, 1, 0, "starting");
    NF_UpdateTextLayers();

    Wifi_MultiplayerHostMode(MAX_CLIENTS, sizeof(tileBatchPacket), sizeof(batchAckPacket));
    Wifi_MultiplayerFromClientSetPacketHandler(FromClientPacketHandler);

    while(!Wifi_LibraryModeReady())
        swiWaitForVBlank();
    Wifi_SetChannel(6);
    Wifi_MultiplayerAllowNewClients(true);

    Wifi_BeaconStart("NintendoDS", 0xCAFEF00D);
    swiWaitForVBlank();
    swiWaitForVBlank();

    while(1)
    {

        scanKeys();
        u16 keys_down = keysDown();
        swiWaitForVBlank();

        Wifi_ConnectedClient client[MAX_CLIENTS];
        int num_clients = Wifi_MultiplayerGetClients(MAX_CLIENTS, &(client[0]));

        if(num_clients > 0)
        {
            snprintf(debugBuf, sizeof(debugBuf), "%d PRESS A TO SEND", num_clients);
        }
        else
        {
            snprintf(debugBuf, sizeof(debugBuf), "waiting for client");
        }
        NF_WriteText(0, 0, 1, 4, debugBuf);
        NF_UpdateTextLayers();

        if((keys_down & KEY_A) && num_clients > 0)
            break;
    }
    Wifi_MultiplayerAllowNewClients(false);

    NF_WriteText(0, 0, 1, 3, "sending tiles");
    NF_UpdateTextLayers();

    int sent = 0;
    int currentBatch = 0;

    tileBatchPacket batch;
    batch.count = 0;
    batch.isEnd = false;
    batch.batchNum = currentBatch;

    for(int y = 0; y < GRID_Y; y++)
    {
        for(int x = 0; x < GRID_X; x++)
        {
            if(TILE_MAP[y][x] == AIR)
                continue;

            batch.tiles[batch.count].x = x;
            batch.tiles[batch.count].y = y;
            batch.tiles[batch.count].blockID = TILE_MAP[y][x];
            batch.count++;

            if(batch.count >= BATCH_SIZE)
            {
                ackBatch = -1;

                while(ackBatch != currentBatch)
                {
                    swiWaitForVBlank();
                    sendTiles(&batch);

                    if(Wifi_MultiplayerGetClientMask() == 0)
                    {
                        snprintf(debugBuf, sizeof(debugBuf), "client lost batch %d", currentBatch);
                        NF_WriteText(0, 0, 1, 6, debugBuf);
                        NF_UpdateTextLayers();
                        goto client_lost;
                    }

                    snprintf(
                        debugBuf, sizeof(debugBuf), "S batch %d ackbatch %d sent %d", currentBatch, ackBatch, sent);
                    NF_WriteText(0, 0, 1, 7, debugBuf);
                    NF_UpdateTextLayers();
                }

                sent += batch.count;
                currentBatch++;
                batch.count = 0;
                batch.batchNum = currentBatch;
            }
        }
    }

    if(batch.count > 0)
    {
        ackBatch = -1;

        while(ackBatch != currentBatch)
        {
            swiWaitForVBlank();
            sendTiles(&batch);

            if(Wifi_MultiplayerGetClientMask() == 0)
            {
                snprintf(debugBuf, sizeof(debugBuf), "client lost batch %d", currentBatch);
                NF_WriteText(0, 0, 1, 6, debugBuf);
                NF_UpdateTextLayers();
                goto client_lost;
            }

            snprintf(debugBuf, sizeof(debugBuf), "S batch %d ackbatch %d sent %d", currentBatch, ackBatch, sent);
            NF_WriteText(0, 0, 1, 7, debugBuf);
            NF_UpdateTextLayers();
        }

        sent += batch.count;
        currentBatch++;
    }

    batch.count = 0;
    batch.isEnd = true;
    batch.batchNum = currentBatch;
    ackBatch = -1;

    while(ackBatch != currentBatch)
    {
        swiWaitForVBlank();
        sendTiles(&batch);

        if(Wifi_MultiplayerGetClientMask() == 0)
            break;

        snprintf(debugBuf, sizeof(debugBuf), "S batch %d ackbatch %d sent %d", currentBatch, ackBatch, sent);
        NF_WriteText(0, 0, 1, 7, debugBuf);
        NF_UpdateTextLayers();
    }

    snprintf(debugBuf, sizeof(debugBuf), "%d tiles sent", sent);
    NF_WriteText(0, 0, 1, 6, debugBuf);
    NF_UpdateTextLayers();

client_lost:
    Wifi_DisconnectAP();
    Wifi_IdleMode();
    state = EDITOR;
}

bool AccessPointSelectionMenu()
{
    char debugBuf[64];

    NF_UpdateTextLayers();

    Wifi_MultiplayerClientMode(sizeof(batchAckPacket));

    while(!Wifi_LibraryModeReady())
        swiWaitForVBlank();

    for(int i = 0; i < 50; i++)
    {
        snprintf(debugBuf, sizeof(debugBuf), "scanning %d", i);
        NF_WriteText(0, 0, 1, 0, debugBuf);
        NF_UpdateTextLayers();

        Wifi_ScanMode();

        for(int i = 0; i < 120; i++)
            swiWaitForVBlank();

        int numAPs = Wifi_GetNumAP();

        snprintf(debugBuf, sizeof(debugBuf), "%d APs", numAPs);
        NF_WriteText(0, 0, 1, 1, debugBuf);
        NF_UpdateTextLayers();

        if(numAPs > 0)
        {
            Wifi_AccessPoint ap;
            Wifi_GetAPData(0, &ap);
            AccessPoint = ap;

            snprintf(debugBuf, sizeof(debugBuf), "'%s'", ap.ssid);
            NF_WriteText(0, 0, 1, 2, debugBuf);
            NF_UpdateTextLayers();

            return true;
        }
    }

    return false;
}

void clientWarning()
{
    NF_WriteText(0,0,1,6,"STILL IN DEVELOPMENT");
    NF_WriteText(0,0,1,8, "LEVELS UP TO 10K TILES CAN BE");
    NF_WriteText(0,0,1,9, "SENT");
    NF_WriteText(0,0,1,11, "PRESS B OR THE SCREEN TO RETURN");
    NF_WriteText(0,0,1,13, "IF STUCK AT CONNECTING GO BACK AND TRY AGAIN");
}
void ClientMode(gameContext *ctx)
{
    char debugBuf[64];
    NF_ClearTextLayer(0, 0);
    levelDone = false;
    pendingBatchAck = -1;
    pendingBatchCount = 0;
    pendingBatchEnd = false;

    for(int y = 0; y < GRID_Y; y++)
    {
        for(int x = 0; x < GRID_X; x++)
        {
            TILE_MAP[y][x] = AIR;
        }
    }
    clientWarning();
    if(!AccessPointSelectionMenu())
    {
        clientWarning();
        //NF_WriteText(0, 0, 1, 3, "no ap found");
        //NF_UpdateTextLayers();
        //state = EDITOR;
        //return;
    }
    clientWarning();
    Wifi_MultiplayerFromHostSetPacketHandler(FromHostPacketHandler);
    NF_WriteText(0, 0, 1, 3, "connecting");
    NF_UpdateTextLayers();

    Wifi_ConnectOpenAP(&AccessPoint);

    while(1)
    {
        clientWarning();
        swiWaitForVBlank();
        int status = Wifi_AssocStatus();

        if(status == ASSOCSTATUS_ASSOCIATED)
            break;

        if(status == ASSOCSTATUS_CANNOTCONNECT || ctx->input->buttonsDown & KEY_TOUCH || ctx->input->buttonsDown & KEY_B)
        {
            NF_WriteText(0, 0, 1, 4, "cannot connect");
            NF_UpdateTextLayers();
            state = EDITOR;
            return;
        }
    }

    NF_WriteText(0, 0, 1, 4, "associated");
    NF_UpdateTextLayers();

    int received = 0;

    while(!levelDone)
    {
        swiWaitForVBlank();
        clientWarning();
        if(pendingBatchAck != -1)
        {
            batchAckPacket ack;
            ack.batchNum = (u16)pendingBatchAck;
            Wifi_MultiplayerClientReplyTxFrame(&ack, sizeof(ack));

            if(pendingBatchEnd)
            {
                levelDone = true;
            }
            else
            {
                received += pendingBatchCount;
                snprintf(debugBuf, sizeof(debugBuf), "recieved %d got %d", pendingBatchAck, received);
                NF_WriteText(0, 0, 1, 5, debugBuf);
                NF_UpdateTextLayers();
            }

            pendingBatchAck = -1;
            pendingBatchCount = 0;
            pendingBatchEnd = false;
        }
    }

    snprintf(debugBuf, sizeof(debugBuf), "done%d tiles", received);
    NF_WriteText(0, 0, 1, 2, debugBuf);
    NF_UpdateTextLayers();

    Wifi_DisconnectAP();
    Wifi_IdleMode();
    state = EDITOR;
}
