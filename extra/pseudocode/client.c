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
	while()
	{
		Read from the Socket()
		{
			if(there is an update to client list)
			[
				Modify Client List.
			}
			
			if(a song file has been received)
			{
				read the data
				write the data to Music Bufferer //shared buffer
			}		
		}
		
		if(User wants to request a song to download)
		{
			Send flag
			Send song request trough TCP socket
		}
		
		if(User wants to request a song to play in stream)
		{
			Send flag
			Send song request trough TCP socket
		}	
	}
	
}

/**
Reads from the UDP socket, and writes the read data to the corresponding jitter 
buffer depending on the type of data that is read from the socket. Running as long as 
the program is in the connected state.
**/
Receive()
{
	while
	{
		Read from UDP Socket
		
		if(data == voice)
		{
			Write data to Voice Jitter Buffer		
		}
		else if(data == music)
		{
			Write data to Music Jitter Buffer
		}
		else
		{
			Prompt for unknown type received			
		}	
	}
}

/**
Reads from the transmit buffer, and sends it out the UDP socket to a user specified
address. Running as long as the program is in the connected state.
**/
Transmit()
{
	while
	{
		if(transmit buffer has data)
		{
			get UDP socket - Client Information
			Send the data trough the socket		
		}	
	}
}

/**
Reads from the music jitter buffer, and writes it the corresponding music buffer
file. Running as long as the program is in the connected state.
**/
Music buffering()
{
	while
	{
		if(there is data in the music jitter buffer)
		{
			read the data
			if(data is music data)
			{
				process data
				Write data to the music buffer
			}		
		}
		
		if(a file has been received from the control process) //probably by sharing a buffer
		{
			read the data
			if(data is music data)
			{
				Write data to the music buffer
			}	
		
		}
	}
}

/**
Reads from the voice jitter buffer, and writes the data to the audio buffer. 
Running as long as the application is in the connected state.
**/
Voice buffering()
{
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
		}	
	}
}

/**
Reads from the mic, and writes the read data into the transmit buffer. 
Running as long as the application is in the transmitting state.
**/

Record()
{
	if(record button is pressed)
	{
		Start Recording audio from the default mic.
		if(record button is pressed again or timeout)
		{
			Save recording to transmit buffer.		
		}
	]
}



/**
Reads from the temp files created to hold the streamed music data, and 
writes the data to the audio buffers. Running as long as the application is in
the connected state.
**/
Music Reader()
{
	while
	{
		if(there are temp files)
		{
			read data from file.
			Send data to Audio Buffer
		}
		
		if(user selects a song from downloaded songs list && there are no tempfiles)
		{
			get the file
			read from it.
			Send data to Audio Buffer.
		
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
	while
	{
		if(there is data in the audio buffer)
		{
			Read data
			Play data- through speakers		
		}	
	}
}






















