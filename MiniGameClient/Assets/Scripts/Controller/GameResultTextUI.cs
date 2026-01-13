using NUnit.Framework;
using System.Collections.Generic;
using TMPro;
using UnityEditor;
using UnityEngine;

public class GameResultTextUI : MonoBehaviour {
    List<TextMeshProUGUI> _playerIds = new List<TextMeshProUGUI>();
    List<TextMeshProUGUI> _scores = new List<TextMeshProUGUI>();
    TextMeshProUGUI _isWinner;

    public void Init() {
        Transform isWinnerTransform = transform.Find("IsWinner");
        if (isWinnerTransform != null) {
            _isWinner = isWinnerTransform.GetComponent<TextMeshProUGUI>();
        }

        for (int i = 1; i <= 4; i++) {
            string playerString = $"PlayerId{i}";
            string scoreString = $"Score{i}";
            Transform playerIdTransform = transform.Find(playerString);
            Transform scoreTransform = transform.Find(scoreString);
            if (playerIdTransform != null) {
                TextMeshProUGUI id = playerIdTransform.GetComponent<TextMeshProUGUI>();
                if (id != null) 
                    _playerIds.Add(id);
            }
            if (scoreTransform != null) {
                TextMeshProUGUI score = scoreTransform.GetComponent<TextMeshProUGUI>();
                if (score != null)
                    _scores.Add(score);
            }
        }
    }

    public void SetResultScore() {
        if (_isWinner != null) {
            if (Managers.Scene._isWinner) {
                _isWinner.text = "½Â¸®";
            }
            else {
                _isWinner.text = "ÆÐ¹è";
            }
        }

        int pSize = Managers.Scene._playerIds.Count;
        int sSize = Managers.Scene._scores.Count;
        if (pSize == 0 || pSize > 4)
            return;

        if (sSize == 0 || sSize > 4)
            return;

        for (int i = 0; i < pSize; i++) {
            _playerIds[i].text = Managers.Scene._playerIds[i];
        }
        for (int i = pSize; i < 4; i++) {
            _playerIds[i].text = "";
        }
        for (int i = 0; i < sSize; i++) {
            _scores[i].text = Managers.Scene._scores[i].ToString();
        }
        for (int i = sSize; i < 4; i++) {
            _scores[i].text = "";
        }
    }

    public void SetResultWinnerIdx() {
        if (_isWinner != null)
            _isWinner.text = "";

        int pSize = Managers.Scene._playerIds.Count;
        if (pSize == 0 || pSize > 4)
            return;

        for (int i = 0; i < pSize; i++) {
            _playerIds[i].text = Managers.Scene._playerIds[i];
        }
        for (int i = pSize; i < 4; i++) {
            _playerIds[i].text = "";
        }
        for (int i = 0; i < pSize; i++) {
            _scores[i].text = (Managers.Scene._winnerIdx == i) ? "½Â¸®" : "ÆÐ¹è";
        }
        for (int i = pSize; i < 4; i++) {
            _scores[i].text = "";
        }
    }
}
