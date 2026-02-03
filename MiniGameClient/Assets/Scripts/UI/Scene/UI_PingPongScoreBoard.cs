using NUnit.Framework;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;

public class UI_PingPongScoreBoard : UI_Scene { 
    enum Texts {
        Player1Score,
        Player2Score,
        Player1Id,
        Player2Id,
    }

    PingPongScene _pingPongScene;

    private CanvasGroup _canvasGroup;
    private bool _isVisible = true;

    private TextMeshProUGUI _player1Score;
    private TextMeshProUGUI _player2Score;
    private TextMeshProUGUI _player1Id;
    private TextMeshProUGUI _player2Id;

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    void Update() {

    }

    public override void Init() {
        base.Init();
        GameObject go = GameObject.Find("GameScene");
        _pingPongScene = go.GetComponent<PingPongScene>();
        Bind<TextMeshProUGUI>(typeof(Texts));
        _player1Id = Get<TextMeshProUGUI>((int)Texts.Player1Id);
        _player2Id = Get<TextMeshProUGUI>((int)Texts.Player2Id);
        _player1Score = Get<TextMeshProUGUI>((int)Texts.Player1Score);
        _player2Score = Get<TextMeshProUGUI>((int)Texts.Player2Score);
        _canvasGroup = GetComponent<CanvasGroup>();
        if (_canvasGroup == null) {
            _canvasGroup = gameObject.AddComponent<CanvasGroup>();
            _isVisible = true;
            _canvasGroup.alpha = 0.4f;
            _canvasGroup.interactable = false;
            _canvasGroup.blocksRaycasts = false;
        }

        Managers.Input.AddKeyListener(KeyCode.Tab, HideScoreBar, InputManager.KeyState.Down);
    }

    public void RenewScores(List<int> scores) {
        if (scores == null)
            return;

        if (scores.Count == 2) {
            _player1Score.text = scores[0].ToString();
            _player2Score.text = scores[1].ToString();
        }
    }

    private void Clear() {
        Managers.Input.RemoveKeyListener(KeyCode.Tab, HideScoreBar, InputManager.KeyState.Down);
    }

    private void HideScoreBar() {
        if (_isVisible == true) {
            _isVisible = false;
            _canvasGroup.alpha = 0f;
        }
        else {
            _isVisible = true;
            _canvasGroup.alpha = 0.4f;
        }
    }
}
