using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class LoadingScene1 : BaseLoadingScene {
    LS1TextUIController _textUI;
    LS1LoadingBarController _loadingBar;

    List<string> _playerIds = new List<string>();
    int _gameId = 0;

    protected override void Init() {
        //base.Init();
        Managers.Network.TestLoadingSceneInit();

        _playerIds = Managers.Network.GetIngamePlayerIds();
        _gameId = Managers.Network._gameId;
        GameObject goTextUI = GameObject.Find("TextUI");
        GameObject goLoadingBar = GameObject.Find("LoadingBar");
        if (goTextUI != null)
            _textUI = goTextUI.GetComponent<LS1TextUIController>();
        if (goLoadingBar != null)
            _loadingBar = goLoadingBar.GetComponent<LS1LoadingBarController>();

        if (_loadingBar != null)
            _loadingBar.Init(Define.Quota(_gameId));
        if (_textUI != null)
            _textUI.Init(_gameId, _playerIds);
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
