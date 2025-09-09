using UnityEngine;

public class TestLoadingScene : BaseScene {
    private float _progressRate = 0f;
    private float _mEpsilon = 0.001f;
    private bool _isReady = false;
    private bool _hadProgress = false;

    protected override void Init() {
        base.Init();
        _progressRate = 0f;
        _isReady = false;
        SceneType = Define.Scene.TestLoadingScene;
    }

    private void Start() {
        Managers.Scene.LoadSceneAsync();
    }

    private void UpdateProgressRate() {
        float progress = Managers.Scene.GetLoadingProgressRate();
        while (progress >= _progressRate + 0.1) {
            _hadProgress = true;
            _progressRate += 0.1f;
        }
        if (Mathf.Abs(0.9f - _progressRate) < _mEpsilon) {
            _isReady = true;
            Debug.Log("로딩 완료됨");
            //TODO : 서버에 완료됨을 전송.
        }
    }

    private void Update() {
        if (!_isReady) {
            UpdateProgressRate();
            if (_hadProgress) {
                Debug.Log($"진전이 있었다. 진행률 : {_progressRate}");
                //TODO : 서버에 진행상황 보내기.
                _hadProgress = false;
            }
        }
    }
}
