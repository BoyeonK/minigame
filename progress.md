# Progress

최종 갱신: 2026-09-09

## 완료

### Server

- [x] (2026-09-07 #0) vcpkg 업데이트로 깨진 서버·DB 빌드 복구. protobuf 런타임과 커밋된 C++ gencode의 버전이 어긋나 `C1189`가, abseil의 `MutexLock` deprecation이 `/sdl` 때문에 `C4996` 에러로 승격되어 빌드가 멈춰 있었다. C++ gencode만 재생성하고 두 `.vcxproj`에 `/w34996`을 `AdditionalOptions`로 넣어 해결했다.
- [x] (2026-09-09 #0) `objectPool`의 해제 순서와 풀 소멸자를 정리. `dealloc()`이 `pushEntry()`로 블록을 공개한 뒤에 소멸자를 돌려서, 다른 스레드가 그 블록을 pop해 placement-new 하는 도중 파괴될 수 있었다. 소멸을 먼저 하도록 순서를 뒤집고, `~poolHeader()`에서 이미 파괴된 객체를 두 번 소멸시키며 `++pSE`라는 잘못된 주소를 `_aligned_free`에 넘기던 코드를 제거했다.
- [x] (2026-09-09 #1) Race 러너의 오브젝트 id를 플레이어 인덱스와 일치시킴. 클라이언트가 `S_R_ResponseState.playerid`를 오브젝트 id와 대조하는데 서버는 `GenerateUniqueGameObjectId()`로 임의 id를 붙이고 있었고, `Start()`의 러너 생성 루프도 정원과 무관하게 4로 하드코딩되어 있었다. `RaceRoom::QUOTA`(2) 상수를 도입해 루프를 `_quota` 기준으로 바꾸고, 러너 id는 인덱스 그대로 쓰되 `_nxtObjectId`를 `QUOTA`에서 시작시켜 나머지 오브젝트와 충돌하지 않게 했다.

### Client

- [x] (2026-09-07 #1) 유실된 TMP 폰트를 Maplestory/Bazzi로 재바인딩. 폰트 에셋 guid가 깨져 UI 프리팹과 씬의 텍스트가 전부 빈 채로 렌더링됐다.
- [x] (2026-09-07 #2) LoginScene의 게임 설명 텍스트 `NullReferenceException` 수정.
- [x] (2026-09-07 #3) 클라이언트 복구: Unity 6000.4.2f1 업그레이드 및 에셋 경로 정리. URP 설정과 Libs DLL의 meta가 어긋나 프로젝트가 열리지 않던 상태를 되살렸다.
- [x] (2026-09-07 #4) README 1차 정리. 기존 문서는 `docs/previous-README.md`로 옮기고 포트폴리오용 구성으로 다시 썼다.
- [x] (2026-09-09 #2) Race 씬의 유실 에셋 재바인딩. `Haons SD series Pack`이 새 폴더 레이아웃(`Material/Character(Ver3)/`)으로 재설치되면서 `SDUnityChan.prefab`의 코스튬·페이스스킨 머티리얼 guid가 끊겨 있었고, `Hammer.prefab`이 참조하는 `Hammer_06.prefab`도 사라져 있었다. 머티리얼을 새 경로로 다시 물리고 `Hammer_06.prefab`을 복구했다.
- [x] (2026-09-09 #3) Unity 6이 생성하는 새 솔루션 포맷으로 전환. `MiniGameClient.sln`이 `MiniGameClient.slnx`로 대체됐다.

### 문서

- [x] (2026-09-09 #4) 문서를 컴포넌트별 `CLAUDE.md`로 분리. 루트 하나에 서버·DB·클라이언트 문맥이 전부 섞여 있어 한 컴포넌트만 볼 때도 전체를 읽어야 했다. 세 디렉터리에 각각 `CLAUDE.md`를 만들어 내부 구조·빌드 함정·파일 지도를 옮기고 루트는 저장소 전역 규칙만 남겼으며(398 → 121줄), `README.md`에는 저장소 구성 표와 절별 소스 위치를 넣어 문서에서 코드로 이어지게 했다.

## TODO

### Server

- [ ] 위 `objectPool` / `RaceRoom` 수정 후 x64 재빌드하고, Race 2인 매칭을 실제로 돌려 러너 id 매칭이 맞는지 확인

### Client

- [ ] 소실된 Sound Asset을 대체할 음원 추가. `Assets/Resources/Sounds/`는 gitignore 대상(`[Ss]ounds/`)이라 저장소에 없고 디스크에도 BGM 3개만 남아 있다. `SoundManager.GetOrAddAudioClip`이 `Sounds/<이름>`으로 로드하므로 파일 이름을 아래와 정확히 맞춰야 한다.
  - BGM (1개 누락): `PingPongScene`  ※ `LobbyScene` / `RaceScene` / `MoleScene`은 있음
  - SFX (17개 전부 누락): `button`, `select`, `swipe`, `gameEnd`, `MoleStunned`, `footstep0`~`footstep4`, `jump0`~`jump1`, `MoleGetPoint0`~`MoleGetPoint3`

### 문서

- [ ] 각 게임 스크린샷 및 데모 플레이 영상 촬영. 게임이 실제로 돌아가는 상태가 된 뒤에 진행하고, README `2. 데모`의 `Screenshot / GIF` 자리와 로비 스크린에 넣는다.
- [ ] README.md 다듬기. 데모 자료를 채운 뒤 문장과 구성을 최종 정리한다.

## 알려진 이슈 / 메모

- `NetworkManager.TryConnectToServer`가 `IPAddress.Loopback`으로 하드코딩되어 있고 `GitIgnores.sAddr` 줄은 주석 처리 상태다. 원격 테스트 전에 되돌려야 한다.
- 에셋 팩은 폴더뿐 아니라 Unity가 만든 폴더 `.meta`도 gitignore 대상으로 둔다. 팩을 새로 무시할 때는 `<팩>/`과 `<팩>.meta`를 **함께** 넣을 것. `git ls-files -i -c --exclude-standard`를 돌리면 규칙에 걸리는데도 추적 중인 파일이 나온다(현재 없음).
- 이 파일이 기존 `TODO.md`를 대체한다(`TODO.md`는 삭제됨).
- 컴포넌트별 문맥은 각 디렉터리의 `CLAUDE.md`에 있다. 루트 `CLAUDE.md`는 저장소 전역 규칙만 담고, `README.md`가 각 디렉터리로 들어가는 입구 역할을 한다.
