#include "Playlist.h"
#include <string.h>

#include <vector>
#include <Windows.h>

Playlist::Playlist( wchar_t * _dir )
{
    static int curId = 0;

    size_t pathSize = wcsnlen_s( _dir, STR_LEN ) + 1;
    sDir = new wchar_t[ pathSize ];
    memset( sDir, 0, pathSize );
    memcpy( sDir, _dir, pathSize );

	hFind = FindFirstFile(sDir, &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
        int err = GetLastError();
		MessageBeep(1);
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
            SongName temp;
            temp.id = ++curId;
            sprintf_s( temp.filepath, "%S", ffd.cFileName );
            playlist.emplace_back( temp );
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
}

Playlist::~Playlist()
{
    delete [] sDir;
}

SongName * Playlist::getSong( int id )
{
    for( std::vector< SongName >::iterator it = playlist.begin()
       ; it != playlist.end()
       ; ++it )
    {
        if( id == it->id )
            return &(*it);
    }
    return NULL;
}
