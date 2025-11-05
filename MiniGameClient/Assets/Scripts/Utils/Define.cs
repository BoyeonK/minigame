using UnityEngine;

public class Define {
    public enum Scene {
        Undefined,
        Login,
        Lobby,
        TestLoadingScene,
        TestGame,
        PingPong,
        GameResult,
    }

    public enum Sound {
        Bgm,
        Effect,
        MaxCount,
    }

    public enum UIEvent {
        Click,
        Drag,
    }

    public enum MouseEvent {
        Press,
        Click,
    }
    /*
    public enum CameraMode {
        QuaterView,
    }
    */

    public enum GameType {
        None,
        TestMatch,
        PingPong,
        Danmaku,
        Undefined,
        InProcess,
    }

    public static GameType IntToGameType(int type) {
        switch (type) {
            case 0: return GameType.None;
            case 1: return GameType.TestMatch;
            case 2: return GameType.PingPong;
            case 3: return GameType.Danmaku;
            case 5: return GameType.InProcess;
            default: return GameType.Undefined;
        }
    }

    public static Scene IntToGameScene(int gameId) {
        switch (gameId) {
            case 1: return Scene.TestGame;
            case 2: return Scene.PingPong;
            default: return Scene.Undefined;
        }
    }

    public enum ObjectType {
        Undefined,
        TestGameBullet,
        MyPlayerBar,
        EnemyPlayerBar,
        PingPongBulletRed,
        PingPongBulletBlue,
        PingPongBulletPupple,
    }
}
