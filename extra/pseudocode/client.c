// client

Client Connects to Server.
Server will Send the Client List to the Client.

/**
Exchanges data between the TCP socket, music files, and the music buffering process.
If the received data is music data, it will be written to a file, or forwarded to the music 
buffering process. Otherwise, it is control information, like requesting to download a file,
or requesting to change streams. Running as long as the program is in the connected state.
**/
Control()
{
	acquire resources
	
	while()
	{
		Read from the Socket()
		Wait for events from the socket read AND User Requests
		switch(event)
		
		case there is an update to client list:
			{
				Modify Client List.
				break
			}
			
		case a song file has been received:
			{
				read the data
				write the data to Message Queue
				
			}			
		case User wants to request a song to download:
			{
				Send flag
				Send song request trough TCP socket
			}
		
		case User wants to request a song to play in stream)
			{
			Send flag
			Send song request trough TCP socket
			}
		
		case stopped - disconnected
			{
				free resources
				break
			}
		default:
				break;
	}
	
}

/**
Reads from the UDP socket, and writes the read data to the corresponding jitter 
buffer depending on the type of data that is read from the socket. Running as long as 
the program is in the connected state.
**/
Receive()
{
	acquire resources
	
	while
	{
		Read from UDP Socket
		Wait for an event from the socket read.
		switch(data)
		{
			case voice:
						Write data to Voice Jitter Buffer		
						break;
			case music:
					
						Write data to Music Jitter Buffer
						break
			default
						Prompt for unknown type received
						break;
		}
		
		if(stopped - disconnected)
		{
			free resources
			break
		}
	}
}

/**
Reads from the transmit buffer, and sends it out the UDP socket to a user specified
address. Running as long as the program is in the connected state.
**/
Transmit()
{
	acquire resources

	while	
	{
		if(transmit buffer has data)
		{
			get UDP socket - Client Information
			Send the data trough the socket		
		}
		
		if(stopped - disconnected)
		{
			free resources
			break
		}
	}
}

/**
Reads from the music jitter buffer, and writes it the corresponding music buffer
file. Running as long as the program is in the connected state.
**/
Music buffering()
{
	acquire resources
	
	while
	{
		if(there is data in the music jitter buffer) // event received
		{
			Open File
			read the data
			if(data is music data)
			{
				process data
				Write data to the music buffer
			}
			Close File			
		}
		
		if(there is a music file in the message queue) // event received
		{
			Open File
			read the data
			if(data is music data)
			{
				Write data to the music buffer
			}
			Close File
		
		}
		
		if(stopped - disconnected)
		{
			free resources
			break
		}
	}
}

/**
Reads from the voice jitter buffer, and writes the data to the audio buffer. 
Running as long as the application is in the connected state.
**/
Voice buffering()
{
	acquire resources	
	
	while
	{
		if(there is data in the voice jitter buffer)
		{
			read the data
			if(data is voice data)
			{
				process data
				Write data to the audio buffer
			}

			if(stopped - disconnected)
			{
				free resources
				break
			}
		}	
	}
}

/**
Reads from the mic, and writes the read data into the transmit buffer. 
Running as long as the application is in the transmitting state.
**/

Record()
{
	Start Recording audio from the default mic.
	
	if(Stop record event received from GUI)
	{
		write recording to transmit buffer.		
	}
		
	if(stopped - disconnected)
	{
		free resources
		break
	}
}



/**
Reads from the temp files created to hold the streamed music data, and 
writes the data to the audio buffers. Running as long as the application is in
the connected state.
**/
Music Reader()
{
	acquire resources
	
	while
	{
		if(there is data in the music buffer)
		{
			get the file
			read from it
			Send data to Audio Buffer
			close the file		
		}
		
		if(stopped - disconnected)
		{
			free resources
			break
		}
	}
	
}

/**
Reads from the audio buffer, and plays it out through the system's speakers.
Running as long as the application is in
the connected state.
**/
Output()
{
	acquire resources
	
	while
	{
		if(there is data in the audio buffer)
		{
			Read data
			Play data- through speakers		
		}
		
		if(stopped - disconnected)
		{
			free resources
			break
		}
	}
}