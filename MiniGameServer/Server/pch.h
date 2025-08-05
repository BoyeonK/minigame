#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0601
#define NOMINMAX

#ifdef _DEBUG
#pragma comment(lib, "Libraries.lib")
//#pragma comment(lib, "libprotobufd.lib")
#else
#pragma comment(lib, "Libraries.lib")
//#pragma comment(lib, "libprotobuf.lib")
#endif

#include "CorePch.h"
#include <grpcpp/grpcpp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include "S2D_Protocol.grpc.pb.h"
#include "S2C_Protocol.pb.h"

using namespace std;