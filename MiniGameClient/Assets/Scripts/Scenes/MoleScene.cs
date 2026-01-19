using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using static Define;

public class MoleScene : BaseScene {
    MoleGameBoardController _gameBoard;
    MoleCameraController _cameraController;
    StunBlur _stunBlur;
    UI_MoleScoreBoard _uiMoleScoreBoard;
    UI_Mole_EndGame _uiEndGame;
    UI_PrintMessage _uiPrintMessage;

    protected override void Init() {
        base.Init();
        SceneType = Scene.Mole;
        Managers.Scene.ResetLoadSceneOp();
        Debug.Log("MoleScene");
        Managers.Network.TryRequestGameState((int)GameType.Mole);

        GameObject goGameBoard = GameObject.Find("GameBoard");
        if (goGameBoard != null) {
            _gameBoard = goGameBoard.GetComponent<MoleGameBoardController>();
            _gameBoard.Init();
        }

        GameObject camera = GameObject.Find("MoleCamera");
        if (camera != null) {
            _cameraController = camera.GetComponent<MoleCameraController>();
            _cameraController.Init();
        }

        GameObject stunBlur = GameObject.Find("StunBlur");
        if (stunBlur != null) {
            _stunBlur = stunBlur.GetComponent<StunBlur>();
            _stunBlur.Init();
        }

        _uiMoleScoreBoard = Managers.UI.ShowSceneUI<UI_MoleScoreBoard>();
        _uiEndGame = Managers.UI.CacheSceneUI<UI_Mole_EndGame>();
        _uiPrintMessage = Managers.UI.CacheSceneUI<UI_PrintMessage>();
    }

    public void LoadState(int playerIdx, List<string> playerIds) {
        _uiMoleScoreBoard.SetPlayerIds(playerIds);
    }

    public void SetSlotState(int slotIdx, int state) { 
        _gameBoard.SetSlotState(slotIdx, state);
    }

    public void Stun() {
        _cameraController.Stun();
        _stunBlur.Stun(); 
    }

    public void RenewScores(List<int> scores) {
        _uiMoleScoreBoard.RenewScores(scores);
    }

    public void EndGame(bool isWinner, List<int> scores) {
        Managers.UI.ShowSceneUI<UI_Mole_EndGame>();
        StartCoroutine(EndGameRoutine(isWinner, scores));
    }

    private IEnumerator EndGameRoutine(bool isWinner, List<int> scores) {
        Time.timeScale = 0f;

        yield return new WaitForSecondsRealtime(2f);

        Time.timeScale = 1f;

        Managers.Network.Match.ResetMatchState();
        Managers.Scene.EndGame(isWinner, scores);
    }

    public void CountdownBeforeStart(int countdown) {
        if (countdown == 0) {
            _uiPrintMessage.SetTextTop("Ω√¿€!");
            _uiPrintMessage.SetTextColorTop(255, 0, 0, 255);
            StartCoroutine(CountdownClearRoutine(0.5f));
        }
        else {
            Managers.UI.ShowSceneUI<UI_PrintMessage>();
            _uiPrintMessage.SetTextTop(countdown.ToString());
        }
    }

    private IEnumerator CountdownClearRoutine(float delay) {
        yield return new WaitForSeconds(delay);
        Managers.UI.DisableUI("UI_PrintMessage");
    }

    public override void Clear() {
        
    }
}
