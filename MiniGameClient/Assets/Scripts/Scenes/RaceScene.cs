using Google.Protobuf.Protocol;
using System.Collections.Generic;
using UnityEngine;
using static Define;

public class RaceScene : BaseScene {
    private GameObject _tempCam;
    private RacePlayerController _myController;
    private Dictionary<int, RaceOpponentController> _opponentControllers = new();
    private List<HammerController> _hammerControllers = new();

    protected override void Init() {
        base.Init();
        SceneType = Scene.Race;
        Managers.Scene.ResetLoadSceneOp();
        Debug.Log("RaceScene");
        _tempCam = GameObject.Find("TempCamera");

        GameObject LHammerObj1 = GameObject.Find("LeftHammer1");
        GameObject LHammerObj2 = GameObject.Find("LeftHammer2");
        GameObject RHammerObj1 = GameObject.Find("RightHammer1");
        GameObject RHammerObj2 = GameObject.Find("RightHammer2");

        if (LHammerObj1 != null) {
            HammerController hammer = LHammerObj1.GetComponent<HammerController>();
            if (hammer != null)
                _hammerControllers.Add(hammer);
        }
        if (LHammerObj2 != null) {
            HammerController hammer = LHammerObj2.GetComponent<HammerController>();
            if (hammer != null)
                _hammerControllers.Add(hammer);
        }
        if (RHammerObj1 != null) {
            HammerController hammer = RHammerObj1.GetComponent<HammerController>();
            if (hammer != null)
                _hammerControllers.Add(hammer);
        }
        if (RHammerObj2 != null) {
            HammerController hammer = RHammerObj2.GetComponent<HammerController>();
            if (hammer != null)
                _hammerControllers.Add(hammer);
        }

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

        if (oppo != null)
            oppo.SetMovementInfo(pos, front, vel, state);
    }

    public GameObjectMovementInfo SerializeMyMovementStateAndCollision() {
        return _myController.SerializeMyMovementInfo();
    }

    public void OperateObstacle(int obstacleId, int triggerId) {
        if (obstacleId < 0 || obstacleId >= _hammerControllers.Count)
            return;
        
        if (triggerId == 0)
            _hammerControllers[obstacleId].SwingToLeft();
        else if (triggerId == 1)
            _hammerControllers[obstacleId].SwingToRight();
    }

    public override void Clear() {
        
    }
}
