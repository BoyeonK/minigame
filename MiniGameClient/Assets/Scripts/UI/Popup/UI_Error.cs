using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class UI_Error : UI_Popup {
    enum Texts {
        ErrorText
    }

    enum Buttons {
        ErrorButton
    }

    private TextMeshProUGUI _errorText;
    private Button _errorButton;
    private bool _isQuit = false;

    private void OnDestroy() {
        Clear();
        if (_isQuit) {
            Managers.ExecuteAtMainThread(() => {
#if UNITY_EDITOR
                UnityEditor.EditorApplication.isPlaying = false;
#endif
                Application.Quit(); 
            });
        }
    }

    public override void Init() {
        base.Init();
    }

    public void Init(string errorDetail, bool isQuit) {
        Init();
        Bind<TextMeshProUGUI>(typeof(Texts));
        Bind<Button>(typeof(Buttons));
        _errorText = Get<TextMeshProUGUI>((int)Texts.ErrorText);
        _errorButton = Get<Button>((int)Buttons.ErrorButton);
        _isQuit = isQuit;

        _errorText.text = errorDetail;
        _errorButton.onClick.AddListener(DestroyThis);
    }

    private void Clear() {
        _errorButton.onClick.RemoveAllListeners();
    }

    private void DestroyThis() {
        GameObject go = this.gameObject;
        Managers.Resource.Destroy(go);
    }
}
