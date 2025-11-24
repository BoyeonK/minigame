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

        Managers.Network.Lobby.TryGetPersonalRecords();
    }

    public void BindRecord() {
        List<int> records = Managers.Network.Lobby.GetPersonalRecord();
        for (int i = 0; i < records.Count; i++) {
            if (i > 2) break;
            _scores[i].text = records[i].ToString();
        }
        Debug.Log("sdf");
    }

    private void Clear() {
        
    }
}
