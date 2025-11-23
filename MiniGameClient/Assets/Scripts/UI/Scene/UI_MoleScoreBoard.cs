using NUnit.Framework;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;

public class UI_MoleScoreBoard : UI_Scene { 
    enum Texts {
        Player0Score,
        Player1Score,
        Player0Id,
        Player1Id,
    }

    MoleScene _moleScene;

    private CanvasGroup _canvasGroup;
    private bool _isVisible = true;

    private TextMeshProUGUI _player0Id;
    private TextMeshProUGUI _player1Id;
    private TextMeshProUGUI _player0Score;
    private TextMeshProUGUI _player1Score;

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
        _moleScene = go.GetComponent<MoleScene>();
        Bind<TextMeshProUGUI>(typeof(Texts));
        _player0Id = Get<TextMeshProUGUI>((int)Texts.Player0Id);
        _player1Id = Get<TextMeshProUGUI>((int)Texts.Player1Id);
        _player0Score = Get<TextMeshProUGUI>((int)Texts.Player0Score);
        _player1Score = Get<TextMeshProUGUI>((int)Texts.Player1Score);
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
            _player0Score.text = scores[0].ToString();
            _player1Score.text = scores[1].ToString();
        }
    }

    public void SetPlayerIds(List<string> playerIds) {
        if (playerIds == null)
            return;

        if (playerIds.Count == 2) {
            _player0Id.text = playerIds[0].ToString();
            _player1Id.text = playerIds[1].ToString();
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
