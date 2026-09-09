# MiniGameClient — Unity 클라이언트

Unity 6000.4.2f1 / C#. Unity 에디터로 `MiniGameClient/` 폴더를 열어 실행한다.

저장소 전역 규칙(프로토콜 코드 생성, 네이밍)은 루트 [`../CLAUDE.md`](../CLAUDE.md)를 먼저 볼 것.

## 실행 전제 조건

`Assets/Scripts/Utils/GitIgnores.cs`가 `GitIgnores.sAddr`(서버 IP)를 제공한다. gitignore 대상이라 **로컬에 없으면 컴파일이 안 된다.**

현재 `NetworkManager.TryConnectToServer`는 `IPAddress.Loopback`으로 하드코딩되어 있고 `GitIgnores.sAddr` 줄은 주석 처리되어 있다. 원격 테스트 시 되돌려야 한다.

서버(`MiniGameServer`)와 DB 게이트웨이(`MiniGameDB`)가 먼저 떠 있어야 한다.

## Managers 싱글톤

`Assets/Scripts/Managers/Managers.cs`. `DontDestroyOnLoad` 싱글톤이 모든 서브시스템을 소유한다.

| 프로퍼티 | 파일 |
|---|---|
| `Network` | `Managers/Content/NetworkManager.cs` |
| `Object` | `Managers/Content/ObjectManager.cs` |
| `Scene` | `Managers/SceneManagerEx.cs` |
| `UI` | `Managers/UIManager.cs` |
| `Resource` | `Managers/ResourceManager.cs` |
| `Pool` | `Managers/PoolManager.cs` (+ `Poolable.cs`) |
| `Sound` | `Managers/SoundManager.cs` |
| `Input` | `Managers/InputManager.cs` |
| `Setting` | `Managers/SettingManager.cs` |

## 스레드 규칙 — 가장 자주 어기는 것

**패킷 핸들러는 Unity 메인 스레드가 아니라 소켓 스레드에서 돈다.**

`GameObject`, UI, 씬 로딩을 건드리는 것은 모두 `Managers.ExecuteAtMainThread(...)`로 감싸야 한다. `Managers.Update()`가 매 프레임 이 큐를 소진한다.

```
Socket Thread → Packet Handler → ExecuteAtMainThread() → Main Thread Job Queue → Unity Scene
```

`NetworkManager` 전체가 이 패턴으로 되어 있으니 그대로 따를 것. 새 핸들러를 추가할 때 이걸 빠뜨리면 에디터에서는 간헐적으로만 터져서 원인을 찾기 어렵다.

## NetworkManager

도메인별 중첩 매니저로 쪼개져 있고, 각각 `NetworkManager.Init()`에서 초기화된다.

`Lobby` / `Match` / `Loading` / `Race` / `PingPong` / `Mole`

핸들러는 `Managers.Scene.CurrentScene`을 구체 `BaseScene` 파생 타입(`RaceScene`, `PingPongScene`, `MoleScene`)으로 패턴 매칭해서 게임플레이에 접근한다. **씬이 null이거나 타입이 안 맞으면 그냥 early-return 하는 것이 정상 동작이며 에러가 아니다** — 씬 전환 도중 도착한 패킷을 버리는 경로다.

## 네트워크 레이어

`Assets/Scripts/Network/`의 `Session.cs`, `Connector.cs`, `RecvBuffer.cs`, `ServerSession.cs`는 C++ `Libraries` 코어를 그대로 옮긴 것이다. 4바이트 `PacketHeader`(`ushort size`, `ushort id`)도 동일하다.

```
| 2Byte        | 2Byte        |       ...       |
┌──────────────┬──────────────┬─────────────────┐
│ Packet Size  │  Packet ID   │ Protobuf Data   │
└──────────────┴──────────────┴─────────────────┘
```

`ClientPacketManager.cs`가 `_onRecv`, `_handler`, `_msgFactories` 세 테이블을 들고 디스패치한다. **`S_Encrypted` 안에 실려오는 패킷이라면 factory가 반드시 있어야 한다.** 새 패킷 추가 시 손대야 하는 5군데는 루트 CLAUDE.md의 "프로토콜 코드 생성"에 정리되어 있다.

`S2CProtocol*.cs`는 `bin/S2C_PBFiles/`에서 생성해 복사해온 파일이다. 직접 고치지 말 것.

암호화는 `Assets/Plugins/BouncyCastle.Cryptography.dll`을 쓴다(RSA로 AES-256 키 전달 → 이후 AES-256-GCM).

## 씬

**씬은 반드시 Build Settings에 등록되어 있어야 한다.** `SceneManagerEx.GetSceneName()`이 `Enum.GetName(typeof(Define.Scene), type)`으로 **이름 문자열**을 만들어 `SceneManager.LoadScene(name)`에 넘기기 때문이다. 등록되지 않은 씬은 에디터 플레이에서도 로드에 실패하며, 플레이어 빌드에는 아예 포함되지 않는다.

실제로 로드되는 씬은 6개다.

