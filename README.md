# Overview
## About ShareDT
* This project is aimed to share and control desktop among different platform (MacOS, Windows, and Linux).
* The shared desktop can be the whole screen, a part region of screen and or a particular window of an App.
* ShareDT internally continuously captures screens, compress the frames captured through ```ffmpeg```, then send to client through ```libvnc```.
* Inputs including keyboard and mouse are supported. Client can control remote desktop through keyboard and mouse.

### NOTES
* Please note that ```libvnc``` transaction currently is not support encryption and authentication, which will be delivered in future.
* Audio capturing is not supported for now. Server cannot send audio data to client.

## Build
* Please refer to [Build.md](Build.md)

## Usage
### Start Server (```ShareDTServer```) on the host you want to share screen.
* Start main server
```
$ ./server/bin/ShareDTServer start
Starting shareDTServer
ShareDTServer Started
```
* Show the windows id or montor id you intend to share
```
$ ./server/bin/ShareDTServer show
Windows Lists:
...
Handle: 603	Pid: 6091	Window name: Menubar
Handle: 3523	Pid: 3177	Window name: shareDT – README.md
Handle: 88	Pid: 795	Window name: Desktop Picture - michael-rogers-1156531-unsplash.jpg
...
Monitor Lists:
Name: Display-0	id: 478241811	index: 0	offset: { 0, 0 }	size: { 3840, 2160 }	OriginalOffset: { 0, 0 }	OriginalSize : { 1920, 1080 }	scale: 2.000
Name: Display-1	id: 69734662	index: 1	offset: { 1920, 0 }	size: { 4096, 2560 }	OriginalOffset: { 1920, 0 }	OriginalSize : { 2048, 1280 }	scale: 2.000

```
* Capture the Monitor id ```69734662```, and share it:
```
$ ./server/bin/ShareDTServer capture -c mon -i 69734662 --wid test
Starting Capture ID(CID) = test
Status: Successfully created Capture Server on port: 5900
```
* Capture the bounds of window
```
$ ./server/bin/ShareDTServer capture -c part -b 0,0,1400,800 --wid testbond
Starting Capture ID(CID) = testbond
Status: Successfully created Capture Server on port: 5901
```
* Capture windows with handle 3523(```Window name: shareDT – README.md``` in above show command)
```
$ ./server/bin/ShareDTServer capture -c win -h 3523 --wid testwin
Starting Capture ID(CID) = testwin
Status: Successfully created Capture Server on port: 5904
```
* Status of all of the started server
```
$ ./server/bin/ShareDTServer status
Capture Server status:
Capture Server on port: 5900	 Status: Started	 WID: test
Capture Server on port: 5901	 Status: Started	 WID: testbond
Capture Server on port: 5904	 Status: Started	 WID: testwin
```
* Server side process tree(```26034``` is ShareDT main server, ```26043``` and ```26131``` are capture server)
```
$ pstree
-+= 00001 root /sbin/launchd
...
 |-+- 26034 yimingz ./server/bin/ShareDTServer start
 | |--- 26043 yimingz /Users/yimingz/X/source/shareDT/cmake-build-debug/install/server//bin/ShareDTServer newCapture -c mon -i 69734662 --wid test --daemon -rfbport 5900
 | |--- 26131 yimingz /Users/yimingz/X/source/shareDT/cmake-build-debug/install/server//bin/ShareDTServer newCapture -c win -h 3523 --wid testwin --daemon -rfbport 5904
...
```
* Usage of Capture command
```
$ ./server/bin/ShareDTServer capture --help
--help                       Help message

The following options related with StartServer option
-c or --capture win/mon/part Capture method(single windows, monitor or partial)
-n or --name    capture-name Capture name for window or monitor(only applied to win and mon capture
                             For window capture (-capture win), name is the handler of win,
                             you can find the handler by -s/-showhandler option
-h or --handler handler      The handler for windows capture
-b or --bounds  t,l,r,b      Capture bounds for partial, top left right and bottom
-s [window/win mon/monitor]  Show the window(default) or monitor if [mon/montior] specified
                             If show window, window that doesn't have name would not be printed
-showall                     Show all of the window even without window names
-i or --id ID                For monitor capture, capture the specific monitor id
-p or --process pid          For window capture (-capture win), capture the specific process id's window
                             This option can overwrite -n/-name
--daemon                     Running in daemon mode
--wid                        Specify the working id (wid) of the capture server. If not
                             sepcified, ShareDTServer will generate a random wid for it.


...
```
* Stop capture server
```
$ ./server/bin/ShareDTServer stop --wid testbond
Stopping Capture Server
Capture Server Stopped: testbond

$ ./server/bin/ShareDTServer status
Capture Server status:
Capture Server on port: 5900	 Status: Started	 WID: test
Capture Server on port: 5901	 Status: Stopped	 WID: testbond
Capture Server on port: 5904	 Status: Started	 WID: testwin
```
* Stop all capture server
```
$ ./server/bin/ShareDTServer stop
Stopping ShareDTServer
ShareDTServer Stopped
```
### ShareDTClient
* Help message:
```
Usage:
    ./client/bin/ShareDTClient [options] <serveraddr:vncport>

Options:
    -encodings <mpeg2_422, mpeg4_420, x265_420, x265_422, x265_444, zlib, raw>

Example:
    ./client/bin/ShareDTClient -encodings mpeg2_422 192.168.56.110:0
```
* Encoding name is seperated to ```<codec name>```_```<pix format(YUV420, YUV422 or YUV444)>```
* Default encoding is ```mpeg2_422```. If client and server are on the same machine, default is ```raw```.

### Example of ShareDTClient and ShareDTServer
* A simple example that shows the ```ShareDTClient```(Left Image) connected to server and displays the images captured by ```ShareDTserver```(Right Image).
![Alt text](image/Example-client-server.png?raw=true "ShareDTServer and ShareDTClient")
* Noted above, there is lose of resolution compared to original server side with ```mpeg2_422```, this is expected to reduce the network bandwidth.