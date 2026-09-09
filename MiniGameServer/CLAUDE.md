# MiniGameServer — 게임 서버

C++17 / Windows IOCP 게임 서버. `0.0.0.0:7777` 리슨.

저장소 전역 규칙(프로토콜 코드 생성, 네이밍)은 루트 [`../CLAUDE.md`](../CLAUDE.md)를 먼저 볼 것. 이 문서는 서버 내부 구조만 다룬다.

**protobuf `string` 필드에 넣을 한글은 `u8"..."`로 쓸 것.** 실행 문자셋이 ANSI라 일반 리터럴은 CP949 바이트로 나가고, UTF-8로 디코드하는 클라이언트에서 깨진다.

## 솔루션 구성

`Server.sln`은 두 프로젝트로 되어 있다.

| 프로젝트 | 디렉토리 | 종류 | 내용 |
|---|---|---|---|
| `Libraries` | `Libraries/` | StaticLibrary | 직접 만든 IOCP 네트워크 코어 |
| `Server` | `Server/` | Application | 그 위에 올린 게임 로직 |

**이 분리가 중요하다** — `Libraries`는 protobuf도 게임도 모르고, `Server`는 overlapped I/O를 모른다. 새 코드를 어디에 둘지 헷갈리면 이 기준으로 판단할 것.

```powershell
msbuild MiniGameServer\Server.sln /p:Configuration=Debug /p:Platform=x64
```

산출물은 `Server/BuildFiles/$(Configuration)/`에 생성된다. 의존성(protobuf, gRPC, OpenSSL)은 vcpkg(`C:\vcpkg\installed\x64-windows`)에서 자동 링크로 들어오므로 `.vcxproj`의 `AdditionalDependencies`가 거의 비어 있다. `props/*.props`는 빈 스텁이라 아무 역할도 하지 않는다.

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

vcpkg 업데이트로 깨지는 두 증상(`C1189` protobuf gencode 버전, `C4996` abseil `MutexLock`)은 DB 게이트웨이와 공통이라 루트 CLAUDE.md에 정리해두었다.

## 실행 전제 조건

작업 디렉토리에 `.env`가 있어야 한다(gitignore 대상).

| 키 | 용도 |
|---|---|
| `DB_ADDRESS` | DB 게이트웨이 주소 |
| `CERTNAME` | DB 서버 인증서 경로 |
| `CNNAME` | gRPC SSL target-name override |

`ServerGlobal.cpp`의 `EnvManager`가 읽는다. 서버는 시작 시 `HelloAsync()`로 DB를 호출하므로 **`MiniGameDB`를 먼저 띄워야 한다.**

## 스레드 구성

`Server.cpp::main`에서 만든다. AWS t3.micro 기준으로 맞춰져 있다.

| 스레드 | 개수 | 하는 일 |
|---|---|---|
| 워커 | 2 | `DoGlobalQueueWork()` → `DBManager->AsyncCompleteRpc()` → `CPCore::Dispatch(10)` 반복 |
| 타이머 | 1 | 20ms마다 `DoTimerQueueDistribution()` |
| 매치메이킹 | 1 | 20ms마다 `GGameManagers[1..3]`에 `MatchMake()` → `RenewMatchQueue()` → `RemoveInvalidRoom()` → `Update()` |

액터 작업, gRPC 완료 처리, IOCP 완료 처리가 모두 워커 스레드를 공유한다. 매치메이킹만 별도 스레드인 것은 `SearchMatchGroups()`가 느려서 워커를 붙잡지 않도록 **의도적으로 분리**한 것이다.

## 액터 모델

`Libraries/Actor.h`, `Actor.cpp`, `GlobalActorQueue`, `ActorEventScheduler`.

`GameRoom`은 `Actor`를 상속한다. **룸 상태 변경은 직접 호출이 아니라 반드시 `DispatchEvent` / `PostEvent` / `PostEventAfter`를 거쳐야 하며**, 이로써 각 룸이 한 번에 한 스레드에서만 처리되도록 직렬화된다.

- `Actor::Execute`는 `LEndTickCount`(워커 루프 매 반복 시작 시 now+64ms로 설정)를 넘기면 전역 큐로 양보한다. 특정 룸이 다른 룸을 굶기지 못한다.
- `LCurrentActor`는 이미 어떤 액터 안에 들어와 있는 스레드가 다른 액터로 재진입하는 것을 막는다.

