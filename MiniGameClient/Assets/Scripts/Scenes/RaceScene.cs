using Google.Protobuf.Protocol;
using System;
using System.Collections.Generic;
using UnityEngine;
using static Define;

public class RaceScene : BaseScene {
    private GameObject _tempCam;
    private RacePlayerController _myController;
    private Dictionary<int, RaceOpponentController> _opponentControllers = new();
    private GameObject _startBlock;
    private List<HammerController> _hammerControllers = new();
    private List<ObjectiveLineController> _objectiveLineControllers = new();
    private int _arrivedLineCount = 0;

    protected override void Init() {
        base.Init();
        SceneType = Scene.Race;
        Managers.Scene.ResetLoadSceneOp();
        Debug.Log("RaceScene");
        _tempCam = GameObject.Find("TempCamera");
        _startBlock = GameObject.Find("StartBlock");

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

        GameObject objLine1 = GameObject.Find("ObjectiveLine1");
        GameObject objLine2 = GameObject.Find("ObjectiveLine2");
        GameObject objLine3 = GameObject.Find("ObjectiveLine3");
        if (objLine1 != null) {
            ObjectiveLineController objLineController1 = objLine1.GetComponent<ObjectiveLineController>();
            if (objLineController1 != null) {
                objLineController1.Init(ArrivedFirstLine);
                _objectiveLineControllers.Add(objLineController1);
            }
        }
        if (objLine2 != null) {
            ObjectiveLineController objLineController2 = objLine2.GetComponent<ObjectiveLineController>();
            if (objLineController2 != null) {
                objLineController2.Init(ArrivedSecondLine);
                _objectiveLineControllers.Add(objLineController2);
            }
        }
        if (objLine3 != null) {
            ObjectiveLineController objLineController3 = objLine3.GetComponent<ObjectiveLineController>();
            if (objLineController3 != null) {
                objLineController3.Init(ArrivedFinishLine);
                _objectiveLineControllers.Add(objLineController3);
            }   
        }

        Managers.Network.TryRequestGameState((int)GameType.Race);
    }

    public void OffTheTempCam() {
        _tempCam.SetActive(false);
    }

    public void CountdownBeforeStart(int count) {
        if (count == 0) {
            _startBlock.SetActive(false);
            ShowCountDownUIEffect("Go");
        }
        else {
            ShowCountDownUIEffect(count.ToString());
        }     
    }

    private void ShowCountDownUIEffect(string msg) {
        Debug.Log($"{msg}...");
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

    public void ArrivedFirstLine() { 
        if (_arrivedLineCount != 0)
            return;

        Managers.Network.Race.ArrivedInLine(1);
    }

    public void ArrivedSecondLine() {
        if (_arrivedLineCount != 1)
            return;

        Managers.Network.Race.ArrivedInLine(2);
    }

    public void ArrivedFinishLine() {
        if (_arrivedLineCount != 2)
            return;

        Managers.Network.Race.ArrivedInLine(3);
    }

    public void ForceMovePlayer(Vector3 pos) {
        _myController.ForceMovePlayer(pos);
    }

    public void ConfirmArrivedLine(int lineId) {
        Debug.Log($"ConfirmArrivedLine: {lineId}");
        _arrivedLineCount = lineId;
    }

    public void EndGame(bool isWinner, int winnerIdx) { 
        //TODO: 3ÃÊ Scene Á¤Áö.
        Managers.Network.Match.ResetMatchState();
        Managers.Scene.EndGame(isWinner, winnerIdx);
    }

    public override void Clear() {
        
    }
}
