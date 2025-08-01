#define _WIN32_WINNT 0x0601
#define NOMINMAX

#pragma once

#include <WinSock2.h>
#include <windows.h>
#include <grpcpp/grpcpp.h>

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>

#include <iostream>
#include <locale>
#include <codecvt>
#include <cwchar>
#include <sqlext.h>
#include <sqltypes.h> 
#include <sql.h> 
#include <cstdlib> 

using namespace std;

#include "S2D_Protocol.grpc.pb.h"
#include "objectPool.h"