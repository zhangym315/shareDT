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

    while (!_stopped) {
        /* After each idle second, send a message */
        if(WaitForMessage(_client,1000000)>0) {
            HandleRFBServerMessage(_client);
        }
        else// if(rfbClientGetClientData(client, sendMessage))
        {
            std::cout << "Hanlding senindg server message" << std::endl;
        }
    }

    std::cout << "Shutdown FetchingDataFromServer" << std::endl;
    _shutdown = true;
}

void FetchingDataFromServer::writeToFile(const char * file, int x, int y, int w, int h, uint8_t * frame)
{
    unsigned int width = w;
    unsigned int height = h;
    FILE *fp = fopen(file, "wb");
    if(!fp) return ;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_set_IHDR(png,
                 info,
                 width, height,
                 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    for ( int i=0 ; i<height ; i++) {
        uint8_t * ptr = frame + i*width*4;
        for ( int j=0; j<width*4; ) {
                *(ptr+j+3) = 0xff;
                j += 4;
            }
        png_write_row(png, (png_bytep)(ptr));
    }

    png_write_end(png, nullptr);
    fclose(fp);

    png_destroy_write_struct(&png, &info);

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
//    writeToFile(path, x, y, w, h, client->frameBuffer);
}

void FetchingDataFromServer::HandleRect (rfbClient* client)
{
    _frame.frame.set(client->frameBuffer, client->width*client->height*4);
    _frame.w = client->width;
    _frame.h = client->height;
    client->_sequence++;
    emit sendRect(client);
}
