using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class GameResultScene : BaseScene {
    GameResultTextUI _textUI;
    Button _sceneChangeBtn;
    TextMeshProUGUI _buttonText;
    bool _isLoaded = false;
    bool _isSceneChanging = false;

    protected override void Init() {
        base.Init();
        SceneType = Define.Scene.GameResult;
        Managers.Input.AddKeyListener(KeyCode.Return, SceneChange, InputManager.KeyState.Down);

        GameObject textUI = GameObject.Find("GameResultTextUI");
        if (textUI != null) {
            _textUI = textUI.GetComponent<GameResultTextUI>();
            if (_textUI != null)
                _textUI.Init();
        }

        GameObject sceneChangeBtn = GameObject.Find("SceneChangeButton");
        if (sceneChangeBtn != null) {
            _sceneChangeBtn = sceneChangeBtn.GetComponent<Button>();
            if (_sceneChangeBtn != null) {
                _sceneChangeBtn.onClick.AddListener(SceneChange);
                _buttonText = _sceneChangeBtn.GetComponentInChildren<TextMeshProUGUI>();

                if (_buttonText != null) 
                    _buttonText.text = "로딩중...";
                _sceneChangeBtn.interactable = false;
            }
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
        if (!_isLoaded) {
            float progress = Managers.Scene.GetLoadingProgressRate();
            if (progress > 0.8999f) {
                _isLoaded = true;
                if (_buttonText != null)
                    _buttonText.text = "로비로 이동";
                if (_sceneChangeBtn != null)
                    _sceneChangeBtn.interactable = true;
            }
        }
    }

    private void SceneChange() {
        if (_isSceneChanging || !_isLoaded) return;

        float progress = Managers.Scene.GetLoadingProgressRate();
        if (progress > 0.8999) {
            _isSceneChanging = true;
            Managers.Scene.CompleteLoadSceneAsync();
        }  
    }

    public override void Clear() {
        _sceneChangeBtn.onClick.RemoveListener(SceneChange);
        Managers.Input.RemoveKeyListener(KeyCode.Return, SceneChange, InputManager.KeyState.Down);
    }
}
