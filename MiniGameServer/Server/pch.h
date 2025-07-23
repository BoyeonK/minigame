#pragma once

#define WIN32_LEAN_AND_MEAN

#ifdef _DEBUG
#pragma comment(lib, "Libraries.lib")
#pragma comment(lib, "libprotobufd.lib")
#else
#pragma comment(lib, "Libraries.lib")
#pragma comment(lib, "libprotobuf.lib")
#endif

#include "CorePch.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rand.h>