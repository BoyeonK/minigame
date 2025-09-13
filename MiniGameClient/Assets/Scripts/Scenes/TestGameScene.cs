using UnityEngine;
using static Define;

public class TestGameScene : BaseScene {
    protected override void Init() {
        base.Init();
        SceneType = Scene.TestGame;
        Managers.Scene.ResetLoadSceneOp();
        Debug.Log("TestGameScene");
        Managers.Network.TryRequestGameState((int)GameType.TestMatch);
    }

    public override void Clear() {
        
    }
}
