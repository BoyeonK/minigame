# MiniGameDB — DB 게이트웨이

C++17 비동기 gRPC 서버. 게임 서버와 SQL Server 사이에 서 있다. `0.0.0.0:50051` 리슨(SSL).

저장소 전역 규칙(프로토콜 코드 생성, 네이밍)은 루트 [`../CLAUDE.md`](../CLAUDE.md)를 먼저 볼 것.

## 왜 분리되어 있나

게임 서버의 워커 스레드가 블로킹 DB I/O를 직접 수행하거나 응답을 기다리지 않게 하려고 DB 접근을 별도 프로세스로 빼냈다. 게임 서버는 gRPC `CompletionQueue`로 요청만 던지고 결과는 나중에 받는다(서버 쪽 구현은 [`../MiniGameServer/CLAUDE.md`](../MiniGameServer/CLAUDE.md)의 "DB 접근").

```
Game Server ──Async gRPC──▶ MiniGameDB ──ODBC Driver 17──▶ SQL Server
```

## 빌드

```powershell
msbuild MiniGameDB\MiniGameDB.sln /p:Configuration=Debug /p:Platform=x64
```

x64 전용이다. 의존성(protobuf, gRPC, OpenSSL)은 vcpkg(`C:\vcpkg\installed\x64-windows`)에서 자동 링크로 들어온다. vcpkg 업데이트로 깨지는 두 증상(`C1189` protobuf gencode 버전, `C4996` abseil `MutexLock`)은 게임 서버와 공통이라 루트 CLAUDE.md에 정리해두었다. ODBC는 시스템에 **ODBC Driver 17 for SQL Server**가 설치되어 있어야 한다.

## 실행 전제 조건

모두 gitignore 대상이며 exe의 작업 디렉토리에 있어야 한다.

| 파일 | 내용 |
|---|---|
| `.env` | `DB_SERVER`, `DB_NAME`, `CONNECTION` |
| `server.key` / `server.crt` | gRPC SSL 인증서 |

`.env`의 값은 프로세스 환경변수로 밀어넣은 뒤 다시 읽어서 ODBC 연결 문자열을 만든다. 인증서가 없거나 비어 있으면 `main()`이 `-1`로 즉시 종료한다.

**DB 이름은 `MyGameDB`다** — 프로젝트 이름 `MiniGameDB`와 다르다. 바꾸려면 `.env`의 `DB_NAME`과 `schema.sql`을 함께 바꿔야 한다.

`server->Wait()`로 블로킹하므로 게임 서버보다 **먼저** 띄운다.

## 구조

`main.cpp`가 SSL 크레덴셜과 `ServerCompletionQueue`를 세팅하고, `ReadyForCall()`로 각 RPC의 첫 `CallData`를 걸어둔 뒤 **4개 스레드**가 `HandleRpcs()`를 무한 반복한다.

진행 중인 RPC 하나가 `CallData` 하나다(`CallData.h/.cpp`). 완료 이벤트를 받으면 자기 자신을 진행시키고 다음 요청용 `CallData`를 새로 걸어둔다.

| 파일 | 역할 |
|---|---|
| `main.cpp` | 서버 부트스트랩, 스레드 4개 |
| `DBServiceImpl.*` | `GreeterServiceImpl` — gRPC 서비스, `HandleRpcs()` |
| `CallData.*` | RPC별 상태 머신 + 실제 DB 작업 |
| `QueryExecuter.*` | ODBC 실행 헬퍼 |
| `GlobalVariables.*` | `GDBManager`(연결 풀), 해시/인코딩 헬퍼, `_DEBUG` 스모크 테스트 |
| `objectPool.*` | 게임 서버 `Libraries`의 풀을 복사해온 것 |
| `schema.sql` | 테이블 복원 스크립트 |
| `S2D_Protocol*.pb.*` | **생성 파일 — 직접 고치지 말 것** |

## 연결 핸들 풀과 Cleaner

