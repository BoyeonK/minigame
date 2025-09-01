using UnityEngine;

public class Define {
    public enum Scene {
        Unknown,
        Login,
        Lobby,
        Game,
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
    }

    public static GameType IntToGameType(int type) {
        switch (type) {
            case 0: return GameType.None;
            case 1: return GameType.TestMatch;
            case 2: return GameType.PingPong;
            case 3: return GameType.Danmaku;
            default: return GameType.Undefined;
        }
    }
}
