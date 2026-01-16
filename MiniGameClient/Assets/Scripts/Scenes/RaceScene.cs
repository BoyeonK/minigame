using Google.Protobuf.Protocol;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using static Define;

public class RaceScene : BaseScene {
    UI_Race_EndGame _uiEndGame;
    UI_PrintMessage _uiPrintMessage;
    GameObject _tempCam;
    RacePlayerController _myController;
    Dictionary<int, RaceOpponentController> _opponentControllers = new();
    GameObject _startBlock;
    List<HammerController> _hammerControllers = new();
    List<BridgeController> _bridgeControllers = new();
    List<ObjectiveLineController> _objectiveLineControllers = new();
    int _arrivedLineCount = 0;

    protected override void Init() {
        base.Init();
        SceneType = Scene.Race;
        Managers.Scene.ResetLoadSceneOp();
        Debug.Log("RaceScene");
        _tempCam = GameObject.Find("TempCamera");
        _startBlock = GameObject.Find("StartBlock");

        GameObject secondLineObj = GameObject.Find("SecondLine");
        if (secondLineObj != null) {
            Transform lh1Trans = secondLineObj.transform.Find("LeftHammer1");
            HammerController hammer1 = lh1Trans.GetComponent<HammerController>();
            if (hammer1 != null)
                _hammerControllers.Add(hammer1);

            Transform lh2Trans = secondLineObj.transform.Find("LeftHammer2");
            HammerController hammer2 = lh1Trans.GetComponent<HammerController>();
            if (hammer2 != null)
                _hammerControllers.Add(hammer1);

            Transform rh1Trans = secondLineObj.transform.Find("RightHammer1");
            HammerController hammer3 = lh1Trans.GetComponent<HammerController>();
            if (hammer3 != null)
                _hammerControllers.Add(hammer1);

            Transform rh2Trans = secondLineObj.transform.Find("RightHammer2");
            HammerController hammer4 = lh1Trans.GetComponent<HammerController>();
            if (hammer4 != null)
                _hammerControllers.Add(hammer1);
        }

        GameObject fourthLineObj = GameObject.Find("FourthLine");
        if (fourthLineObj != null) {
            Transform br1Trans = fourthLineObj.transform.Find("Bridge1");
            BridgeController bridge1 = br1Trans.GetComponent<BridgeController>();
            if (bridge1 != null) {
                bridge1.Init();
                _bridgeControllers.Add(bridge1);
            }
                

            Transform br2Trans = fourthLineObj.transform.Find("Bridge2");
            BridgeController bridge2 = br2Trans.GetComponent<BridgeController>();
            if (bridge2 != null) {
                bridge2.Init();
                _bridgeControllers.Add(bridge2);
            }

            Transform br3Trans = fourthLineObj.transform.Find("Bridge3");
            BridgeController bridge3 = br3Trans.GetComponent<BridgeController>();
            if (bridge3 != null) {
                bridge3.Init();
                _bridgeControllers.Add(bridge3);
            }

            Transform br4Trans = fourthLineObj.transform.Find("Bridge4");
            BridgeController bridge4 = br4Trans.GetComponent<BridgeController>();
            if (bridge4 != null) {
                bridge4.Init();
                _bridgeControllers.Add(bridge4);
            }
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

        _uiEndGame = Managers.UI.CacheSceneUI<UI_Race_EndGame>();
        _uiPrintMessage = Managers.UI.CacheSceneUI<UI_PrintMessage>();

        Managers.Network.TryRequestGameState((int)GameType.Race);
    }

    public void OffTheTempCam() {
        _tempCam.SetActive(false);
    }

    public void CountdownBeforeStart(int count) {
        if (count == 0) {
            _startBlock.SetActive(false);
            _uiPrintMessage.SetTextTop("Go!");
            _uiPrintMessage.SetTextColorTop(255, 0, 0, 255);
            StartCoroutine(CountdownClearRoutine(0.5f));
        }
        else {
            Managers.UI.ShowSceneUI<UI_PrintMessage>();
            _uiPrintMessage.SetTextTop(count.ToString());
        }     
    }

    private IEnumerator CountdownClearRoutine(float delay) {
        yield return new WaitForSeconds(delay);
        Managers.UI.DisableUI("UI_PrintMessage");
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
        if (obstacleId == 0) {
            if (_hammerControllers.Count != 4)
                return;

            if (triggerId == 0) {
                _hammerControllers[0].SwingToLeft();
                _hammerControllers[1].SwingToRight();
                _hammerControllers[2].SwingToRight();
                _hammerControllers[3].SwingToLeft();
            }
            else if (triggerId == 1) {
                _hammerControllers[0].SwingToRight();
                _hammerControllers[1].SwingToLeft();
                _hammerControllers[2].SwingToLeft();
                _hammerControllers[3].SwingToRight();
            }
        }
        else if (obstacleId == 1) {
            if (_bridgeControllers.Count != 4)
                return;
            for (int i = 0; i < 4; i++) {
                bool isActive = (triggerId & (1 << i)) != 0;

                if (isActive)
                    _bridgeControllers[i].EnableBridgeCollider();
                else
                    _bridgeControllers[i].DisableBridgeColliderAfterSecond(2f);
            }
        }
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
        Managers.UI.ShowSceneUI<UI_Race_EndGame>();

        StartCoroutine(EndGameRoutine(isWinner, winnerIdx));
    }

    private IEnumerator EndGameRoutine(bool isWinner, int winnerIdx) {
        Time.timeScale = 0f;

        yield return new WaitForSecondsRealtime(2f);

        Time.timeScale = 1f;

        Managers.Network.Match.ResetMatchState();
        Managers.Scene.EndGame(isWinner, winnerIdx);
    }

    public override void Clear() {
        
    }
}
