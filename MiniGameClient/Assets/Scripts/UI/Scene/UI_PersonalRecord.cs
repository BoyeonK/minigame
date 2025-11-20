using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;

public class UI_PersonalRecord : UI_Scene {
    List<TextMeshProUGUI> _scores = new List<TextMeshProUGUI>();

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    public override void Init() {
        base.Init();
        _scores.Clear();

        List<TextMeshProUGUI> allTexts = new List<TextMeshProUGUI>();
        GetComponentsInChildren<TextMeshProUGUI>(true, allTexts);
        foreach (TextMeshProUGUI text in allTexts) {
            if (text.name.StartsWith("Score")) {
                _scores.Add(text);
            }
        }
        _scores.Sort((a, b) => string.Compare(a.name, b.name));

        List<int> scores = Managers.Network.Lobby.GetPersonalRecord();
        for (int i = 0; i < scores.Count; i++) {
            if (i >= _scores.Count)
                break;
            _scores[i].text = scores[i].ToString();
        }
    }

    private void Clear() {
        
    }
}
