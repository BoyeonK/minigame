using UnityEngine;
using static Define;

public class TestGameScene : BaseScene {
    protected override void Init() {
        base.Init();
        SceneType = Scene.TestGame;
        Managers.Scene.ResetLoadSceneOp();
        Debug.Log("TestGameScene");
        Managers.Network.TryRequestGameState((int)GameType.TestMatch);
        Managers.Network.OnTestGameEndAct += EndGame;
    }

    private void EndGame() {
        Application.Quit();
#if UNITY_EDITOR
        UnityEditor.EditorApplication.isPlaying = false;
#endif
    }

    public override void Clear() {
        Managers.Network.OnTestGameEndAct -= EndGame;
    }
}
