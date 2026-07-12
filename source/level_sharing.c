//
// Created by Nicholas Soukmandjie on 12/07/2026.
//
#include  "level_sharing.h"


Wifi_AccessPoint AccessPoint;
int ackX = -1;
int ackY = -1;
volatile int pendingAckX = -1;
volatile int pendingAckY = -1;
bool levelDone = false;

void SendTile(u16 x, u16 y, u8 blockID)
{
    tilePacket pkt = { x, y, blockID };
    Wifi_MultiplayerHostCmdTxFrame(&pkt, sizeof(pkt));
}

void FromClientPacketHandler(Wifi_MPPacketType type, int aid, int base, int len)
{
    if (len < sizeof(tilePacket))
    {
        return;
    }

    if (type != WIFI_MPTYPE_REPLY)
        return;

    tilePacket pkt;
    Wifi_RxRawReadPacket(base, sizeof(pkt), (void *)&pkt);
    ackX = pkt.x;
    ackY = pkt.y;
}

void FromHostPacketHandler(Wifi_MPPacketType type, int base, int len)
{
    if (len < sizeof(tilePacket))
    {
        return;
    }
    if (type != WIFI_MPTYPE_CMD)
        return;

    tilePacket pkt;
    Wifi_RxRawReadPacket(base,sizeof(pkt),(void *)&pkt);

    if (pkt.x != END)
    {
        TILE_MAP[pkt.y][pkt.x] = pkt.blockID;
    }

    pendingAckX = pkt.x;
    pendingAckY = pkt.y;
}

void hostMode()
{
    char debugBuf[64];

    NF_ClearTextLayer(0, 0);
    NF_WriteText(0, 0, 1, 0, "starting");
    NF_UpdateTextLayers();


    Wifi_MultiplayerHostMode(MAX_CLIENTS, sizeof(tilePacket), sizeof(tilePacket));
    Wifi_MultiplayerFromClientSetPacketHandler(FromClientPacketHandler);

    while (!Wifi_LibraryModeReady()) swiWaitForVBlank();
    Wifi_SetChannel(6);
    Wifi_MultiplayerAllowNewClients(true);

    Wifi_BeaconStart("NintendoDS", 0xCAFEF00D);
    swiWaitForVBlank();
    swiWaitForVBlank();

    while (1)
    {

        scanKeys();
        u16 keys_down = keysDown();
        swiWaitForVBlank();

        Wifi_ConnectedClient client[MAX_CLIENTS];
        int num_clients = Wifi_MultiplayerGetClients(MAX_CLIENTS, &(client[0]));

        if (num_clients > 0)
        {
            snprintf(debugBuf, sizeof(debugBuf), "%d client A", num_clients);
        }
        else
        {
            snprintf(debugBuf, sizeof(debugBuf), "waiting for client");
        }
        NF_WriteText(0, 0, 1, 4, debugBuf);
        NF_UpdateTextLayers();


        if ((keys_down & KEY_A) && num_clients > 0)
            break;
    }
    Wifi_MultiplayerAllowNewClients(false);

    NF_WriteText(0, 0, 1, 3, "seding tiles");
    NF_UpdateTextLayers();

    int sent = 0;

    for (int y = 0; y < GRID_Y; y++)
    {
        for (int x = 0; x < GRID_X; x++)
        {
            if (TILE_MAP[y][x] == AIR)
                continue;

            ackX = -1;
            ackY = -1;

            while (!(ackX == x && ackY == y))
            {
                swiWaitForVBlank();
                SendTile(x, y, TILE_MAP[y][x]);

                if (Wifi_MultiplayerGetClientMask() == 0)
                {
                    snprintf(debugBuf, sizeof(debugBuf), "client lost %d,%d", x, y);
                    NF_WriteText(0, 0, 1, 6, debugBuf);
                    NF_UpdateTextLayers();
                    goto client_lost;
                }
            }

            sent++;

            if (sent % 20 == 0)
            {
                snprintf(debugBuf, sizeof(debugBuf), "%d tiles", sent);
                NF_WriteText(0, 0, 1, 6, debugBuf);
                NF_UpdateTextLayers();
            }
        }
    }

    NF_UpdateTextLayers();

    ackX = -1;

    while (ackX != END)
    {
        swiWaitForVBlank();
        SendTile(END, 0, 0);

        if (Wifi_MultiplayerGetClientMask() == 0)
            break;
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

    Wifi_MultiplayerClientMode(sizeof(tilePacket));

    while (!Wifi_LibraryModeReady()) swiWaitForVBlank();

    NF_WriteText(0, 0, 1, 0, "scanning");
    NF_UpdateTextLayers();

    Wifi_ScanMode();

    for (int i = 0; i < 60; i++)
        swiWaitForVBlank();

    int numAPs = Wifi_GetNumAP();

    snprintf(debugBuf, sizeof(debugBuf), "%d APs", numAPs);
    NF_WriteText(0, 0, 1, 1, debugBuf);
    NF_UpdateTextLayers();

    if (numAPs <= 0)
        return false;

    Wifi_AccessPoint ap;
    Wifi_GetAPData(0, &ap);
    AccessPoint = ap;

    snprintf(debugBuf, sizeof(debugBuf), "'%s'", ap.ssid);
    NF_WriteText(0, 0, 1, 2, debugBuf);
    NF_UpdateTextLayers();

    return true;
}

void ClientMode()
{
    char debugBuf[64];

    NF_ClearTextLayer(0, 0);
    levelDone = false;
    pendingAckX = -1;
    pendingAckY = -1;

    for (int y = 0; y < GRID_Y; y++)
    {
        for (int x = 0; x < GRID_X; x++)
        {
            TILE_MAP[y][x] = AIR;
        }
    }

    if (!AccessPointSelectionMenu())
    {
        NF_WriteText(0, 0, 1, 3,  "no ap found");
        NF_UpdateTextLayers();
        state =EDITOR;
        return;
    }

    Wifi_MultiplayerFromHostSetPacketHandler(FromHostPacketHandler);
    NF_WriteText(0, 0, 1, 3,  "connecting");
    NF_UpdateTextLayers();

    Wifi_ConnectOpenAP(&AccessPoint);

    while (1)
    {
        swiWaitForVBlank();
        int status = Wifi_AssocStatus();

        if (status == ASSOCSTATUS_ASSOCIATED)
            break;

        if (status == ASSOCSTATUS_CANNOTCONNECT)
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

    while (!levelDone)
    {
        swiWaitForVBlank();

        if (pendingAckX != -1|| pendingAckY != -1)
        {
            tilePacket ack = {(u16)pendingAckX,(u16)pendingAckY,0};
            Wifi_MultiplayerClientReplyTxFrame(&ack, sizeof(ack));

            if (pendingAckX == END)
            {
                levelDone = true;
            }
            else
            {
                received++;

                if (received % 20 == 0)
                {
                    snprintf(debugBuf, sizeof(debugBuf), "got %d tiles", received);
                    NF_WriteText(0, 0, 1, 7, debugBuf);
                    NF_UpdateTextLayers();
                }
            }

            pendingAckX = -1;
            pendingAckY = -1;
        }
    }

    snprintf(debugBuf, sizeof(debugBuf), "done%d tiles", received);
    NF_WriteText(0, 0, 1, 6, debugBuf);
    NF_UpdateTextLayers();


    Wifi_DisconnectAP();
    Wifi_IdleMode();
    state = EDITOR;
}


