### TODO List

- Server
  - 0Byte recv를 받았을 경우(높은 확률로 연결 종료로 인한), session의 ptr을 가지고 있는 모든 객체에서의 일관된 소멸 처리
  - GameResult
    - 최초에 DBID를 배열로 가지고있다가, 결과를 전송할 때, DB를 조회하여 playerId를 전송할 수 있도록 함.

- Client
  - 최초 접속 시에, 접속할 서버의 IP주소를 입력하도록 하기 (마크처럼)
  - LobbyScene
    - 개인 기록과 월드 레코드를 조회할 수 있도록 함.
    - 매칭중인 경우 UI표시
    - 매치 취소 UI표시
    - 스크린에 인게임 이미지 표시
  - PingPong
    - 게임 종료시, 즉각적으로 결과창으로 넘어가지 않고 UI를 띄워 종료됨을 알리고 추가적인 조작으로 결과창 유도
  - GameResultScene
    - 버튼으로 Scene전환, LobbyScene이 로딩되는 도중인 경우, 버튼 비활성화
    - 벽에 부딪힌 Bullet 무효화

- DB

---

### 현재 우선순위

- 클라이언트 쪽에 미구현 기능 채우기