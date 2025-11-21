using UnityEngine;
using static Define;

public class MoleScene : BaseScene {
    private MoleGameBoardController _gameBoard;
    private MoleCameraController _cameraController;
    private StunBlur _stunBlur;
    
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
    }

    public void LoadState(int playerIdx) {

    }

    public void SetSlotState(int slotIdx, int state) { 
        _gameBoard.SetSlotState(slotIdx, state);
    }

    public void Stun() {
        _cameraController.Stun();
        _stunBlur.Stun(); 
    }

    public override void Clear() {
        
    }
}
