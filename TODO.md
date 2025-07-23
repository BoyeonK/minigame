### TODO List

- Server
  
  - ~~RSA key값들을 미리 생성해서 pool을 만들어 둔다.~~
  - ~~client와 socket연결 성공시 (on connected 단계) public key를 던져 준다.~~
    - ~~TODO : DER방식으로 인코딩해야 함.~~
  - 클라이언트로부터 AES key를 받고, 해당 Key로서 특정 문자열을 암호화하여 클라이언트에게 전송한다.

- Client
  
  - ~~특정 키 입력시, 서버에 연결 시도.~~
    
    - ~~연결 성공시 RSA public key를 받고, AES key를 생성해서 RSA public key로 암호화해서 전송.~~
      
      - ~~생성한 AES key는, session에서 저장하여야 함.~~
    
    - 이후, 서버로부터 받은 바이너리를 복호화하여 특정 문자열을 받는데 성공하면 전체적인 연결 성공으로 간주. (특정 bool값을 true로 바꾼다.)

- 공통
  
  - 메모리 안의 특정 int값의 변화 추이를 시각화하기.
    - python으로 작성한 별도의 응용 프로그램을 만들어서 서버측과 통신할 예정.
  - Microsoft SQL server를 사용할 예정
    - gRPC를 활용하여 DB조작하기
      - 트랜잭션, 실패시 예외처리 주의
    - SSMS (SQL Server Management Studio) 사용해보기
      - SSIS (Integration Service) 사용해보기 (우선순위 낮음.)

ㅇ

### Until Today
