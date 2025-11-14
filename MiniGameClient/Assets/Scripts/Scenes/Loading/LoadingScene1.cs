using UnityEngine;

public class LoadingScene1 : BaseLoadingScene {
    LS1TextUIController _textUI;
    LS1LoadingBarController _loadingBar;
    int _quota = 2;

    protected override void Init() {
        //base.Init();
        //_quota = Managers.Network.GetIngamePlayerIds().Count;
        GameObject goTextUI = GameObject.Find("TextUI");
        GameObject goLoadingBar = GameObject.Find("LoadingBar");
        if (goTextUI != null)
            _textUI = goTextUI.GetComponent<LS1TextUIController>();
        if (goLoadingBar != null)
            _loadingBar = goLoadingBar.GetComponent<LS1LoadingBarController>();

        if (_loadingBar != null)
            _loadingBar.Init(_quota);
    }

    private void Start() {
        //OnStart();
    }

    private void Update() {
        //OnUpdate();
    }

    public override void Clear() {
        base.Clear();
    }
}
