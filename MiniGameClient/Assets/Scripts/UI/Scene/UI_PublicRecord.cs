using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;

public class UI_PublicRecord : UI_Scene {
    List<TextMeshProUGUI> _scores = new List<TextMeshProUGUI>();
    List<TextMeshProUGUI> _ids = new List<TextMeshProUGUI>();

    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    public override void Init() {
        base.Init();
        _scores.Clear();
        _ids.Clear();

        List<TextMeshProUGUI> allTexts = new List<TextMeshProUGUI>();
        GetComponentsInChildren<TextMeshProUGUI>(true, allTexts);
        foreach (TextMeshProUGUI text in allTexts) {
            if (text.name.StartsWith("Score"))
                _scores.Add(text);
            else if (text.name.StartsWith("PlayerId"))
                _ids.Add(text);
        }
        _scores.Sort((a, b) => string.Compare(a.name, b.name));
        _ids.Sort((a, b) => string.Compare(a.name, b.name));

        Managers.Network.Lobby.TryGetPublicRecords();
    }

    public void BindRecord() {
        List<string> ids = Managers.Network.Lobby.GetPublicIds();
        List<int> records = Managers.Network.Lobby.GetPublicRecord();

        for (int i = 0; i < records.Count; i++) {
            if (i > 2) break;
            _scores[i].text = records[i].ToString();
        }

        for (int i = 0; i < _ids.Count; i++) {
            if (i > 2) break;
            _ids[i].text = ids[i];
        }
        Debug.Log("123f");
    }

    private void Clear() {
        
    }
}
