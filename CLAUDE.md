# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

이 문서는 **저장소 전역**에 해당하는 내용만 다룬다. 각 컴포넌트의 내부 구조는 해당 디렉토리의 CLAUDE.md를 볼 것.

## 저장소 구성

하나의 git 저장소 안에 3개의 독립된 프로그램이 있고, 2개의 생성된(generated) 프로토콜로 연결되어 있다.

| 디렉토리 | 내용 | 상세 | 빌드 방법 |
|---|---|---|---|
| `MiniGameServer/` | 게임 서버 (C++17, Windows/IOCP). `Server.sln` = `Server`(exe) + `Libraries`(정적 라이브러리) | [MiniGameServer/CLAUDE.md](MiniGameServer/CLAUDE.md) | MSBuild, x64 |
| `MiniGameDB/` | DB 게이트웨이 서버 (C++17). ODBC로 SQL Server에 붙는 gRPC 서버 | [MiniGameDB/CLAUDE.md](MiniGameDB/CLAUDE.md) | MSBuild, x64 |
| `MiniGameClient/` | Unity 6000.4.2f1 클라이언트 (C#) | [MiniGameClient/CLAUDE.md](MiniGameClient/CLAUDE.md) | Unity 에디터 |
| `bin/` | `.proto` 원본 + `protoc` 산출물. 빌드 디렉토리가 아니라 **프로토콜 코드 생성이 이루어지는 곳** | 아래 "프로토콜 코드 생성" | `GenPacket.bat` |

`server/` (소문자, git 미추적)는 빈 VS 스캐폴드이며 프로젝트의 일부가 아니다.

### 그 밖의 문서

| 파일 | 내용 |
|---|---|
| `README.md` | 프로젝트 소개와 설계 의도. 각 디렉토리로 들어가는 입구 |
| `progress.md` | 완료된 작업 로그, TODO, 알려진 이슈 |
| `docs/` | 아키텍처 다이어그램, 이전 README |

## 빌드 및 실행

**서버 / DB** — MSBuild, x64 전용 (.sln에 `Any CPU`/`Win32` 구성도 있지만 실제 타겟이 아니다):

```powershell
msbuild MiniGameServer\Server.sln /p:Configuration=Debug /p:Platform=x64
msbuild MiniGameDB\MiniGameDB.sln /p:Configuration=Debug /p:Platform=x64
```

의존성(protobuf, gRPC, OpenSSL)은 **vcpkg**(`C:\vcpkg\installed\x64-windows`)에서 자동 링크로 들어온다. 그래서 `.vcxproj`의 `AdditionalDependencies`가 거의 비어 있다.

현재 검증된 조합: Visual Studio 2026 Community(v18) + PlatformToolset **v145**, vcpkg의 protobuf **6.33.4** / grpc **1.76.0** / abseil **20260107.1** / openssl 3.6.2.

x64가 아닌 플랫폼으로 빌드하면 안 되는 이유와 그때 나는 `LNK2019`의 정체는 [MiniGameServer/CLAUDE.md](MiniGameServer/CLAUDE.md)에 있다.

### 실행 순서

`MiniGameDB` → `Server` → Unity 클라이언트. 게임 서버가 시작 시 `HelloAsync()`로 DB를 호출하므로 순서를 지켜야 한다.

각 프로그램의 실행 전제 조건(`.env`, 인증서, `GitIgnores.cs`)은 전부 gitignore 대상이며 해당 컴포넌트의 CLAUDE.md에 정리되어 있다.

### 빌드가 깨질 때 — vcpkg 업데이트가 원인인 두 가지

`vcpkg upgrade` 후 아래 두 증상이 같이 나타난다. **두 C++ 프로젝트 모두에 해당한다.** 둘 다 코드 잘못이 아니라 버전 어긋남이다.

1. **`error C1189: Protobuf C++ gencode is built with an incompatible version`** (`*.pb.h` 16번째 줄 근처)
   커밋된 생성 코드와 설치된 protobuf 런타임의 버전이 다르다. `.pb.h` 상단의 `Protobuf C++ Version:` 주석과 `protoc --version`을 비교해보면 바로 확인된다.
   → 아래 "프로토콜 코드 생성" 절차대로 **C++ gencode만** 재생성한다. `--csharp_out`은 돌리지 말 것 — 클라이언트의 `.cs`는 Unity가 들고 있는 `Google.Protobuf.dll` 버전에 묶여 있고, C++/C# gencode는 서로 독립적이라(와이어 포맷은 호환) 한쪽만 올려도 된다.

2. **`error C4996: absl::MutexLock::MutexLock ... Use the constructor that takes a reference instead`** (`grpcpp/completion_queue.h`, `grpcpp/support/server_callback.h`)
   abseil이 포인터를 받는 `MutexLock` 생성자를 deprecated 처리했는데 grpc 헤더가 아직 그것을 쓴다. 우리 코드가 아니라 **서드파티 헤더**의 문제이고, `<SDLCheck>true</SDLCheck>`(`/sdl`)가 C4996을 에러로 승격시키기 때문에 빌드가 멈춘다.
   → `Server.vcxproj`와 `MiniGameDB.vcxproj`의 x64 Debug/Release `ClCompile`에 `<AdditionalOptions>/w34996 %(AdditionalOptions)</AdditionalOptions>`가 들어 있어 다시 경고로 되돌린다. `/sdl`의 나머지 검사는 그대로 유지된다. **`AdditionalOptions`여야 한다** — `/sdl` 뒤에 붙어야 이기기 때문이다. 같은 이유로 `CL` 환경변수(앞에 붙음)로는 안 되고 `_CL_`(뒤에 붙음)이어야 한다.

### 테스트

테스트 스위트는 없다. `MiniGameDB`가 `_DEBUG` 빌드에서만 시작 시점에 인라인 CRUD/인증 스모크 테스트를 돌린다. 이 경로가 저장소에서 테스트에 가장 가까운 것이다 — 자세한 내용은 [MiniGameDB/CLAUDE.md](MiniGameDB/CLAUDE.md).

## 프로토콜 코드 생성 — 가장 중요한 작업 흐름

`.proto` 파일은 **오직** `bin/S2C_PBFiles/`와 `bin/S2D_PBFiles/`에만 있다. `MiniGameServer/Server/`, `MiniGameDB/`, `MiniGameClient/Assets/Scripts/Network/`에 있는 생성 파일들은 바이트 단위로 동일한 **복사본**이다.

패킷을 변경하려면:

1. `bin/` 아래의 `.proto`를 수정한다.
2. 해당 `GenPacket.bat`을 실행한다 (vcpkg의 `protoc.exe`, S2D는 추가로 `grpc_cpp_plugin.exe` 사용).
   단, `GenPacket.bat`은 `--cpp_out`과 `--csharp_out`을 **함께** 돌린다. 버전 불일치 복구처럼 C++만 갱신하고 싶을 때는 배치 파일 대신 `--cpp_out`만 넣어 `protoc`를 직접 호출할 것.
3. **산출물을 직접 복사해 넣는다** — 자동화된 것이 전혀 없다:
   - `S2C_Protocol*.pb.{h,cc}` → `MiniGameServer/Server/`
   - `S2CProtocol*.cs` → `MiniGameClient/Assets/Scripts/Network/`
   - `S2D_Protocol.{grpc.,}pb.{h,cc}` → `MiniGameServer/Server/`와 `MiniGameDB/` **양쪽 모두**

클라이언트↔서버 패킷을 하나 추가하면, 코드 생성이 동기화해주지 **않는** 5군데를 손대야 한다:

- `bin/S2C_PBFiles/S2C_Protocol_Common.proto` — `MsgId` proto enum (C# 클라이언트가 쓰는 것).
- `MiniGameServer/Server/S2CPacketHandler.h` — 위 enum을 **손으로 복제해둔** 것 (`PKT_C_*` / `PKT_S_*`). 두 값이 숫자상 완전히 일치해야 한다.
- `S2CPacketHandler::Init()` — C_ 패킷은 `GPacketHandler`(원본/비암호 경로)와 `PlaintextHandler`(복호화 후 경로) **둘 다** 등록. S_ 패킷은 `MakeSendBufferRef` 오버로드 추가(평문용, AES 키용 두 가지).
- `S2CPacketHandler::Init()` — 그 패킷을 보낼 수 있는 세션 상태에 대해 `GAllowedPacketIdsPerSecureLevel`에서 새 id를 허용. **빠뜨리면 패킷이 조용히 버려진다.**
- `MiniGameClient/Assets/Scripts/Network/ClientPacketManager.cs` — `_onRecv`, `_handler`, `_msgFactories` (`S_Encrypted` 안에 실려오는 패킷이라면 factory가 반드시 필요하다).

`.proto` 파일들에는 설계의 근거(핸드셰이크, 보안 레벨, 로그인/계정 생성 트랜잭션)가 한글 주석으로 상세히 적혀 있다. 프로토콜 동작을 바꾸기 전에 먼저 읽을 것.

## 컨벤션 및 주의사항

### 인코딩

모든 `.cpp`/`.h`/`.cs` 파일은 UTF-8이 아니라 **CP949**(한국어 ANSI)다. `.proto`와 `.md`만 UTF-8이다. UTF-8로 읽으면 주석이 깨져 보이는데 이는 정상이며 파일이 손상된 것이 아니다.

**CP949 소스 파일을 Edit/Write 도구로 수정하지 말 것.** 두 도구는 파일을 UTF-8로 재작성하는데, 이때 CP949 바이트가 UTF-8로 디코딩되지 못해 전부 `U+FFFD`(`ef bf bd`)로 바뀐다. **한글 주석이 복구 불가능하게 소실된다.** 편집한 줄이 ASCII만 담고 있어도 파일 전체가 당한다.

대신 `sed -i` 를 쓸 것. sed는 바이트 단위로 동작해서 매칭된 ASCII 부분만 바꾸고 나머지 바이트는 건드리지 않는다. 삽입하는 주석도 ASCII로만 쓰고, CRLF 유지를 위해 줄바꿈은 `\r\n` 으로 넣는다.

```bash
sed -i 's|기존 ASCII 코드|새 ASCII 코드|' Foo.cs
```

수정 후 반드시 검증할 것. `file Foo.cs` 가 여전히 `ISO-8859 text` 여야 한다(`UTF-8 text` 로 바뀌었으면 이미 손상된 것). 한글 줄의 바이트가 `c0 cc` 같은 CP949 이중바이트로 남아 있는지 `sed -n '45p' Foo.cs | od -An -tx1` 로 확인하고, `git diff --numstat` 으로 의도한 줄 수만 바뀌었는지 본다.

이미 손상시켰다면 `git checkout -- <파일>` 로 되돌린 뒤 sed로 다시 적용한다.

### 네이밍

- 패킷은 **보내는 쪽 기준**으로 `S_*`(서버→클라), `C_*`(클라→서버)다.
- 게임별 패킷은 방향 뒤에 글자 접두사가 붙는다: `_R_` Race, `_P_` PingPong, `_M_` Mole.
- 패킷 id 블록: 0–99 로비/공통, 100+ Race, 200+ PingPong, 300+ Danmaku, 400+ Mole.
- 참조: `xxxRef` = `shared_ptr`, `xxxWRef` = `weak_ptr`, `G*` = 전역, `L*` = `thread_local`.
- 룸과 큐 안에서 세션은 `weak_ptr`로 들고 있으므로, 항상 `.lock()` 후 `PlayerSession::IsInvalidPlayerSession`으로 검사할 것.

### 커밋하면 안 되는 것

`.env`, `*.key`, `*.crt`, `*.csr`, `GitIgnores.cs`는 절대 커밋하지 말 것. 모두 gitignore 대상이며, 특히 마지막 파일이 로컬에 없으면 클라이언트가 접속하지 못한다.

### 잔재

`Danmaku` 프로토콜 메시지는 존재하지만 게임은 구현되어 있지 않다. `TestScene`/`TestGame*`/`gRPC_test`도 잔재다.
