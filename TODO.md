### TODO List

- Server
  - GameRoom
    - BeforeInitState
      - Init
        - 매치메이킹 로직으로 정해진 그룹에 대해서 유효한 상태인지 확인
          - session의 weak_ptr이 유효한가?
          - Disconnect되지않은 유효한 세션인가?
          - CAS연산으로 해당 Session의 state를 올바르게 변경에 성공했는가?
          - 모든 패킷이 위의 과정을 통과하지 못했다면 다시 대기열로.
        - 위의 과정을 통과한 Session에는 S_MatchmakeKeepAlive패킷을 보내 응답을 유도.
      - Init2
        - Init 실행에 따라 모든 세션이 유효하면 TimerJobQueue에 Push되어 1000ms 뒤에 실행 유도됨.
        - S_MatchmakeKeepAlive패킷에 대한 응답을 해당 Room의 모든 세션에서 확인 S_MatchmakeCompleted
          - 1000ms이상 걸리는 친구는, 현실적으로 게임 속행이 불가능하다고 판단  
          - 응답하지 않은 세션이 있는 경우, 다시 모든 인원을 대기열로 돌려보냄.
            - 대기열은 자체적으로 유효하지 않은 친구들을 걸러내므로, 여기서 올바른 응답을 한 세션만 골라서 돌려보내는 것은 자원낭비.
        - 모든 세션이 올바르게 응답한 경우, State를 BeforeStartState로 전환. S_MatchmakeCompleted패킷을 Broadcast.
          - Client로 하여금 Scene의 전환을 유도한다.
          - 지금부터는 매칭이 완료되었음으로 간주.
          - 이후부터 세션이 유효하지 않아도 작동하도록 설계
          - 이후부터 통신에 문제가 발생하면, 전적으로 플레이어의 책임으로 간주한다. (게임 종료로 간주)
    - BeforeStartState
      - Start
        - 각 플레이어가 로딩이 완료되었는지 주기적으로 확인.
        - 로딩 진행상황을 전달받음. 
        - (바뀔 수 있음) 필요하다면 각 클라에서 받은 로딩 상황을 Broadcast해서 각 클라이언트가 다른 클라이언트의 진행상황을 알 수 있게 함.
        - 모든 플레이어가 로딩이 완료된 경우, OnGoingState로 전환. 진짜 게임로직을 시작.
        - 일정 시간동안 모든 플레이어의 로딩 진행 상황에 대한 진전이 없을경우 (델타값 모두 0), OnGoingState로 전환.
        - OnGoingState로 전환할 경우, S_GameStart 패킷을 Broadcast.
    - OnGoingState
      - 시작된 게임으로부터 인게임 로직. 게임마다 완전히 다르다.
      - TestGame은 정원 1명인 게임으로, 아무것도 하지 않고 10초후 CountingState로 전환.
        - OnGoingState(진짜 인게임 로직)을 제외한 나머지가 정상적으로 동작하는지 디버그용.
    - CountingState
      - 게임 종료 후, 게임 결과 DB에 반영.
        - TestGame의 경우, elo가 짝수면 +1, elo가 홀수면 -1 하는 것으로 테스트.
      - 각 플레이어 세션으로 하여금 다시 로비로 올바르게 복귀할 수 있도록 함.
      - Room내에서 사용된 자원들을 반환. 올바르게 진행된 경우 EndGameState로 전환.
    - EndGameState (구현 되어 있음.)
      - 주기적으로 Matchmake thread에서 EndGame인 방들을 ObjectPool로 반환. (5초마다)

- Client
  - S_MatchmakeCompleted 패킷을 받은 경우, LoadingScene으로 선제적으로 전환하고, Manager에서 GameScene을 비 동기적으로 로딩.
  - 일정 주기 혹은 일정 진행상황마다 로딩 진행상황을 서버에 전송.
  - S_GameStart를 받은 경우, 로딩 완료된 GameScene으로 전환. RequestGameInfo패킷을 보내 서버에 현재 Room의 상태를 받아 GameScene초기화(동기화).
  - 늦게라도 로딩이 완료된 경우, RequestGameInfo를 보내 클라이언트의 상태와 서버의 상태를 동기화.

  - 튕긴 플레이어 재접속 기능 구현 예정 절대 없음 (엄청 힘들다는걸 알고 있다.)

- DB
  - CountingState가 구현될 때 까지 할 일 없음.

- 공통

---

### 현재 우선순위

- 미니게임 추가하기
- 인게임 로직 완성하기
