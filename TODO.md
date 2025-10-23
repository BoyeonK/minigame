### TODO List

- Server
  - 0Byte recv를 받았을 경우(높은 확률로 연결 종료로 인한), session의 ptr을 가지고 있는 모든 객체에서의 일관된 소멸 처리
  - PingPongGameRoom
    - OnGoingState
      - Phase
        - EasyPattern만을 사용하는 Phase1 (20초, 6초대기)
        - Easy와 Medium이 섞인 Phase2 (20초, 6초대기)
        - Medium과 Hard가 섞인 Phase3 (20초, 6초 대기) 이후 CountingState로 전환.
      - KeepAlivePkt
        - Update문이 30번 호출 될 때마다(3초) KeepAlive 패킷을 보내 Ping을 확인.
        - 응답한 플레이어에게 추가점수 10점.
          - 연결만 유지되어있어도, 점수 260점은 먹고 시작함과 같음.
          - 연결이 올바르지 않은 경우, 실점이 적용되지 않는 비율(C_P_CollisionGoalLine)보다 승점이 적용되지 않는 비율(S_P_KeepAlivePkt)이 크도록 하기 위함
            - 일명, 랜선 흔들기 방지.
        - 이전 KeepAlive패킷에 연속으로 2번 이상 응답하지 않은 경우, 유효하지 않은 연결로 간주.
    - CountingState
      - IsUpdate를 false로 전환 (Update문 호출 중단)
      - 게임 결과를 계산, DB에 반영.
      - 참가한 Player들이 LobbyScene으로 돌아가도록 유도.
        - 현재 GameScene안의 모든 Object를 정지시키고 SceneUI로 결과를 통보.
        - 확인 버튼을 누른 경우 LobbyScene으로 전환.
      - 모든 과정이 성공함을 리턴한 경우, 사용한 모든 자원을 반환.
      - 이후, EndState로 전환하여 GameRoom 자체도 Pool에 반환될 수 있게 함.
  - PingPongPlayerGoalLine
    - Line에 충돌한 경우 해당 Player의 점수를 깎음.
      - Pupple -20점
      - Blue -15점
      - red -10점

- Client
  - 최초 접속 시에, 접속할 서버의 IP주소를 입력하도록 하기 (마크처럼)
  - PingPongPlayerGoalLine
    - OnTrigger 옵션을 이용하여, Line에 Object가 충돌한 경우. 해당 내용을 서버에 전송. (C_P_CollisionGoalLine)

- DB
  - 게임 끝난 후 결과 반영하기.
  - 승, 패자의 elo조정.

---

### 현재 우선순위

- 미니게임 추가하기
- 인게임 로직 완성하기
