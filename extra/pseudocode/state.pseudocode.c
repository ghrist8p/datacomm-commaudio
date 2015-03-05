SERVER PSEUDOCODE
=================

STATE: INITIALIZE
-----------------

Read last used port from config file
Create GUI
Go To State: IDLE


STATE: IDLE
-----------
IF user connects:
	Create a listening socket
	Attempt to bind socket to stored port

	WHILE the socket is not bound to a port:
		Prompt user for a different port
		
	Go To State: RUNNING
	
IF user exits:
	Cleanup GUI
	Go To State: TERMINATED
	
STATE: RUNNING
--------------
Listen for data on all sockets

IF a new connection is made:
	Store the new socket
	Send TCP message to socket with initialization data
	Send TCP message to all clients informing of new client
	
IF a client request a file download:
	Open requested file
	Write file to client socket
	
IF a client requests song change:
	Close current song (if exists)
	Clear buffers
	Open requested song file
	Send TCP message to all clients informing of song change
	Decode song data
	
	Write song data on multicast UDP socket
	
IF a client disconnects:
	Remove stored socket
	
IF user stops server:
	Go To State: STOPPING
	
STATE: STOPPING
---------------
Close all open files
Close all sockets
Clear Buffers
Go To State: IDLE



CLIENT PSEUDOCODE
=================

STATE: CONFIGURATION
--------------------
Create/Show "configuration" GUI
	Prompt for Port and Host

Go To State: CONNECTING

STATE: CONNECTING
-----------------
Create TCP socket / Connect To Server
Create UDP socket / Bind to Port

IF success
	Read initialization data from TCP socket
	Go To State: CONNECTED
IF failure
	Go To State: CONFIGURATION

	
STATE: CONNECTED
-----------------
IF receive voice data (on multicast socket)
	IF client IP in datagram matches current IP
		Set voice jitter buffer as active buffer
		Disable microphone input button
		Put voice data in voice jitter buffer
		Set 1s timer
			IF timer elapses before more voice data received
				Enable microphone input button
				Set music jitter buffer as active buffer
			ELSE
				Reset timer
	
ELSE IF user holding microphone input button
	Disable playback

IF receive music data (on multicast socket)
	Write data to music jitter buffer
	Write music data to temp music file
		
IF receive music data (on TCP socket)
	Write music data to temp music file
	
IF receive music file data (on TCP socket)
	Write data to open file
	
IF user requests song download
	Create new file with name of song
	Write request to TCP socket
	
	
ASYNCHRONOUS STATE: TRANSMITTING
--------------------------------
Read data from microphone
Write data to UDP multicast socket including client IP

Set 1s timer
		IF timer elapses before more voice data received
			Set music jitter buffer as active buffer
		ELSE
			Reset timer
			
ASYNCHRONOUS STATE: PLAYING MUSIC
---------------------------------
WHILE connected
	Read music data from temp file starting from seek point into music buffer
	IF data missing in loaded segment
		Request retransmission of missing data over TCP socket
	IF data exists and music buffer is active buffer
		Playback music buffer
		
ASYNCHRONOUS STATE: PLAYING VOICE
---------------------------------
WHILE connected
	Read data from voice buffer
	IF data exists and voice buffer is active buffer
		Playback voice buffer
	