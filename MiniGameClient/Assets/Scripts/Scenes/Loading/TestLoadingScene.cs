using UnityEngine;

public class TestLoadingScene : BaseScene {
    private float _progressRate = 0f;
    private bool _isReady = false;
    private bool _hadProgress = false;

    protected override void Init() {
        base.Init();
        _progressRate = 0f;
        _isReady = false;
        SceneType = Define.Scene.TestLoadingScene;
        Managers.Network.OnResponseGameStartedAct += SceneChange;
    }

    private void Start() {
        Debug.Log("�ε��� ��ŸƮ");
        Managers.Scene.LoadSceneAsync();
    }

    private void UpdateProgressRate() {
        float progress = Managers.Scene.GetLoadingProgressRate();
        while (progress >= _progressRate + 0.099f) {
            _hadProgress = true;
            _progressRate += 0.1f;
            Debug.Log("+0.1");
        }
    }

    private void Update() {
        if (!_isReady) {
            UpdateProgressRate();
            if (_hadProgress) {
                Debug.Log($"������ �־���. ����� : {_progressRate}");
                Managers.Network.TrySendLoadingProgressRate(_progressRate);
                _hadProgress = false;
            }
        }
    }

    private void SceneChange() {
        Managers.Scene.CompleteLoadSceneAsync();
    }

    public override void Clear() {
        Managers.Network.OnResponseGameStartedAct -= SceneChange;
    }
}
