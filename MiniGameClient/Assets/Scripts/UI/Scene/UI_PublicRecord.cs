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

        List<int> scores = Managers.Network.Lobby.GetPublicRecord();
        List<string> ids = Managers.Network.Lobby.GetPublicIds();
        for (int i = 0; i < scores.Count; i++) {
            if (i >= _scores.Count)
                break;
            _scores[i].text = scores[i].ToString();
        }
        for (int i = 0; i < ids.Count; i++) {
            if (i >= _ids.Count)
                break;
            _ids[i].text = ids[i];
        }
    }

    private void Clear() {
        
    }
}