| # | 경로 | 용도 | 스크립트 |
|---|---|---|---|
| 0 | `Assets/Scenes/Login.unity` | 시작 씬. 로비 UI도 여기서 담당 | `Scenes/LoginScene.cs` |
| 1 | `Assets/Scenes/LoadingScenes/LoadingScene1.unity` | 게임 진입 로딩 | `Scenes/Loading/LoadingScene1.cs` |
| 2 | `Assets/Scenes/LoadingScenes/GameResult.unity` | 게임 종료 후 결과 | `Scenes/Loading/GameResultScene.cs` |
| 3 | `Assets/Scenes/Race.unity` | | `Scenes/RaceScene.cs` |
| 4 | `Assets/Scenes/PingPong.unity` | | `Scenes/PingPongScene.cs` |
| 5 | `Assets/Scenes/Mole.unity` | | `Scenes/MoleScene.cs` |

`Define.Scene`의 `Lobby`, `TestLoadingScene`, `Undefined`는 `LoadScene`에 전달되는 곳이 없다(`Lobby`는 `Login` 씬이 겸하고, `TestLoadingScene`은 잔재). `Assets/Scenes/TestScene.unity`도 잔재다. 등록할 필요 없다.

**Unity 에디터가 열려 있는 동안 `ProjectSettings/*.asset`을 디스크에서 직접 편집하지 말 것.** 에디터가 해당 설정을 메모리에 들고 있다가 종료 시 덮어써서 수정이 사라진다. 에디터를 닫고 편집하거나, 에디터 UI / `EditorBuildSettings` API로 바꿔야 한다.

## Define.GameType

클라이언트의 `Define.GameType`(`Utils/Define.cs`)에는 서버 `GameType`에 없는 **`InProcess = 5`**가 있다. 클라이언트 전용 매치메이킹 상태이므로 불일치를 "고치려" 하지 말 것.

## 스크립트 배치

| 디렉토리 | 내용 |
|---|---|
| `Scripts/Managers/` | 싱글톤과 서브시스템 |
| `Scripts/Managers/Content/` | `NetworkManager`, `ObjectManager` |
| `Scripts/Network/` | 세션·버퍼·패킷 디스패치 + 생성된 protobuf `.cs` |
| `Scripts/Scenes/` | `BaseScene` 파생. 씬별 게임플레이 진입점 |
| `Scripts/Controller/GameObjectControllers/` | 서버 `UnityGameObject`에 대응하는 클라 컨트롤러 |
| `Scripts/Controller/{Race,Mole,Lobby,LoadingScene1}/` | 게임별 씬 오브젝트 컨트롤러 |
| `Scripts/UI/Scene/`, `Scripts/UI/Popup/` | `UI_Scene` / `UI_Popup` 파생 |
| `Scripts/Utils/` | `Define.cs`, `Util.cs`, `PriorityQueue.cs`, (`GitIgnores.cs`) |

`Controller/TestScene/`, `TestGameBulletController.cs`, `Scenes/TestScene.cs`는 잔재다.

## 에셋

`Assets/Plugins/*.dll`은 빌드 산출물이 아니라 실행에 필요한 외부 의존성이라 **추적한다**(`.gitignore`에 `!` 예외로 명시). `Google.Protobuf.dll`의 버전이 `S2CProtocol*.cs` gencode를 묶어두고 있으므로, 이 DLL을 갈아끼우지 않는 한 C# gencode를 재생성할 이유가 없다.

반면 에셋 스토어 팩은 전부 gitignore 대상이다.

```
Haons SD series Pack/   unity-chan!/   uchwch/   LowPoly*/
TextMesh Pro/   UI Toolkit/   Packages/   Fonts/   Sounds/
```

폴더가 추적되지 않으면 Unity가 만든 그 폴더의 `.meta`도 저장소에 있을 이유가 없다. **팩을 무시할 때는 `<팩>/`과 `<팩>.meta`를 함께 넣을 것**(`LowPoly*/` + `LowPoly*.meta`가 그 예다). 이미 추적 중이던 `.meta`라면 `.gitignore`만으로는 빠지지 않으므로 `git rm --cached`로 인덱스에서 내려야 한다. 빠진 게 있는지는 이 명령으로 확인한다.

```bash
git ls-files -i -c --exclude-standard   # 무시 규칙에 걸리는데도 추적 중인 파일
```

### 유실 에셋 대응

팩 폴더가 추적되지 않기 때문에, **팩을 새로 받거나 폴더 레이아웃이 바뀌면 추적 중인 프리팹·씬의 guid 참조가 끊긴다.** 실제로 반복해서 겪은 문제다.

- 증상: 프리팹의 머티리얼이 분홍색, TMP 텍스트가 빈 채로 렌더링, 프리팹 하위 오브젝트 누락
- 대응: 끊긴 guid를 팩의 현재 경로에 있는 에셋으로 다시 물린다. 프리팹 `.meta`/`.prefab`은 텍스트라 `grep -rl <guid> Assets/`로 참조처와 대상을 양쪽에서 찾을 수 있다
- 한 팩 안에 구/신 레이아웃이 동시에 존재할 수 있다(예: `Haons SD series Pack`의 `Materials/`와 `Material/Character(Ver3)/`). 어느 쪽을 가리켜야 하는지 확인하고 고칠 것

## 솔루션 파일

Unity 6은 `.sln` 대신 `MiniGameClient.slnx`(새 XML 솔루션 포맷)를 생성한다. `.csproj`는 gitignore 대상이지만 솔루션 파일은 아니어서, 구 `MiniGameClient.sln`은 추적되고 있었고 새 `.slnx`는 아직 추적되지 않은 상태다. 어느 쪽이든 에디터가 재생성하는 파일이니 직접 편집하지 말 것.
