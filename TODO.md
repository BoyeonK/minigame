### TODO List

- Server
  
  - 대기열 진입 로직 추가.
    - player의 elo를 토대로 다른 큐에 배치되도록 함. (DB 열람 필요)

- Client
  
  - 대기열 진입 로직 추가.
    
    - 매치메이킹, 인게임에서 각각 같은 socket을 사용할 것인가 결정 필요

- 공통
  
  - 3가지 이상의 미니게임을 추가하고, 매치메이킹을 통해 플레이 할 수 있도록 함.
  
  - 메모리 안의 특정 int값의 변화 추이를 시각화하기.
    
    - python으로 작성한 별도의 응용 프로그램을 만들어서 서버측과 통신할 예정.
  
  - Microsoft SQL server를 사용할 예정
    
    - gRPC를 활용하여 DB조작하기
      - 트랜잭션, 실패시 예외처리 주의
    - SSMS (SQL Server Management Studio) 사용해보기
      - SSIS (Integration Service) 사용해보기 (우선순위 낮음.)

ㅇ

### Until Today
