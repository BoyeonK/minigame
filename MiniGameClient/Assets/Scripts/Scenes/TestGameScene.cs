using UnityEngine;

public class TestGameScene : BaseScene {
    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.TestGame;
        Managers.Scene.ResetLoadSceneOp();
        Debug.Log("TestGameScene");
        //Managers.Network.Try
    }

    public override void Clear() {
        
    }
}
