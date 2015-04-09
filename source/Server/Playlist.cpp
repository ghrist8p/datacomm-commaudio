#include "Playlist.h"
#include <string.h>

#include <vector>

// static function forward declarations
static int getSongfileInfo(SongName* song, wchar_t* filepath, wchar_t* filename, int songId);

// Playlist implementation
Playlist::Playlist( wchar_t * _dir )
{
	static int curId = 0;

	size_t pathSize = wcsnlen_s( _dir, STR_LEN ) + 1;
	sDir = new wchar_t[ pathSize ];
	memset( sDir, 0, pathSize );
	memcpy( sDir, _dir, pathSize * sizeof( wchar_t ) );

	OutputDebugString( sDir );
	OutputDebugString( L"\n" );

	hFind = FindFirstFile(sDir, &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		wchar_t errorStr[256] = {0};
		swprintf( errorStr, 256, L"FindFirstFile() failed: %d", GetLastError() );
		#ifdef DEBUG
		MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
		#endif
		return;
	}

	// reading all files in the music folder
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
		}
		else
		{
			// get directory name
			wchar_t directory[STR_LEN];
			wchar_t* fileName;
			GetFullPathName(sDir,pathSize,directory,&fileName);

			// append music file name to directory name
			wsprintf(fileName,L"%s",ffd.cFileName);

			// get the song information
			SongName temp;
			getSongfileInfo(&temp,directory,fileName,++curId);
			playlist.emplace_back(temp);
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
}

Playlist::~Playlist()
{
	delete [] sDir;
}


wchar_t * Playlist::getSongPath( int id )
{
	int lastBackSlash = wcslen( sDir );
	while( sDir[ lastBackSlash ] != L'\\' ) --lastBackSlash;
	++lastBackSlash;

	std::vector< SongName >::iterator it;
	for( it = playlist.begin(); it != playlist.end() && id != it->id; ++it );

	if( it != playlist.end() )
	{
		wchar_t * output = new wchar_t[ lastBackSlash + wcslen( it->filepath ) + 1 ];
		memcpy( output, sDir, lastBackSlash * sizeof( wchar_t ) );
		memcpy( output + lastBackSlash, it->filepath, ( wcslen( it->filepath ) + 1 ) * sizeof( wchar_t ) );
		return output;
	}
	else
	{
		return NULL;
	}
}

SongName * Playlist::getSong( int id )
{
	std::vector< SongName >::iterator it;
	for( it = playlist.begin(); it != playlist.end() && id != it->id; ++it );

	if( it != playlist.end() )
	{
		return &(*it);
	}
	else
	{
		return NULL;
	}
}

// static function implementations
int getSongfileInfo(SongName* song, wchar_t* filepath, wchar_t* filename, int songId)
{
	// bail out if cant open file
	sprintf_s(song->cFilepath,"%S",filepath);
	FILE* fp = fopen(song->cFilepath, "rb");
	if(!fp)
	{
		int err = GetLastError();
		return 1;
	}

	// set song id and filename
	song->id = songId;
	wsprintf(song->filepath,L"%s",filename);
	sprintf_s(song->cFilename,"%S",filename);

	// allocate memory to hold the song file meta data.
	char id[5];
	unsigned long size;
	short format_tag, channels, block_align, bits_per_sample;
	unsigned long format_length, sample_rate, avg_bytes_sec, data_size;
	int data_read = 0;

	fread(id, sizeof(char), 4, fp);
	id[4] = '\0';

	// check if file type is RIFF
	if (strcmp(id, "RIFF"))
	{
		#ifdef DEBUG
		MessageBox(NULL, L"NOT WAVE", L"ERROR", MB_ICONERROR);
		#endif
		fclose(fp);
		return 1;
	}

	// get RIFF headers
	fread(&song->size, sizeof(unsigned long), 1, fp);
	song->size = song->size - 44;
	fread(id, sizeof(char), 4, fp);
	id[4] = '\0';

	// check if file type is WAVE
	if (strcmp(id, "WAVE"))
	{
		#ifdef DEBUG
		MessageBox(NULL, L"NOT RIFF", L"ERROR", MB_ICONERROR);
		#endif
		fclose(fp);
		return 1;
	}

	//get WAVE headers
	fread(id, sizeof(char), 4, fp);
	fread(&format_length, sizeof(unsigned long), 1, fp);
	fread(&format_tag, sizeof(short), 1, fp);
	fread(&song->channels, sizeof(short), 1, fp);
	fread(&song->sample_rate, sizeof(unsigned long), 1, fp);
	fread(&avg_bytes_sec, sizeof(unsigned long), 1, fp);
	fread(&block_align, sizeof(short), 1, fp);
	fread(&song->bps, sizeof(short), 1, fp);
	fread(id, sizeof(char), 4, fp);
	fread(&data_size, sizeof(unsigned long), 1, fp);

	fclose(fp);
	return 0;
}
