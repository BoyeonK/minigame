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

    public void UpdateMovementAndCollision(Vector3 nestedForce, List<GameObjectMovementInfo> movementInfos) {
        _myController.ApplyCollisionForceVector(nestedForce);

        //movementInfos를 읽어서 다른 플레이어 피상 위치 변경하기
        foreach (GameObjectMovementInfo movementInfo in movementInfos) {
            RaceOpponentController oppo;
            _opponentControllers.TryGetValue(movementInfo.ObjectId, out oppo);
            Vector3 pos = new(movementInfo.Position.X, movementInfo.Position.Y, movementInfo.Position.Z);
            Vector3 front = new(movementInfo.Front.X, movementInfo.Front.Y, movementInfo.Front.Z);
            Vector3 velocity = new(movementInfo.Velocity.X, movementInfo.Velocity.Y, movementInfo.Velocity.Z);
            int state = movementInfo.State;
            if (oppo != null) {
                oppo.SetMovementInfo(pos, front, velocity, state);
            }
        }
    }

    public GameObjectMovementInfo SerializeMyMovementStateAndCollision() {
        //TODO: 내 유닛의 움직임을 직렬화하기
        GameObjectMovementInfo serializedInfo = new();
        return serializedInfo;
    }

    public override void Clear() {
        
    }
}
