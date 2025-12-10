using UnityEngine;
using static Define;

public class RaceScene : BaseScene {
    protected override void Init() {
        base.Init();
        SceneType = Scene.Race;
        Managers.Scene.ResetLoadSceneOp();
        Debug.Log("RaceScene");
        Managers.Network.TryRequestGameState((int)GameType.Race);
    }

    public override void Clear() {
        
    }
}
