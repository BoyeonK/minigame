# Progress

최종 갱신: 2026-09-09

## 완료

### Server

- [x] (2026-09-09 #0) `objectPool`의 해제 순서와 풀 소멸자를 정리. `dealloc()`이 `pushEntry()`로 블록을 공개한 뒤에 소멸자를 돌려서, 다른 스레드가 그 블록을 pop해 placement-new 하는 도중 파괴될 수 있었다. 소멸을 먼저 하도록 순서를 뒤집고, `~poolHeader()`에서 이미 파괴된 객체를 두 번 소멸시키며 `++pSE`라는 잘못된 주소를 `_aligned_free`에 넘기던 코드를 제거했다.
- [x] (2026-09-09 #1) Race 러너의 오브젝트 id를 플레이어 인덱스와 일치시킴. 클라이언트가 `S_R_ResponseState.playerid`를 오브젝트 id와 대조하는데 서버는 `GenerateUniqueGameObjectId()`로 임의 id를 붙이고 있었고, `Start()`의 러너 생성 루프도 정원과 무관하게 4로 하드코딩되어 있었다. `RaceRoom::QUOTA`(2) 상수를 도입해 루프를 `_quota` 기준으로 바꾸고, 러너 id는 인덱스 그대로 쓰되 `_nxtObjectId`를 `QUOTA`에서 시작시켜 나머지 오브젝트와 충돌하지 않게 했다.
- [x] (2026-09-09 #9) 매치 취소 실패 사유가 깨져 전송되던 문제 수정. `Handle_C_MatchmakeCancel`의 한글 문자열 두 개에 `u8` 접두사가 빠져 있어, 실행 문자셋(ANSI)을 거친 CP949 바이트가 proto3 `string`에 실려 나가고 클라이언트가 `Encoding.UTF8`로 디코드하면서 깨져 보였다. 같은 파일의 `MatchmakeRequest` 쪽 네 곳은 원래 `u8`을 쓰고 있었고, 이제 `set_err` 경로의 한글 리터럴 여섯 개가 모두 일관된다.

### Client

- [x] (2026-09-09 #2) Race 씬의 유실 에셋 재바인딩. `Haons SD series Pack`이 새 폴더 레이아웃(`Material/Character(Ver3)/`)으로 재설치되면서 `SDUnityChan.prefab`의 코스튬·페이스스킨 머티리얼 guid가 끊겨 있었고, `Hammer.prefab`이 참조하는 `Hammer_06.prefab`도 사라져 있었다. 머티리얼을 새 경로로 다시 물리고 `Hammer_06.prefab`을 복구했다.
- [x] (2026-09-09 #3) Unity 6이 생성하는 새 솔루션 포맷으로 전환. `MiniGameClient.sln`이 `MiniGameClient.slnx`로 대체됐다.
- [x] (2026-09-09 #5) 에셋 팩 폴더의 `.meta`를 gitignore 대상에 추가. 팩 폴더는 무시되는데 Unity가 만든 폴더 `.meta`는 추적되고 있어서, 저장소에 있을 이유가 없는 파일이 남고 팩을 다시 받을 때마다 diff가 났다. `Haons SD series Pack` / `unity-chan!` / `uchwch` / `TextMesh Pro` / `UI Toolkit` / `Fonts` 여섯 개에 `<팩>.meta` 패턴을 넣고 이미 추적 중이던 것은 `git rm --cached`로 내렸으며, DLL이 `Plugins/`로 옮겨간 뒤 남아 있던 빈 `Assets/Libs/`와 `Libs.meta`도 함께 제거했다.
- [x] (2026-09-09 #7) 추가된 음원에 맞춰 사운드 참조를 정리. `Resources.Load`가 없는 경로에 `null`을 돌려주고 `SoundManager.Play`가 `null`이면 조용히 return해서, 이름이 어긋난 참조가 로그 한 줄 없이 무음 처리되고 있었다. `footstep`·`PingPongImpact` 제거, `MoleSetPoint` 오타를 `MoleGetPoint`로 교정하며 3개로 축소, `jump` 단일화, 음원이 없던 `swipe`를 `select`로 대체했고, 이후 `button.wav`까지 추가되어 코드가 참조하는 클립 12개가 모두 로드된다.

### 저장소

- [x] (2026-09-09 #8) 손으로 쓴 소스 64개를 CP949에서 UTF-8(BOM)로 전면 통일. MSVC는 BOM이 없으면, Roslyn은 UTF-8 디코드에 실패하면 시스템 ANSI 코드페이지로 소스를 읽기 때문에 기존 소스는 ACP가 949인 기계에서만 올바르게 컴파일됐고, protoc 생성물은 이미 UTF-8이라 CP949 쪽으로 맞추는 선택지도 없었다. 변환은 CP949 왕복 비교로 무손실을 확인했고, 두 `.vcxproj`에 `/source-charset:utf-8`을 넣고 `.gitattributes`로 개행을 고정했다. 인코딩 혼재를 설명하느라 상시 컨텍스트를 잡아먹던 루트 `CLAUDE.md`의 인코딩 절은 설명할 것이 없어져 삭제했다(9637 → 8361 bytes).

### 문서

- [x] (2026-09-09 #4) 문서를 컴포넌트별 `CLAUDE.md`로 분리. 루트 하나에 서버·DB·클라이언트 문맥이 전부 섞여 있어 한 컴포넌트만 볼 때도 전체를 읽어야 했다. 세 디렉터리에 각각 `CLAUDE.md`를 만들어 내부 구조·빌드 함정·파일 지도를 옮기고, 루트는 저장소 전역 규칙과 하위 문서 참조만 남겼다(398 → 121줄).
- [x] (2026-09-09 #6) `README.md`를 문서 재구성 이전 상태로 되돌리고 수정 금지 규칙을 명시. 포트폴리오용이라 문장과 구성을 직접 관리하는 문서인데, `CLAUDE.md` 분리 작업의 부수 효과로 저장소 구성 표와 절별 소스 포인터가 들어가 있었다. `b36f35f^` 시점의 파일로 복원하고, 루트 `CLAUDE.md`의 문서 표와 아래 메모에 별도 요청 없이는 수정하지 않는다는 규칙을 남겼다.

## TODO

### Server

- [ ] 위 `objectPool` / `RaceRoom` 수정 후 x64 재빌드하고, Race 2인 매칭을 실제로 돌려 러너 id 매칭이 맞는지 확인

### 문서

- [ ] 각 게임 스크린샷 및 데모 플레이 영상 촬영. 게임이 실제로 돌아가는 상태가 된 뒤에 진행하고, README `2. 데모`의 `Screenshot / GIF` 자리와 로비 스크린에 넣는다.
- [ ] README.md 다듬기. 데모 자료를 채운 뒤 문장과 구성을 최종 정리한다. **별도 요청이 있을 때만 손댄다.**

## 알려진 이슈 / 메모

- `NetworkManager.TryConnectToServer`가 `IPAddress.Loopback`으로 하드코딩되어 있고 `GitIgnores.sAddr` 줄은 주석 처리 상태다. 원격 테스트 전에 되돌려야 한다.
- 에셋 팩은 폴더뿐 아니라 Unity가 만든 폴더 `.meta`도 gitignore 대상으로 둔다. 팩을 새로 무시할 때는 `<팩>/`과 `<팩>.meta`를 **함께** 넣을 것. `git ls-files -i -c --exclude-standard`를 돌리면 규칙에 걸리는데도 추적 중인 파일이 나온다(현재 없음).
- Race 씬은 `gameEnd`를 프리로드만 하고 재생하지 않는다. PingPong·Mole과 달리 종료음이 없는 것은 현행 유지로 결정된 상태다.
- 이 파일이 기존 `TODO.md`를 대체한다(`TODO.md`는 삭제됨).
- 컴포넌트별 문맥은 각 디렉터리의 `CLAUDE.md`에 있고, 루트 `CLAUDE.md`는 저장소 전역 규칙만 담는다.
- **`README.md`는 별도 요청이 없으면 수정하지 않는다.** 포트폴리오용 문서라 문장과 구성을 직접 관리한다. 내용 갱신이 필요해 보이면 고치지 말고 TODO나 메모로 남길 것.
