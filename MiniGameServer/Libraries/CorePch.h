#pragma once

#include <iostream>
#include <thread>
#include <windows.h>
#include <cstdint>
#include <string>
#include <atomic>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib")
#include <MSWSock.h>

#include "MyMacros.h"
#include "objectPool.h"
#include "RWLock.h"
#include "Session.h"
#include "CompletionPortCore.h"
#include "NetAddress.h"
#include "SocketUtils.h"
#include "CoreGlobal.h"
#include "JobQueue.h"

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

using namespace std;
