using UnityEngine;

public class UI_Error : UI_Popup {
    enum Texts {
        ErrorText
    }

    enum Buttons {
        ErrorButton
    }

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
