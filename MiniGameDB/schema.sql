/*
    MiniGameDB 스키마 복원 스크립트

    원본 DB가 유실되어, MiniGameDB의 C++ 코드에서 실제로 실행되는 쿼리와
    바인딩 타입/버퍼 크기를 역으로 읽어 재구성한 것이다.
    근거가 된 위치를 각 테이블에 주석으로 남겨둔다.

    사용법 (sqlcmd):
        sqlcmd -S localhost\SQLEXPRESS -E -i schema.sql
    또는 SSMS에서 이 파일을 열고 실행.

    DB 이름 MyGameDB 는 유실 전 .env 에서 실제로 쓰이던 값이다.
    (프로젝트 이름이 MiniGameDB 인 것과 다르니 주의)
    바꾸려면 .env 의 DB_NAME 과 반드시 함께 바꿔야 한다.
*/

--------------------------------------------------------------------------------
-- 데이터베이스
--------------------------------------------------------------------------------
IF DB_ID(N'MyGameDB') IS NULL
    CREATE DATABASE MyGameDB;
GO

USE MyGameDB;
GO

--------------------------------------------------------------------------------
-- Players
--   INSERT INTO Players (player_id) VALUES (?)      -> dbid는 IDENTITY
--   SELECT dbid FROM Players WHERE player_id = ?
--   SELECT player_id FROM Players WHERE dbid = ?
--
--   player_id 길이 16: CallData.cpp GetRecorderId의 SQLWCHAR playerIdBuffer[17]
--   (널 종료 문자 1개 포함) 에서 역산.
--   계정 생성 시 중복 아이디는 SQLSTATE 23000으로 판별하므로 UNIQUE 제약이 필수다.
--------------------------------------------------------------------------------
IF OBJECT_ID(N'dbo.Players', N'U') IS NULL
CREATE TABLE dbo.Players (
    dbid      INT IDENTITY(1,1) NOT NULL,
    player_id NVARCHAR(16)      NOT NULL,
    CONSTRAINT PK_Players     PRIMARY KEY (dbid),
    CONSTRAINT UQ_Players_pid UNIQUE (player_id)
);
GO

--------------------------------------------------------------------------------
-- Accounts
--   INSERT INTO Accounts (dbid, password_hash, salt) VALUES (?, ?, ?)
--   SELECT password_hash, salt FROM Accounts WHERE dbid = ?
--
--   GlobalVariables.h: salt_size = 16, hash_size = 32, pbkdf2_iter = 10000
--   해시/솔트는 v2wsRef()로 바이트당 2자리 16진 문자열로 변환해 저장하므로
--   각각 64자, 32자가 된다. (ScandinavianFlick의 HASH_SIZE=64 / SALT_SIZE=32와 일치)
--------------------------------------------------------------------------------
IF OBJECT_ID(N'dbo.Accounts', N'U') IS NULL
CREATE TABLE dbo.Accounts (
    dbid          INT          NOT NULL,
    password_hash NVARCHAR(64) NOT NULL,
    salt          NVARCHAR(32) NOT NULL,
    CONSTRAINT PK_Accounts         PRIMARY KEY (dbid),
    CONSTRAINT FK_Accounts_Players FOREIGN KEY (dbid) REFERENCES dbo.Players(dbid)
);
GO

--------------------------------------------------------------------------------
-- Elos
--   INSERT INTO Elos (dbid) VALUES (?)          -> elo 컬럼은 DEFAULT로 채워진다
--   SELECT elo1, elo2, elo3 FROM Elos WHERE dbid = ?
--   UPDATE Elos SET elo{1|2|3} = ? WHERE dbid = ?
--
--   기본값 1200은 S2D_Protocol.proto의 계정 생성 절차 주석에 명시되어 있다.
--   컬럼 번호는 gameId와 1:1 (1=Race, 2=PingPong, 3=Mole).
--------------------------------------------------------------------------------
IF OBJECT_ID(N'dbo.Elos', N'U') IS NULL
CREATE TABLE dbo.Elos (
    dbid INT NOT NULL,
    elo1 INT NOT NULL CONSTRAINT DF_Elos_elo1 DEFAULT (1200),
    elo2 INT NOT NULL CONSTRAINT DF_Elos_elo2 DEFAULT (1200),
    elo3 INT NOT NULL CONSTRAINT DF_Elos_elo3 DEFAULT (1200),
    CONSTRAINT PK_Elos         PRIMARY KEY (dbid),
    CONSTRAINT FK_Elos_Players FOREIGN KEY (dbid) REFERENCES dbo.Players(dbid)
);
GO

