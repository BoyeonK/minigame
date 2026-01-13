using UnityEngine;

public class GameResultScene : BaseScene {
    GameResultTextUI _textUI;

    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.GameResult;
        Managers.Input.AddKeyListener(KeyCode.Tab, SceneChange, InputManager.KeyState.Down);

        GameObject textUI = GameObject.Find("GameResultTextUI");
        if (textUI != null) {
            _textUI = textUI.GetComponent<GameResultTextUI>();
        }
    }

    private void Start() {
        Debug.Log("게임 결과창 스타트함수");
        Managers.Scene.LoadSceneAsync();

        if (Managers.Scene._isScoreResult)
            _textUI.SetResultScore();
        else
            _textUI.SetResultWinnerIdx();
    }

    private void Update() {

    }

    private void SceneChange() {
        float progress = Managers.Scene.GetLoadingProgressRate();
        if (progress > 0.8999) 
            Managers.Scene.CompleteLoadSceneAsync();
    }

    public override void Clear() {
        Managers.Input.RemoveKeyListener(KeyCode.Tab, SceneChange, InputManager.KeyState.Down);
    }
}
