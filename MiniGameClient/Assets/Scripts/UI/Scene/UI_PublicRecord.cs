using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;

public class UI_PublicRecord : UI_Scene {


    private void OnEnable() {
        Init();
    }

    private void OnDisable() {
        Clear();
    }

    public override void Init() {
        base.Init();

    }

    private void Clear() {
        
    }
}
