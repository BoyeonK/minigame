using Google.Protobuf.Protocol;
using System.Collections.Generic;
using UnityEngine;
using static Define;

public class RaceScene : BaseScene {
    private GameObject _tempCam;
    private RacePlayerController _myController;
    private Dictionary<int, RaceOpponentController> _opponentControllers = new();

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

    public void RegisterMyController(GameObject obj) {
        if (obj != null)
            _myController = obj.GetComponent<RacePlayerController>();
    }

    public void RegisterOppoController(int objId, GameObject obj) {
        if (obj != null) {
            RaceOpponentController roc = obj.GetComponent<RaceOpponentController>();
            if (roc != null)
                _opponentControllers[objId] = roc;
        }
    }

    public void UpdateCollision(Vector3 nestedForce) {
        _myController.ApplyCollisionForceVector(nestedForce);
    }

    public void UpdateMovement(int objectId, Vector3 pos, Vector3 front, Vector3 vel, int state) {
        RaceOpponentController oppo;
        _opponentControllers.TryGetValue(objectId, out oppo);

        if (oppo != null) {
            oppo.SetMovementInfo(pos, front, vel, state);
            Debug.Log($"oppo : {objectId}");
        }
        else {
            Debug.Log($"oppo is null, there's no objID : { objectId }");
        }
    }

    public GameObjectMovementInfo SerializeMyMovementStateAndCollision() {
        //TODO: 내 유닛의 움직임을 직렬화하기
        return _myController.SerializeMyMovementInfo();
    }

    public override void Clear() {
        
    }
}
