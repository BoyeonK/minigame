using UnityEngine;
using static Define;

public class RaceScene : BaseScene {
    private GameObject _tempCam;

    protected override void Init() {
        base.Init();
        SceneType = Scene.Race;
        Managers.Scene.ResetLoadSceneOp();
        Debug.Log("RaceScene");
        _tempCam = GameObject.Find("TempCamera");
        Managers.Network.TryRequestGameState((int)GameType.Race);
    }

    public void OffTheTempCam() {
        _tempCam.SetActive(false);
    }

    public override void Clear() {
        
    }
}
