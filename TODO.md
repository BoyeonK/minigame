### TODO List

완료된 부분은 취소줄로 처리.

- Server
  
  - 로그인
    
    - 로그인처리되지 않은 세션에서는, HandShake과정에서 필요한 패킷들과 로그인시도 패킷 이외의 모든 요청을 reject. (packetID > 4 인 패킷에 대한 처리를 하지 않는다.)
    
    - 암호화된 패킷으로서 Client에서 ID와 password를 받는다.
    
    - DB를 조회한다. (`C2S_Login`을 받은 경우 `S2D_Login`전송)
      
      1. 있는 경우 (`D2S_LoginDbid`을 받은 경우 처리)
         
         1. 비밀번호가 맞은 경우 (0 이외의 값을 받았다.) : session에 해당 dbid를 session에 저장. 로그인 성공. `S2C_Login`에 True를 담아 전송.
         
         2. 비밀번호가 틀린 경우 (0을 받았다.) : 로그인 실패. 클라이언트에 통보. `S2C_Login`에 False를 담아 전송
      
      2. 없는 경우 (`D2S_LoginFailed`를 받은 경우 처리)
         
         - 계정을 만들고, 다시 로그인 시도한다. 
           해당 ID와 password로 계정 생성을 시도. (`S2D_CreateAccount`전송. 지금은 새로 만들지만, 나중에 계정 생성 기능이 따로 생긴다면, 1-2와 동일하게 처리할 예정)
           이후, OnSucceed함수로서 같은 ID, password로 DB조회 과정을 반복한다. (`S2D_Login` 전송과정부터 다시 시작.)
           정상적인 경우, 1-1상황과 같아진다.
  
  - 대기열 진입 로직 추가.
    
    - player의 elo를 토대로 매칭
      
      - gRPC를 활용하여 DB에서 해당 player의 elo? mmr? 을 조회.
      
      - 2개의 vector를 사용.
        
        - 1 대기열에 들어온 플레이어를 일단 받아들일 대기열
        
        - 2 실제로 sliding window방식으로, 플레이 인원끼리 elo차이가 최소가 되는 플레이어 집합을 찾을 때 사용할 대기열.
        
        - 대기열에 들어오는 플레이어를 vector안에 집어넣고 elo순으로 정렬하면서, 동시에 sliding window방식의 알고리즘을 적용하는 것은 불가능하므로, 우선 대기열에 들어온 플레이어는 1의 vector에 추가하고, 이후 일정 시간간격마다 한번씩 (현재 5초 정도로 상정하는중) 2의 vector에 추가하는 방식을 고려중.

- Client
  
  - 로그인UI `C2D_Login`에 ID와 password를 담을 수 있는 UI생성
  
  - 대기열 진입 로직 추가. `C2D_MatchMaking`
  
  - 일단은 dummy를 사용해서 test

- DB
  
  - 로그인 시도 (`S2D_Login`을 받은 경우 처리)
    
    - DB를 조회해서 해당 ID를 조회한다.
      
      1. 해당 ID가 없는경우.
         
         - 해당 ID가 없다는 내용의 패킷을 전송한다. `D2S_LoginFailed`
      
      2. 해당 ID가 있는경우.
         
         - password에 해당 ID의 salt를 섞어서 해싱한 후, 해당 ID의 password가 맞는지 검사한다.
           
           1. 맞는경우, dbid를 서버에 건네준다. `D2S_LoginDbid`전송
           
           2. 틀린경우, 0을 건네준다. `D2S_LoginDbid`전송
  
  - 계정 생성 시도 (`S2D_CreateAccount`를 받은 경우 처리)
    
    - 한 계정에 대한 중앙테이블, 관계 설정중.....
  
  - Server에게 player의 mmr을 조회할 수 있도록 한다.
  
  - 한 게임이 끝난 경우, 게임 결과를 기록한다.
  
  - 한 게임이 끝난 경우, 결과에 따라 각 플레이어의 mmr을 조정한다
    
    - 트랜잭션 처리 주의!

- 공통
  
  - 3가지 이상의 미니게임을 추가하고, 매치메이킹을 통해 플레이 할 수 있도록 함.
  
  - 메모리 안의 특정 int값의 변화 추이를 시각화하기.
    
    - python으로 작성한 별도의 응용 프로그램을 만들어서 서버측과 통신할 예정.
  
  - SSIS (Integration Service) 사용해서 DB 정보 옮겨보기.

---

### Until Today

- ~~Player정보를, DB에 어떻게 저장할 것인가? (어떻게 정규화 할 것인가)~~
  
  1. ~~PlayerID (문자)자체를 primary key로~~
  
  2. ~~PlayerID (int)를 primary key로, name column 추가.~~
  
  ~~챗 선생님 + 제 선생님과의 면담 끝에 테이블 하나를 사용하되.
  dbid(int) - primary key와 player_id(nvarchar(16)) - unique key(비클러스트형 인덱스)를 사용하기로 결정됨.~~

- 로그인 및 계정 생성
