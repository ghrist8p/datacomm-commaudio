// server

Start Listening for Connections
while
{
	if(there is a new client)
	{
		Create a new Session.
		Add session to session List.
	]
	if(stopped)
	{
		free resources
	}
}

/**
Interfaces directly with the client, one is created for each client. 
This process is used to exchange control data with the client 
@socket Client - New client connection
**/
Session(socket client)
{	
	Send List of Songs to the Client
	Send Client the list of all the sessions (Clients Connected)
	
	start Listening from the socket
	while
	{			
		if(stopped)
		{
			free resources
		}
		
		Check Client List.
		
		if(there is an update)
		{
			Send update to every client
		}
		
		Read 'flag' from the socket.
		
		if(flag == download request)
		{
			read from the socket
			get the Song out of the message received
			Call Upload(Song, client)
		}		
		else if(flag == stream request)
		{
			read from the socket
			get the Song out of the message received
			Add Song to Song Queue.
		}
		else if(flag == disconnect flag)
		{
			Remove client from client list
			close socket and connection
			free resources and exit the process
		}
		else
		{
			Flag Error
			Let the client know
		}
		
	}
}


/**
Reads from the currently selected music file, and multicasts it on the network
**/
Stream()
{
	While 
	{
		get resources
		
		if(there are Song requests)
		{
			Open the File(Song)
			
			while
			{
				Start Playing Song
				//Let Client know the song being played
						
				Multicast the audio using the UDP socket
				
				if(Song is complete)
				{
					Close File
					break
				}
				
				if(there is a new request)
				{
					Close File
					break
				}
			}
		}

		if(stopped)
		{
			free resources
		}
		
	}
	

}



/**
Spawned whenever a session's client requests to download a song file. 
It reads from the requested song file, and sends it to the requesting client
@param Song   - piece of sound requested by the client
@param Client - client with the request 
**/
Upload(string Song, Session client)
{
	Open the File(Song)
	Send File to client using the TCP Socket.
	Close File
}