using UnityEngine;
using static Define;

public class MoleScene : BaseScene {
    protected override void Init() {
        base.Init();
        SceneType = Scene.Mole;
        Managers.Scene.ResetLoadSceneOp();
        Debug.Log("MoleScene");
        Managers.Network.TryRequestGameState((int)GameType.Mole);
    }

    public void LoadState(int playerIdx) {

    }

    public override void Clear() {
        
    }
}
