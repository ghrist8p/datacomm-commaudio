#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <Windows.h>

#include "../protocol.h"

class Playlist
{
public:
    Playlist( wchar_t * _dir );
    ~Playlist();

    SongName * getSong( int id );

private:
    WIN32_FIND_DATA   ffd;
    HANDLE            hFind;
    wchar_t         * sDir;
    LARGE_INTEGER     filesize;

    std::vector< SongName > playlist;

    LPCSTR hi;
};

#endif