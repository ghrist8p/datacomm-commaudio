
#ifndef _COMMON_H_
#define _COMMON_H_

#include <winsock2.h>
#include <windows.h>
#include "synchronizationHelper.h"
#include "Client/Sockets.h"


#pragma warning(disable:4996)

#define MUSICSTREAM '1'
#define MICSTREAM '2'
#define CHANGE_STREAM '3'
#define NEW_SONG '4'
#define DOWNLOAD '5'
#define REQUEST_DOWNLOAD '6'
#define CANCEL_DOWNLOAD '7'
#define ACTUAL_MUSIC '8'
#define DISCONNECT '9'

#define WM_SEEK (WM_USER + 22)

#endif
