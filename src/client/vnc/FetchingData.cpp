#include "FetchingData.h"
#include <png.h>
#include <iostream>

#define rfbBackChannel 155

typedef struct backChannelMsg {
    uint8_t type;
    uint8_t pad1;
    uint16_t pad2;
    uint32_t size;
} backChannelMsg;

static int backChannelEncodings[] = { rfbBackChannel, 0 };

static rfbBool handleBackChannelMessage(rfbClient* client,
                                        rfbServerToClientMsg* message)
{
    backChannelMsg msg;
    char* text;

    if(message->type != rfbBackChannel)
        return FALSE;

    //rfbClientSetClientData(client, sendMessage, sendMessage);

    if(!ReadFromRFBServer(client, ((char*)&msg)+1, sizeof(msg)-1))
        return TRUE;
    msg.size = rfbClientSwap32IfLE(msg.size);
    text = (char *) malloc(msg.size);
    if(!ReadFromRFBServer(client, text, msg.size)) {
            free(text);
            return TRUE;
        }

    rfbClientLog("got back channel message: %s\n", text);
    free(text);

    return TRUE;
}

static rfbClientProtocolExtension backChannel = {
    backChannelEncodings,		/* encodings */
    NULL,				        /* handleEncoding */
    handleBackChannelMessage,	/* handleMessage */
    NULL,				/* next extension */
    NULL,				/* securityTypes */
    NULL				/* handleAuthentication */
};

FetchingDataFromServer::FetchingDataFromServer (int argc, char **argv) :
        _client(rfbGetClient(VNC_BITS_PER_SAMPLE,
                             VNC_SAMPLS_PER_PIXEL,
                             VNC_BYTES_PER_PIXEL)),
        _isInited(false)
{
    _client->GotFrameBufferUpdate = HandleRectFromServer;
    rfbClientRegisterExtension(&backChannel);

    if (!rfbInitClient(_client,&argc,argv))
        return;

    _isInited = true;
    _shutdown = false;

    _client->_fetcher = this;
    _client->_serverClosed = 0;
}

FetchingDataFromServer::~FetchingDataFromServer ()
{
    rfbClientCleanup(_client);
}

void FetchingDataFromServer::run ()
{
    if (!isInited()) {
        std::cout << "Failed to initialise vnc client" << std::endl;
        return;
    }

    while (!_stopped && !_client->_serverClosed) {
        /* After each idle second, send a message */
        if(WaitForMessage(_client, 1000) > 0) {
            HandleRFBServerMessage(_client);
        }
    }

    if (_client->_serverClosed) {
        std::cout << "server connection closed, start to emit message" << std::endl;
        emit serverConnectionClosedSend(); 
    }

    std::cout << "Shutdown FetchingDataFromServer" << std::endl;
    _shutdown = true;
}

void FetchingDataFromServer::HandleRectFromServer(rfbClient* client, int x, int y, int w, int h)
{
    static int index = 0;
    char path[128] = { 0 };
    sprintf(path, "File_before_sentHandleRect.%d.png", index++);

    auto * fetcher = (FetchingDataFromServer *) client->_fetcher;

    // check if the old frame is used
    if (fetcher->getFrame().frame.isUsed())
        fetcher->HandleRect(client);
    else {
        std::cout << "Waiting for fetcher's HandleRect " << std::endl;
    }
}

void FetchingDataFromServer::HandleRect (rfbClient* client)
{
    if (std::string(client->appData.encodingsString) == "ffmpeg" && !client->_available_frame) return;
    _frame.frame.set(client->frameBuffer, client->width*client->height*4);
    _frame.w = client->width;
    _frame.h = client->height;
    client->_sequence++;
    emit sendRect(client);
}
