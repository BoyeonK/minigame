# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 저장소 구성

하나의 git 저장소 안에 3개의 독립된 프로그램이 있고, 2개의 생성된(generated) 프로토콜로 연결되어 있다.

| 디렉토리 | 내용 | 빌드 방법 |
|---|---|---|
| `MiniGameServer/` | 게임 서버 (C++17, Windows/IOCP). `Server.sln` = `Server`(exe) + `Libraries`(정적 라이브러리) | Visual Studio / MSBuild, x64 |
| `MiniGameDB/` | DB 게이트웨이 서버 (C++17). ODBC로 SQL Server에 붙는 gRPC 서버 | `MiniGameDB.sln`, x64 |
| `MiniGameClient/` | Unity 6000.4.2f1 클라이언트 (C#) | Unity 에디터 |
| `bin/` | `.proto` 원본 + `protoc` 산출물. 빌드 디렉토리가 아니라 **프로토콜 코드 생성이 이루어지는 곳** | `GenPacket.bat` |

`server/` (소문자, git 미추적)는 빈 VS 스캐폴드이며 프로젝트의 일부가 아니다.

## 빌드 및 실행

**서버 / DB** — MSBuild, x64 전용 (.sln에 `Any CPU`/`Win32` 구성도 있지만 실제 타겟이 아니다):

```powershell
msbuild MiniGameServer\Server.sln /p:Configuration=Debug /p:Platform=x64
msbuild MiniGameDB\MiniGameDB.sln /p:Configuration=Debug /p:Platform=x64
```

의존성(protobuf, gRPC, OpenSSL)은 **vcpkg**(`C:\vcpkg\installed\x64-windows`)에서 자동 링크로 들어온다. 그래서 `.vcxproj`의 `AdditionalDependencies`가 거의 비어 있다. `Server` 산출물은 `Server/BuildFiles/$(Configuration)/`에 생성된다.

현재 검증된 조합: Visual Studio 2026 Community(v18) + PlatformToolset **v145**, vcpkg의 protobuf **6.33.4** / grpc **1.76.0** / abseil **20260107.1** / openssl 3.6.2.

### x64가 아닌 플랫폼으로 빌드하면 안 된다

`.sln`에 `Any CPU`/`x86` 구성이 남아 있지만 **둘 다 프로젝트의 `Win32` 구성으로 매핑되고, Win32는 동작하지 않는다.** Visual Studio에서 빌드한다면 솔루션 플랫폼이 `x64`인지 반드시 확인할 것.

```
Debug|Any CPU  →  Debug|Win32   (실패)
Debug|x86      →  Debug|Win32   (실패)
Debug|x64      →  Debug|x64     (정상)
```

Win32로 빌드하면 나는 증상: **`LNK2019: unresolved external symbol _main` + `LNK1120`**.
원인은 `Libraries/Library.vcxproj`의 `Win32` 구성만 `ConfigurationType`이 `StaticLibrary`가 아니라 **`Application`**으로 남아 있어서다(x64 구성만 StaticLibrary로 고쳐진 상태). 링커가 `main()`이 없는 라이브러리를 EXE로 만들려다 실패한다. `_main`의 선행 언더스코어가 32비트 데코레이션이라 이 증상만 봐도 Win32 빌드임을 알 수 있다.

Win32 구성을 `StaticLibrary`로 고쳐도 32비트 빌드는 여전히 불가능하다. vcpkg에 `x64-windows` 트리플릿만 설치되어 있어 그다음엔 protobuf/grpc/openssl 심볼이 전부 미해결로 남는다. 이 프로젝트는 x64 전용이다.

### 빌드가 깨질 때 — vcpkg 업데이트가 원인인 두 가지

`vcpkg upgrade` 후 아래 두 증상이 같이 나타난다. 둘 다 코드 잘못이 아니라 버전 어긋남이다.

1. **`error C1189: Protobuf C++ gencode is built with an incompatible version`** (`*.pb.h` 16번째 줄 근처)
   커밋된 생성 코드와 설치된 protobuf 런타임의 버전이 다르다. `.pb.h` 상단의 `Protobuf C++ Version:` 주석과 `protoc --version`을 비교해보면 바로 확인된다.
   → 아래 "프로토콜 코드 생성" 절차대로 **C++ gencode만** 재생성한다. `--csharp_out`은 돌리지 말 것 — 클라이언트의 `.cs`는 Unity가 들고 있는 `Google.Protobuf.dll` 버전에 묶여 있고, C++/C# gencode는 서로 독립적이라(와이어 포맷은 호환) 한쪽만 올려도 된다.

2. **`error C4996: absl::MutexLock::MutexLock ... Use the constructor that takes a reference instead`** (`grpcpp/completion_queue.h`, `grpcpp/support/server_callback.h`)
   abseil이 포인터를 받는 `MutexLock` 생성자를 deprecated 처리했는데 grpc 헤더가 아직 그것을 쓴다. 우리 코드가 아니라 **서드파티 헤더**의 문제이고, `<SDLCheck>true</SDLCheck>`(`/sdl`)가 C4996을 에러로 승격시키기 때문에 빌드가 멈춘다.
   → `Server.vcxproj`와 `MiniGameDB.vcxproj`의 x64 Debug/Release `ClCompile`에 `<AdditionalOptions>/w34996 %(AdditionalOptions)</AdditionalOptions>`가 들어 있어 다시 경고로 되돌린다. `/sdl`의 나머지 검사는 그대로 유지된다. **`AdditionalOptions`여야 한다** — `/sdl` 뒤에 붙어야 이기기 때문이다. 같은 이유로 `CL` 환경변수(앞에 붙음)로는 안 되고 `_CL_`(뒤에 붙음)이어야 한다.

테스트 스위트는 없다. `MiniGameDB`가 `_DEBUG` 빌드에서만 시작 시점에 인라인 CRUD/인증 스모크 테스트를 돌린다 (`GlobalVariables.cpp`의 `DBManager::InitialC/R/U/D`, `AkagiRedSunsNo2`, `ScandinavianFlick`). 이 생성자 경로가 테스트에 가장 가까운 것이다.

**실행 전제 조건** (모두 gitignore 대상이며, 각 exe의 작업 디렉토리에 존재해야 한다):

- `MiniGameServer`: `DB_ADDRESS`, `CERTNAME`(DB 서버 인증서 경로), `CNNAME`(SSL target-name override)이 담긴 `.env`. `ServerGlobal.cpp`의 `EnvManager`가 읽는다. `0.0.0.0:7777` 리슨.
- `MiniGameDB`: `DB_SERVER`, `DB_NAME`, `CONNECTION`이 담긴 `.env`(프로세스 환경변수로 밀어넣은 뒤 다시 읽어서 ODBC Driver 17 연결 문자열을 만든다) + gRPC SSL용 `server.key` / `server.crt`. `0.0.0.0:50051` 리슨.
- 클라이언트: `Assets/Scripts/Utils/GitIgnores.cs`가 `GitIgnores.sAddr`(서버 IP)를 제공한다. 현재 `NetworkManager.TryConnectToServer`는 `IPAddress.Loopback`으로 하드코딩되어 있고 `GitIgnores.sAddr` 줄은 주석 처리되어 있다. 원격 테스트 시 되돌려야 한다.

실행 순서: `MiniGameDB` 먼저(서버가 시작 시 `HelloAsync()`로 DB를 호출한다), 그다음 `Server`, 마지막에 Unity 클라이언트.

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

## 서버 아키텍처 (`MiniGameServer/`)

`Libraries/`는 직접 만든 IOCP 네트워크 코어이고, `Server/`는 그 위에 올린 게임 로직이다. 이 분리가 중요하다 — `Libraries`는 protobuf도 게임도 모르고, `Server`는 overlapped I/O를 모른다.

**스레드 구성** (`Server.cpp::main`, AWS t3.micro 기준으로 맞춰져 있음):

- 워커 스레드 2개. 각각 `DoGlobalQueueWork()` → `DBManager->AsyncCompleteRpc()` → `CPCore::Dispatch(10)`을 반복한다. 즉 액터 작업, gRPC 완료 처리, IOCP 완료 처리가 모두 같은 스레드를 공유한다.
- 타이머 스레드 1개. 20ms마다 `DoTimerQueueDistribution()`.
- 매치메이킹 스레드 1개. 20ms마다 `GGameManagers[1..3]`에 대해 `MatchMake()` → `RenewMatchQueue()` → `RemoveInvalidRoom()` → `Update()`. `SearchMatchGroups()`가 느리기 때문에 매치메이킹을 워커 스레드에서 **의도적으로 분리**한 것이다.

**액터 모델** (`Libraries/Actor.h`, `Actor.cpp`, `GlobalActorQueue`, `ActorEventScheduler`): `GameRoom`은 `Actor`를 상속한다. 룸 상태 변경은 직접 호출이 아니라 반드시 `DispatchEvent` / `PostEvent` / `PostEventAfter`를 거쳐야 하며, 이로써 각 룸이 한 번에 한 스레드에서만 처리되도록 직렬화된다. `Actor::Execute`는 `LEndTickCount`(워커 루프 매 반복 시작 시 now+64ms로 설정)를 넘기면 전역 큐로 양보하므로, 특정 룸이 다른 룸을 굶기지 못한다. `LCurrentActor`는 이미 어떤 액터 안에 들어와 있는 스레드가 다른 액터로 재진입하는 것을 막는다.

매치메이킹 스레드도 이 규칙을 지킨다. `RaceManager::Update()`는 `room->Update()`를 직접 부르지 않고 `roomRef->PostEvent(&GameRoom::Update)`를 한다.

**세션 상태 머신** (`PlayerSession::SessionState`): `BeforeHandShake → BeforeLogin → Lobby → {Race | PingPong | Mole}`. `GAllowedPacketIdsPerSecureLevel[state][msgId]`가 들어오는 모든 패킷을 상태 기준으로 검사하며, `S2CPacketHandler::HandlePacket`과 복호화 이후 `Handle_C_Encrypted` 내부에서 **두 번** 확인한다. 거부된 패킷은 `_suspiciousStack`을 증가시키고, 5회 누적되면 세션을 끊는다.

**암호화 핸드셰이크**: 접속 시 서버가 `CryptoManager` 풀에서 RSA 키를 꺼내 공개키를 `S_Welcome`으로 보낸다. 클라이언트는 AES-256 키를 만들어 RSA로 암호화한 뒤 `C_Welcome`으로 되돌려준다. 이후 대부분의 트래픽은 `C_Encrypted`/`S_Encrypted`로 감싸진다(AES-256-GCM, `msgId`를 AAD로 함께 넣어 tag와 함께 무결성 검증에 활용). 순서에 주의: protobuf 직렬화를 **먼저** 하고 그다음 암호화한다 — 덕분에 동일한 `Handle_*` 함수가 평문 경로와 복호화 경로 양쪽을 모두 처리할 수 있다.

**게임별 구조** — 세 게임 모두 같은 3종 세트를 따른다. 게임을 추가한다는 것은 이 셋 + `GameType` 항목 + 패킷 id 블록을 추가한다는 뜻이다:

- `GameManager` 파생 클래스 (`RaceManager`, `PingPongManager`, `MoleManager`) — `MatchQueue`, 룸 생명주기, 캐시된 최고 기록을 소유. `GameType` int를 키로 `GGameManagers`에 등록 (1=Race, 2=PingPong, 3=Mole).
- `GameRoom` 파생 클래스 (`RaceRoom`, `PingPongGameRoom`, `MoleRoom`) — `Actor`이며, 상태 머신은 `BeforeInit → BeforeStart → OnGoing → Counting → EndGame`.
- 클라이언트가 스폰/디스폰/보간해야 하는 모든 것에 대한 `UnityGameObject` 파생 클래스.

`MatchQueue`는 `_allowDevi`(50) 이내의 Elo 차이로 매칭한다. 기록은 매니저의 인프로세스 캐시(`_publicRecord`)에 두고 시작 시 DB에서 갱신한다 — `Server.cpp` 주석대로 원래 Redis로 했어야 할 부분을 대신하고 있다.

**DB 접근**은 fire-and-forget 비동기 gRPC다. `DBClientImpl::S2D_*`가 `CompletionQueue`에 넣고, 워커 루프에서 호출되는 `AsyncCompleteRpc()`가 완료를 소진하며 `S2D_CallData`를 통해 이어받는다. 게임 경로 어디에도 블로킹 DB 호출은 없다.

## DB 게이트웨이 (`MiniGameDB/`)

비동기 gRPC 서버(진행 중인 RPC마다 `CallData`, `HandleRpcs()`를 도는 스레드 4개)이며 ODBC Driver 17으로 SQL Server에 붙는다. `DBManager`가 `SQLHDBC` 핸들을 풀링하고(`PopHDbc`/`ReturnHDbc`), 핸들 반납과 트랜잭션 롤백에 `Cleaner` RAII 가드를 쓴다. 에러 분기마다 수동으로 정리하지 말고 이 패턴을 따를 것.

비밀번호는 계정별 OpenSSL `RAND_bytes` salt를 사용한 PBKDF2-HMAC-SHA256이다. 해시와 salt는 16진 문자열로 `NVARCHAR` 컬럼에 저장한다(`v2wsRef`/`ws2vRef`가 변환 담당). 테이블: `Players`(player_id → dbid), `Accounts`, `Elos`, `PersonalRecords`. 인코딩 헬퍼는 서로 다르며 혼용하면 안 된다 — `a2wsRef`는 CP949→UTF-16, `s2wsRef`는 UTF-8→UTF-16이다.

## Unity 클라이언트 (`MiniGameClient/`)

`Managers`는 모든 서브시스템(`Network`, `Scene`, `UI`, `Resource`, `Pool`, `Sound`, `Object`, `Input`, `Setting`)을 소유하는 `DontDestroyOnLoad` 싱글톤이다.

**스레드 규칙**: 패킷 핸들러는 Unity 메인 스레드가 아니라 소켓 스레드에서 돈다. `GameObject`, UI, 씬 로딩을 건드리는 것은 모두 `Managers.ExecuteAtMainThread(...)`로 감싸야 하고, `Managers.Update()`가 매 프레임 이 큐를 소진한다. `NetworkManager` 전체가 이 패턴으로 되어 있으니 그대로 따를 것.

`NetworkManager`는 도메인별 중첩 매니저(`Lobby`, `Match`, `Loading`, `PingPong`, `Mole`, `Race`)로 쪼개져 있고, 각각 `NetworkManager.Init()`에서 초기화된다. 핸들러는 `Managers.Scene.CurrentScene`을 구체 `BaseScene` 파생 타입(`RaceScene`, `PingPongScene`, `MoleScene`)으로 패턴 매칭해서 게임플레이에 접근한다. 씬이 null이거나 타입이 안 맞으면 그냥 early-return 하는 것이 정상 동작이며 에러가 아니다.

`Assets/Scripts/Network/`(`Session.cs`, `Connector.cs`, `RecvBuffer.cs`, `ServerSession.cs`)는 C++ `Libraries` 코어를 그대로 옮긴 것이며, 4바이트 `PacketHeader`(`ushort size`, `ushort id`)도 동일하다.

**씬은 반드시 Build Settings에 등록되어 있어야 한다.** `SceneManagerEx.GetSceneName()`이 `Enum.GetName(typeof(Define.Scene), type)`으로 **이름 문자열**을 만들어 `SceneManager.LoadScene(name)`에 넘기기 때문이다. 등록되지 않은 씬은 에디터 플레이에서도 로드에 실패하며, 플레이어 빌드에는 아예 포함되지 않는다. 실제로 로드되는 씬은 6개다.

| # | 경로 | 용도 |
|---|---|---|
| 0 | `Assets/Scenes/Login.unity` | 시작 씬. 로비 UI도 여기서 담당 |
| 1 | `Assets/Scenes/LoadingScenes/LoadingScene1.unity` | 게임 진입 로딩 |
| 2 | `Assets/Scenes/LoadingScenes/GameResult.unity` | 게임 종료 후 결과 |
| 3 | `Assets/Scenes/Race.unity` | |
| 4 | `Assets/Scenes/PingPong.unity` | |
| 5 | `Assets/Scenes/Mole.unity` | |

`Define.Scene`의 `Lobby`, `TestLoadingScene`, `Undefined`는 `LoadScene`에 전달되는 곳이 없다(`Lobby`는 `Login` 씬이 겸하고, `TestLoadingScene`은 잔재). `Assets/Scenes/TestScene.unity`도 마찬가지로 잔재다. 등록할 필요 없다.

**Unity 에디터가 열려 있는 동안 `ProjectSettings/*.asset`을 디스크에서 직접 편집하지 말 것.** 에디터가 해당 설정을 메모리에 들고 있다가 종료 시 덮어써서 수정이 사라진다. 에디터를 닫고 편집하거나, 에디터 UI/`EditorBuildSettings` API로 바꿔야 한다.

클라이언트의 `Define.GameType`에는 서버 `GameType`에 없는 `InProcess = 5`가 있다. 클라이언트 전용 매치메이킹 상태이므로 불일치를 "고치려" 하지 말 것.

## 컨벤션 및 주의사항

- **인코딩**: 모든 `.cpp`/`.h`/`.cs` 파일은 UTF-8이 아니라 **CP949**(한국어 ANSI)다. `.proto`와 `CLAUDE.md`만 UTF-8이다. UTF-8로 읽으면 주석이 깨져 보이는데 이는 정상이며 파일이 손상된 것이 아니다.

  **CP949 소스 파일을 Edit/Write 도구로 수정하지 말 것.** 두 도구는 파일을 UTF-8로 재작성하는데, 이때 CP949 바이트가 UTF-8로 디코딩되지 못해 전부 `U+FFFD`(`ef bf bd`)로 바뀐다. **한글 주석이 복구 불가능하게 소실된다.** 편집한 줄이 ASCII만 담고 있어도 파일 전체가 당한다.

  대신 `sed -i` 를 쓸 것. sed는 바이트 단위로 동작해서 매칭된 ASCII 부분만 바꾸고 나머지 바이트는 건드리지 않는다. 삽입하는 주석도 ASCII로만 쓰고, CRLF 유지를 위해 줄바꿈은 `\r\n` 으로 넣는다.

  ```bash
  sed -i 's|기존 ASCII 코드|새 ASCII 코드|' Foo.cs
  ```

  수정 후 반드시 검증할 것. `file Foo.cs` 가 여전히 `ISO-8859 text` 여야 한다(`UTF-8 text` 로 바뀌었으면 이미 손상된 것). 한글 줄의 바이트가 `c0 cc` 같은 CP949 이중바이트로 남아 있는지 `sed -n '45p' Foo.cs | od -An -tx1` 로 확인하고, `git diff --numstat` 으로 의도한 줄 수만 바뀌었는지 본다.

  이미 손상시켰다면 `git checkout -- <파일>` 로 되돌린 뒤 sed로 다시 적용한다.
- **네이밍**: 서버 패킷은 **보내는 쪽 기준**으로 `S_*`(서버→클라), `C_*`(클라→서버)다. 게임별 패킷은 방향 뒤에 글자 접두사가 붙는다: `_R_` Race, `_P_` PingPong, `_M_` Mole. 패킷 id 블록은 0–99 로비/공통, 100+ Race, 200+ PingPong, 300+ Danmaku, 400+ Mole.
- **참조 네이밍**: `xxxRef` = `shared_ptr`, `xxxWRef` = `weak_ptr`, `G*` = 전역, `L*` = `thread_local`. 룸과 큐 안에서 세션은 `weak_ptr`로 들고 있으므로, 항상 `.lock()` 후 `PlayerSession::IsInvalidPlayerSession`으로 검사할 것.
- `Danmaku` 프로토콜 메시지는 존재하지만 게임은 구현되어 있지 않다. `TestScene`/`TestGame*`/`gRPC_test`는 잔재다.
- `.env`, `*.key`, `*.crt`, `*.csr`, `GitIgnores.cs`는 절대 커밋하지 말 것. 모두 gitignore 대상이며, 특히 마지막 파일이 로컬에 없으면 클라이언트가 접속하지 못한다.