--------------------------------------------------------------------------------
-- PersonalRecords
--   INSERT INTO PersonalRecords (dbid) VALUES (?)
--   SELECT score1, score2, score3 FROM PersonalRecords WHERE dbid = ?
--   UPDATE PersonalRecords SET score{1|2|3} = ? WHERE dbid = ?
--------------------------------------------------------------------------------
IF OBJECT_ID(N'dbo.PersonalRecords', N'U') IS NULL
CREATE TABLE dbo.PersonalRecords (
    dbid   INT NOT NULL,
    score1 INT NOT NULL CONSTRAINT DF_PR_score1 DEFAULT (0),
    score2 INT NOT NULL CONSTRAINT DF_PR_score2 DEFAULT (0),
    score3 INT NOT NULL CONSTRAINT DF_PR_score3 DEFAULT (0),
    CONSTRAINT PK_PersonalRecords         PRIMARY KEY (dbid),
    CONSTRAINT FK_PersonalRecords_Players FOREIGN KEY (dbid) REFERENCES dbo.Players(dbid)
);
GO

--------------------------------------------------------------------------------
-- PublicRecords  (전역 최고기록. 행이 정확히 1개인 테이블)
--   SELECT dbid{n}, score{n} FROM PublicRecords
--   UPDATE PublicRecords SET dbid{n} = ?, score{n} = ?     <- WHERE 절이 없다
--
--   WHERE가 없다는 점이 이 테이블이 단일 행임을 보여준다.
--   주의: 서버는 시작 시 RenewPublicRecordFromDB()로 이 값을 읽고,
--   CallData.cpp의 GetRecorderId()가 dbid로 Players를 조회한다.
--   매칭되는 Player가 없으면 예외("Player not found with the given dbid")를 던지므로
--   빈 DB에서도 조회가 성립하도록 아래에서 자리표시자 계정을 함께 시드한다.
--------------------------------------------------------------------------------
IF OBJECT_ID(N'dbo.PublicRecords', N'U') IS NULL
CREATE TABLE dbo.PublicRecords (
    dbid1  INT NOT NULL CONSTRAINT DF_PubR_dbid1  DEFAULT (0),
    score1 INT NOT NULL CONSTRAINT DF_PubR_score1 DEFAULT (0),
    dbid2  INT NOT NULL CONSTRAINT DF_PubR_dbid2  DEFAULT (0),
    score2 INT NOT NULL CONSTRAINT DF_PubR_score2 DEFAULT (0),
    dbid3  INT NOT NULL CONSTRAINT DF_PubR_dbid3  DEFAULT (0),
    score3 INT NOT NULL CONSTRAINT DF_PubR_score3 DEFAULT (0)
);
GO

--------------------------------------------------------------------------------
-- CRUD  (Debug 빌드 전용 스모크 테스트 대상)
--   GlobalVariables.cpp의 DBManager 생성자가 _DEBUG에서
--   InitialC/R/U/D를 실행한다. 이 테이블이 없으면 시작할 때마다
--   ODBC 에러가 출력된다. (예외는 잡히므로 죽지는 않는다)
--------------------------------------------------------------------------------
IF OBJECT_ID(N'dbo.CRUD', N'U') IS NULL
CREATE TABLE dbo.CRUD (
    id    INT NOT NULL,
    value INT NULL
);
GO

--------------------------------------------------------------------------------
-- 시드 데이터
--   1) 최고기록 자리표시자 계정. 아직 아무도 기록을 세우지 않은 상태를 표현한다.
--      Players에만 있으면 GetRecorderId()가 성립한다(Accounts 행은 불필요 =
--      이 계정으로는 로그인할 수 없다).
--   2) PublicRecords 단일 행. 점수 0, 기록자 = 자리표시자.
--------------------------------------------------------------------------------
IF NOT EXISTS (SELECT 1 FROM dbo.Players WHERE player_id = N'-')
    INSERT INTO dbo.Players (player_id) VALUES (N'-');
GO

IF NOT EXISTS (SELECT 1 FROM dbo.PublicRecords)
BEGIN
    DECLARE @seed INT = (SELECT dbid FROM dbo.Players WHERE player_id = N'-');
    INSERT INTO dbo.PublicRecords (dbid1, score1, dbid2, score2, dbid3, score3)
    VALUES (@seed, 0, @seed, 0, @seed, 0);
END
GO

--------------------------------------------------------------------------------
-- 확인
--------------------------------------------------------------------------------
SELECT name AS created_table
FROM sys.tables
WHERE name IN (N'Players', N'Accounts', N'Elos', N'PersonalRecords', N'PublicRecords', N'CRUD')
ORDER BY name;
GO

SELECT * FROM dbo.PublicRecords;
GO