`DBManager`(`GlobalVariables.h`)가 `SQLHDBC` 핸들을 풀링한다: `PopHDbc()` / `ReturnHDbc()`.

핸들 반납과 트랜잭션 롤백에는 `Cleaner` RAII 가드를 쓴다. **에러 분기마다 수동으로 정리하지 말고 이 패턴을 따를 것** — RPC 경로는 분기가 많아서 수동 정리가 새기 쉽다.

## 비밀번호

계정별 OpenSSL `RAND_bytes` salt를 사용한 PBKDF2-HMAC-SHA256이다.

| 파라미터 | 값 |
|---|---|
| `salt_size` | 16 바이트 |
| `hash_size` | 32 바이트 |
| `pbkdf2_iter` | 10000 |

해시와 salt는 바이트당 2자리 16진 문자열로 변환해 `NVARCHAR` 컬럼에 저장한다(각각 64자, 32자). 변환은 `v2wsRef`(바이트→16진 문자열) / `ws2vRef`(역방향)가 담당한다.

### 인코딩 헬퍼를 혼용하지 말 것

| 헬퍼 | 변환 |
|---|---|
| `a2wsRef` | CP949 → UTF-16 |
| `s2wsRef` | UTF-8 → UTF-16 |
| `v2wsRef` | 바이트 배열 → 16진 문자열 |
| `ws2vRef` | 16진 문자열 → 바이트 배열 |

앞의 둘은 이름이 비슷하지만 입력 인코딩이 다르다. 잘못 고르면 한글 아이디가 깨진다.

## 스키마

`schema.sql`은 원본 DB 유실 후 **C++ 코드가 실제로 실행하는 쿼리와 바인딩 타입/버퍼 크기에서 역산해 재구성한 것**이다. 각 테이블에 근거가 된 코드 위치가 주석으로 남아 있으니, 컬럼 타입이나 길이를 바꿀 때는 그 주석이 가리키는 코드를 함께 볼 것.

```powershell
sqlcmd -S localhost\SQLEXPRESS -E -i schema.sql
```

| 테이블 | 내용 |
|---|---|
| `Players` | `dbid`(IDENTITY) ↔ `player_id`(NVARCHAR(16), UNIQUE) |
| `Accounts` | `dbid`, `password_hash`(64), `salt`(32) |
| `Elos` | 게임별 Elo |
| `PersonalRecords` | 개인 최고 기록 |
| `PublicRecords` | 전체 최고 기록 |
| `CRUD` | `_DEBUG` 스모크 테스트용 |

`player_id`의 UNIQUE 제약은 필수다. 계정 생성 시 중복 아이디를 **SQLSTATE 23000**으로 판별하기 때문에, 제약이 없으면 중복 검사가 통째로 무력화된다.

## RPC 목록

`bin/S2D_PBFiles/S2D_Protocol.proto`의 `S2D_Service`:

| RPC | 용도 |
|---|---|
| `SayHello` | 서버 기동 시 연결 확인 |
| `LoginRequest` | 로그인 |
| `CreateAccountRequest` | 계정 생성 |
| `PlayerInfomation` | 플레이어 정보 조회 |
| `UpdateElo` | Elo 갱신 |
| `UpdatePersonalRecord` / `PublicRecord` / `UpdatePublicRecord` | 기록 조회·갱신 |

`RenewElosRequest`는 주석 처리되어 있다.

## 테스트

자동화된 테스트 스위트는 없다. `_DEBUG` 빌드에서만 시작 시점에 인라인 CRUD/인증 스모크 테스트가 돈다 — `GlobalVariables.cpp`의 `DBManager::InitialC/R/U/D`, `AkagiRedSunsNo2`, `ScandinavianFlick`. **이 생성자 경로가 이 저장소에서 테스트에 가장 가까운 것이다.** DB 쪽을 손봤다면 Debug로 한 번 띄워 이 경로가 통과하는지 보는 것이 최소 검증이다.
