### TODO List

완료된 부분은 취소줄로 처리.

- Server
  
  - 대기열 진입 로직 추가.
    
    - player의 elo를 토대로 매칭
      - gRPC를 활용하여 DB에서 해당 player의 elo? mmr? 을 조회.
      - 2개의 vector를 사용.
        - 1 대기열에 들어온 플레이어를 일단 받아들일 대기열
        - 2 실제로 sliding window방식으로, 플레이 인원끼리 elo차이가 최소가 되는 플레이어 집합을 찾을 때 사용할 대기열.
        - 대기열에 들어오는 플레이어를 vector안에 집어넣고 elo순으로 정렬하면서, 동시에 sliding window방식의 알고리즘을 적용하는 것은 불가능하므로, 우선 대기열에 들어온 플레이어는 1의 vector에 추가하고, 이후 일정 시간간격마다 한번씩 (현재 5초 정도로 상정하는중) 2의 vector에 추가하는 방식을 고려중.
  
  - DB작업의 진행은 가능한 비동기적으로 실행되는 것이 바람직함. (느리니까)
    
    - DB작업을 Functor형태로 만들어 Queue에 밀어넣고, 특정 thread가 queue에 있는 DB작업을 도맡아 하는 형태로 구상중.
    - Player Session에 의해 일어난 작업(클라이언트의 요청)인 경우 , Functor가 해당 session에 대한 정보(weak_ptr)를 가지고 있게 만들어서, 작업 완료시 해당 세션으로 하여금 (필요한 경우) 추가적인 작업을 하도록 로직도 만들 수 있을듯 하다.
  
  - 비동기 gRPC에 사용할 객체, CallData를 Pooling해서 사용
  
  - !현재 PlayerSession에 Send부분은, 멀티쓰레드 환경을 충분히 고려하지 못한 상황!
    
    - Send과정에서 사용할 overlapped객체를 pooling해서 사용해야 함.

- Client
  
  - 대기열 진입 로직 추가.
  - 일단은 dummy를 사용해서 test

- DB
  
  - ~~Microsoft SQL server사용.~~
  - ~~인게임 로직 서버와 gRPC 방식으로 통신할 예정.~~
    - Server에게 player의 mmr을 조회할 수 있도록 한다.
    - 한 게임이 끝난 경우, 게임 결과를 기록한다.
    - 한 게임이 끝난 경우, 결과에 따라 각 플레이어의 mmr을 조정한다
      - 트랜잭션 처리 주의!
  - 각 Table의 column이 어떤 이름으로 어떤 변수형을 가지고 있는지를 토대로, 각 Table에 대한 DB작업을 간소화하는 handler함수 작성 (위험한 발상인가?) 
    - ~~Query문을 작성할 때, 매개변수를 사용한 방법으로서 작성 (코드인젝션)~~
    - 올바르지 못한 방법으로 DB조작 방지.
  - 비동기 gRPC에 사용할 객체, CallData를 Pooling해서 사용

- 공통
  
  - 3가지 이상의 미니게임을 추가하고, 매치메이킹을 통해 플레이 할 수 있도록 함.
  - 메모리 안의 특정 int값의 변화 추이를 시각화하기.
    - python으로 작성한 별도의 응용 프로그램을 만들어서 서버측과 통신할 예정.
  - ~~SSMS (SQL Server Management Studio) 사용해보기~~
    - ~~지금은 한 대의 기기에서 인게임 로직 서버와 DB를 같이 돌리고 있지만, 나중에 노트북을 1대 들고와서 DB서버를 별도의 기기에서 실행할 예정~~
    - SSIS (Integration Service) 사용해서 DB 정보 옮겨보기.

ㅇ

### Until Today