매치메이킹 스레드도 이 규칙을 지킨다. `RaceManager::Update()`는 `room->Update()`를 직접 부르지 않고 `roomRef->PostEvent(&GameRoom::Update)`를 한다.

### objectPool 규칙 (`Libraries/objectPool.h`)

Windows SLIST 기반 락프리 풀이다. `dealloc()`은 **소멸자를 먼저 돌리고 그다음 `pushEntry()`로 블록을 공개**해야 한다. 순서를 뒤집으면 다른 스레드가 그 블록을 pop해 placement-new 하는 도중 소멸자가 도는 경합이 생긴다. `~poolHeader()`는 이미 파괴된 엔트리를 재소멸시키지 않고, `_aligned_malloc`이 돌려준 기저 주소(`pSE`, 오프셋 금지)만 `_aligned_free`에 넘긴다.

## 세션 상태 머신과 패킷 검증

`PlayerSession::SessionState`:

```
BeforeHandShake → BeforeLogin → Lobby → { Race | PingPong | Mole }
```

`GAllowedPacketIdsPerSecureLevel[state][msgId]`(`S2CPacketHandler.h`)가 들어오는 모든 패킷을 상태 기준으로 검사한다. **두 번** 확인한다.

1. `S2CPacketHandler::HandlePacket` — 수신 즉시
2. `Handle_C_Encrypted` 내부 — 복호화 이후

거부된 패킷은 `_suspiciousStack`을 증가시키고, 5회 누적되면 세션을 끊는다. 새 패킷을 추가하고 여기에 등록하지 않으면 **패킷이 조용히 버려진다.**

## 암호화 핸드셰이크

1. 접속 시 서버가 `CryptoManager`(`ServerGlobal.h`) 풀에서 RSA 키를 꺼내 공개키를 `S_Welcome`으로 보낸다.
2. 클라이언트가 AES-256 키를 만들어 RSA로 암호화해 `C_Welcome`으로 되돌린다.
3. 이후 대부분의 트래픽은 `C_Encrypted`/`S_Encrypted`로 감싸진다. AES-256-GCM이고, `msgId`를 AAD로 함께 넣어 tag와 무결성 검증에 활용한다.

**순서에 주의**: protobuf 직렬화를 **먼저** 하고 그다음 암호화한다. 덕분에 동일한 `Handle_*` 함수가 평문 경로와 복호화 경로 양쪽을 모두 처리할 수 있다.

## 게임별 구조

세 게임 모두 같은 3종 세트를 따른다. 게임을 추가한다는 것은 이 셋 + `GameType` 항목 + 패킷 id 블록을 추가한다는 뜻이다.

| 역할 | Race | PingPong | Mole |
|---|---|---|---|
| `GameManager` 파생 | `RaceManager` | `PingPongManager` | `MoleManager` |
| `GameRoom` 파생 (`Actor`) | `RaceRoom` | `PingPongGameRoom` | `MoleRoom` |
| `UnityGameObject` 파생 | `RacePlayer` | `PingPongGameBullet` | (슬롯) |

- `GameManager` 파생은 `MatchQueue`, 룸 생명주기, 캐시된 최고 기록을 소유한다. `GameType` int를 키로 `GGameManagers`에 등록한다 (**1=Race, 2=PingPong, 3=Mole**).
- `GameRoom` 상태 머신: `BeforeInit → BeforeStart → OnGoing → Counting → EndGame`.
- `UnityGameObject` 파생은 클라이언트가 스폰/디스폰/보간해야 하는 모든 것을 표현한다.

### 오브젝트 id 규약 (Race)

`RaceRoom`에서 **러너 i의 오브젝트 id는 플레이어 인덱스 i와 같다.** 클라이언트가 `S_R_ResponseState.playerid`를 오브젝트 id와 대조하기 때문이다. 그래서 `RaceRoom::QUOTA`(=2)를 상수로 두고, 러너 id는 `Start()`에서 손으로 배정하며, `_nxtObjectId`는 `QUOTA`부터 시작해 나머지 오브젝트가 러너 id를 침범하지 않게 한다. `PingPongGameRoom`처럼 이 규약이 없는 룸은 그냥 `GenerateUniqueGameObjectId()`를 쓴다.

