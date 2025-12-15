using UnityEngine;

public class Define {
    public enum Scene {
        Undefined,
        Login,
        Lobby,
        TestLoadingScene,
        LoadingScene1,
        Race,
        PingPong,
        Mole,
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
        Race,
        PingPong,
        Mole,
        Undefined,
        InProcess,
    }

    public static GameType IntToGameType(int type) {
        switch (type) {
            case 0: return GameType.None;
            case 1: return GameType.Race;
            case 2: return GameType.PingPong;
            case 3: return GameType.Mole;
            case 5: return GameType.InProcess;
            default: return GameType.Undefined;
        }
    }

    public static Scene IntToGameScene(int gameId) {
        switch (gameId) {
            case 1: return Scene.Race;
            case 2: return Scene.PingPong;
            case 3: return Scene.Mole;
            default: return Scene.Undefined;
        }
    }

    public static int Quota(int gameId) {
        switch (gameId) {
            case 1: return 2;
            case 2: return 4;
            case 3: return 1;
            default: return 0;
        }
    }

    public enum ObjectType {
        Undefined,
        RacePlayer,
        RaceOpponent,
        TestGameBullet,
        MyPlayerBar,
        EnemyPlayerBar,
        PingPongBulletRed,
        PingPongBulletBlue,
        PingPongBulletPupple,
    }
}
