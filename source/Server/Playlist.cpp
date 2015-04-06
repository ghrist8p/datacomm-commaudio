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
    memcpy( sDir, _dir, pathSize * sizeof( wchar_t ) );
    
    OutputDebugString( sDir );
    OutputDebugString( L"\n" );

	hFind = FindFirstFile(sDir, &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
        wchar_t errorStr[256] = {0};
        swprintf( errorStr, 256, L"FindFirstFile() failed: %d", GetLastError() );
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
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
            OutputDebugString( ffd.cFileName );
            OutputDebugString( L"\n" );
            memcpy( temp.filepath, ffd.cFileName, ( wcslen( ffd.cFileName ) + 1 ) * sizeof( wchar_t ) );
            playlist.emplace_back( temp );
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
}

Playlist::~Playlist()
{
    delete [] sDir;
}


char * Playlist::getSongPath( int id )
{
    int lastBackSlash = wcslen( sDir );
    while( sDir[ lastBackSlash ] != L'\\' ) --lastBackSlash;
    ++lastBackSlash;
    char * path = (char *) malloc( lastBackSlash + 1 );
    memset( path, 0, lastBackSlash + 2 );
    size_t retval = 0;
    wcstombs_s( &retval              // size_t        * pReturnValue
              , path                 // char          * mbstr
              , lastBackSlash + 1    // size_t          sizeInBytes
              , sDir                 // const wchar_t * wcstr
              , lastBackSlash + 1 ); // size_t          count 

    std::vector< SongName >::iterator it;
    for( it = playlist.begin(); it != playlist.end() && id != it->id; ++it );

    if( it != playlist.end() )
    {
        path = (char *) realloc( path, lastBackSlash + wcslen( it->filepath ) + 1 );
        sprintf_s( path + lastBackSlash, lastBackSlash + wcslen( it->filepath ) + 1, "%S", it->filepath );
        return path;
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