## 매치메이킹

`MatchQueue`(`MatchQueue.h`)는 `_allowDevi`(50) 이내의 Elo 차이로 매칭한다. 입력 큐와 탐색 큐를 분리해 새 요청 입력과 후보 탐색이 직접 경쟁하지 않도록 했다.

기록은 매니저의 인프로세스 캐시(`_publicRecord`)에 두고 시작 시 DB에서 갱신한다. `Server.cpp` 주석대로 원래 Redis로 했어야 할 부분을 대신하고 있다.

룸과 큐 안에서 세션은 `weak_ptr`로 들고 있다. **항상 `.lock()` 후 `PlayerSession::IsInvalidPlayerSession`으로 검사할 것.**

## DB 접근

fire-and-forget 비동기 gRPC다. **게임 경로 어디에도 블로킹 DB 호출은 없다.**

```
DBClientImpl::S2D_*  →  CompletionQueue  →  (워커 루프) AsyncCompleteRpc()  →  S2D_CallData
```

`DBClientImpl`(`ServerGlobal.h`에 전역 `DBManager`로 노출)이 요청을 큐에 넣고, 워커 루프에서 호출되는 `AsyncCompleteRpc()`가 완료를 소진하며 `S2D_CallData`를 통해 이어받는다. 프로토콜 정의는 `bin/S2D_PBFiles/S2D_Protocol.proto`이고 게이트웨이 쪽 구현은 [`../MiniGameDB/CLAUDE.md`](../MiniGameDB/CLAUDE.md) 참고.

## 파일 지도

### `Libraries/` — 네트워크 코어

| 파일 | 내용 |
|---|---|
| `CompletionPortCore.*` | IOCP 래퍼, `Dispatch()` |
| `Session.*` | 소켓 연결/비동기 Recv·Send. `PBSession`이 패킷 헤더 해석 |
| `Listener.*` | `AcceptEx` 기반 연결 수락 |
| `Service.*` | `ServerService` / 클라이언트 서비스 |
| `RecvBuffer.*`, `SendBuffer.*` | 링버퍼 / 청크 할당 |
| `Actor.*`, `ActorEvent*.*`, `GlobalActorQueue.*` | 액터 모델 |
| `objectPool.*`, `StompAllocator.*` | SLIST 풀, 디버그 할당자 |
| `MPSCQueue.*`, `RWLock.*` | 동기화 프리미티브 |
| `NetAddress.*`, `SocketUtils.*` | 소켓 유틸 |

### `Server/` — 게임 로직

| 파일 | 내용 |
|---|---|
| `Server.cpp` | `main`, 스레드 생성 |
| `ServerGlobal.*` | `GCryptoManager`, `GEnvManager`, `DBManager` 등 전역 |
| `PlayerSession.*` | 세션 상태 머신 |
| `S2CPacketHandler.*` | 패킷 디스패치, 상태별 허용 id 테이블 |
| `S2CPacketMaker.*` | 송신 버퍼 생성(평문/AES) |
| `S2CServerServiceImpl.*` | 서비스 구현 |
| `GameManager.*`, `GameRoom.*`, `GameType.*` | 게임 공통 베이스 |
| `MatchQueue.*`, `Deviset.*`, `WatingPlayerData.*` | 매치메이킹 |
| `RaceManager/Room/Player.*` | Race |
| `PingPongManager/GameRoom/GameBullet.*` | PingPong |
| `MoleManager.*`, `MoleRoom.*` | 호박쪼개기 |
| `UnityGameObject.*`, `XYZ.*` | 동기화 대상 객체 / 좌표 |
| `DBClientImpl.*`, `S2DPacketMaker.*`, `S2D_CallData.*` | DB 게이트웨이 클라이언트 |
| `S2C_Protocol*.pb.*`, `S2D_Protocol*.pb.*` | **생성 파일 — 직접 고치지 말 것** |

`TestGameBullet.*`, `gRPC_test.*`는 잔재다. `Danmaku` 프로토콜 메시지는 존재하지만 게임은 구현되어 있지 않다.
