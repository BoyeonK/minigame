### TODO List

- Server
  
  - 대기열 진입 로직 추가.
    - player의 elo를 토대로 매칭
      - gRPC를 활용하여 DB에서 해당 player의 elo? mmr? 을 조회.
      - 2개의 vector를 사용.
        - 1 -> 대기열에 들어온 플레이어를 일단 받아들일 대기열
        - 2 -> 실제로 sliding window방식으로, 플레이 인원끼리 elo차이가 최소가 되는 플레이어 집합을 찾을 때 사용할 대기열.
        - 대기열에 들어오는 플레이어를 vector안에 집어넣고 elo순으로 정렬하면서, 동시에 sliding window방식의 알고리즘을 적용하는 것은 불가능하므로, 우선 대기열에 들어온 플레이어는 1의 vector에 추가하고, 이후 일정 시간간격마다 한번씩 (현재 5초 정도로 상정하는중) 2의 vector에 추가하는 방식을 고려중.

- Client
  
  - 대기열 진입 로직 추가.
  
  - 일단은 dummy를 사용해서 test

- DB
  
  - Microsoft SQL server사용.
    
    - 인게임 로직 서버와 gRPC 방식으로 통신할 예정.
      
      - Server에게 player의 mmr을 조회할 수 있도록 한다.
      
      - 한 게임이 끝난 경우, 게임 결과를 기록한다.
      
      - 한 게임이 끝난 경우, 결과에 따라 각 플레이어의 mmr을 조정한다
        
        - 트랜잭션 처리 주의!

- 공통
  
  - 3가지 이상의 미니게임을 추가하고, 매치메이킹을 통해 플레이 할 수 있도록 함.
  
  - 메모리 안의 특정 int값의 변화 추이를 시각화하기.
    
    - python으로 작성한 별도의 응용 프로그램을 만들어서 서버측과 통신할 예정.
  
  - SSMS (SQL Server Management Studio) 사용해보기
    
    - 지금은 한 대의 기기에서 인게임 로직 서버와 DB를 같이 돌리고 있지만, 나중에 노트북을 1대 들고와서 DB서버를 별도의 기기에서 실행할 예정
    - SSIS (Integration Service) 사용해서 DB 정보 옮겨보기.

ㅇ

### Until Today
