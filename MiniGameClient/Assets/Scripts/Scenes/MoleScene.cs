using UnityEngine;
using static Define;

public class MoleScene : BaseScene {
    MoleGameBoardController _gameBoard;

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
            
    }

    public void LoadState(int playerIdx) {

    }

    public void SetSlotState(int slotIdx, int state) { 
        _gameBoard.SetSlotState(slotIdx, state);
    }

    public override void Clear() {
        
    }
}
