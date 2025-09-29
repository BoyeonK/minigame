using System;
using UnityEngine;
using UnityEngine.SceneManagement;

//�� ��ü�� ����ϴ� ��� �Լ���, ����Ƽ�� �޼��带 ����ؾ� �ϹǷ� ���ν����忡�� ���ุ�� ������ ���������.
//����ȭ ����� ������� ���� ����.
public class SceneManagerEx {
    private enum LoadingState {
        None,
        Loading,
        Ready
    }
    
    private LoadingState _loadingState = LoadingState.None;
    private Define.Scene _nextScene = Define.Scene.Undefined;
    //private bool _sceneActiveImmidiately = false;
    private AsyncOperation _asyncLoadSceneOp;
    private float _progress = 0.0f;

    public BaseScene CurrentScene { get { return GameObject.FindFirstObjectByType<BaseScene>(); } }

    public void OnUpdate() {
        if (_loadingState != LoadingState.Loading || _asyncLoadSceneOp == null)
            return;
        _progress = _asyncLoadSceneOp.progress;
    }
    
    public void LoadScene(Define.Scene type) {
        if (_loadingState != LoadingState.None)
            return;

        Managers.Clear();
        SceneManager.LoadScene(GetSceneName(type));
    }

    string GetSceneName(Define.Scene type) {
        string name = System.Enum.GetName(typeof(Define.Scene), type);
        return name;
    }

    public void LoadSceneWithLoadingScene(Define.Scene targetScene, Define.Scene loadingScene) {
        if (_loadingState != LoadingState.None)
            return;

        _loadingState = LoadingState.Loading;
        _nextScene = targetScene;
        Managers.Clear();
        SceneManager.LoadScene(GetSceneName(loadingScene));
    }

    public void LoadSceneAsync() {
        if (_loadingState != LoadingState.Loading || _nextScene == Define.Scene.Undefined)
            return;

        Debug.Log("�Ŵ��� Ŭ�������� �� ������ �ε� ����");
        _asyncLoadSceneOp = SceneManager.LoadSceneAsync(GetSceneName(_nextScene));
        _asyncLoadSceneOp.allowSceneActivation = false;
    }

    public float GetLoadingProgressRate() {
        return _progress;
    }

    public void CompleteLoadSceneAsync() {
        Managers.Clear();
        _asyncLoadSceneOp.allowSceneActivation = true;
    }

    //LoadingScene�� �ƴ� Scene�� �ʱ�ȭ �������� ȣ���ؾ���.
    public void ResetLoadSceneOp() {
        _asyncLoadSceneOp = null;
        _loadingState = LoadingState.None;
        _nextScene = Define.Scene.Undefined;
        _progress = 0;
    }

    public BaseScene GetCurrentSceneComponent() {
        GameObject go = GameObject.Find("GameScene");
        if (go == null) return null;
        return go.GetComponent<BaseScene>();
    }

    public void Clear() {
        CurrentScene.Clear();
    }
}
