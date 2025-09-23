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
        Managers.Scene.LoadScene(Scene.Login);
    }

    public override void Clear() {
        Managers.Network.OnTestGameEndAct -= EndGame;
    }
}
