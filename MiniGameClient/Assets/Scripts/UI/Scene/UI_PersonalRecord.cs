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
        
    }

    private void Clear() {
        
    }
}
