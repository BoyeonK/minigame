using TMPro;
using UnityEngine;

public class UI_Race_EndGame : UI_Scene {
    enum Texts {
        Message,
    }

    private void OnEnable() {
        Init();
    }

    public override void Init() {
        base.Init();
        Bind<TextMeshProUGUI>(typeof(Texts));
    }

    private void OnDisable() {
        Clear();
    }

    private void Clear() {

    }
}
