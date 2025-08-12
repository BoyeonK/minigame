using System.Collections.Generic;
using System.Threading;
using UnityEngine;

public class UIManager {
    //������ : ���� �̸��� ������ UI�� ����� �� �ȴ�.
           //: SceneUI�� ������ ���ص� 10�� ���� ������, PopupUI�� SceneUI���� �տ� ��µȴ�.
    private Dictionary<string, GameObject> _uiCache = new Dictionary<string, GameObject>();
    private int _sceneOrder = 0;
    private int _popupOrder = 10;
    private Transform _root = null;

    public void Init() {
        if (_root == null) {
            GameObject go = GameObject.Find("@UI_Root");
            if (go == null)
                go = new GameObject { name = "@UI_Root" };
            _root = go.transform;
            Object.DontDestroyOnLoad(_root.gameObject);
        }
    }

    //name -> prefab�� �̸�
    //name�̶�� �̸��� ���� UI_Popup�� �ش��ϴ� GameObject�� �����ϰ� <T> ������Ʈ�� ����.
    public T ShowPopupUI<T>(string name = null) where T : UI_Popup {
        if (string.IsNullOrEmpty(name))
            name = typeof(T).Name;

        if (_uiCache.TryGetValue(name, out GameObject ext)) {
            Debug.Log($"�̹� �����ϴ� UI�� Ȱ��ȭ�մϴ�: {name}");
            ext.SetActive(true);
            return ext.GetComponent<T>();
        }

        GameObject go = Managers.Resource.Instantiate($"UI/Popup/{name}");
        T popup = Util.GetOrAddComponent<T>(go);
        //go�� ��������(�����Ͷ�� �����ϸ� ����) ��� �ֱ� ������, 
        //���� ��ųʸ� �����̳ʿ��� �ش� object�� ���� ��Ʈ�� �� �� �ִ�.
        _uiCache.Add(name, go);

        Init();
        go.transform.SetParent(_root.transform);

        Canvas canvas = Util.GetOrAddComponent<Canvas>(go);
        canvas.renderMode = RenderMode.ScreenSpaceOverlay;
        canvas.overrideSorting = true;

        int newOrder = Interlocked.Increment(ref _popupOrder);
        canvas.sortingOrder = newOrder;

        return popup;
    }

    //name�̶�� �̸��� ���� UI_Scene�� �ش��ϴ� GameObject�� �����ϰ� <T> ������Ʈ�� ����.
    public T ShowSceneUI<T>(string name = null) where T : UI_Scene {
        if (string.IsNullOrEmpty(name))
            name = typeof(T).Name;

        if (_uiCache.TryGetValue(name, out GameObject ext)) {
            Debug.Log($"�̹� �����ϴ� UI�� Ȱ��ȭ�մϴ�: {name}");
            ext.SetActive(true);
            return ext.GetComponent<T>();
        }

        GameObject go = Managers.Resource.Instantiate($"UI/Scene/{name}");
        T sceneUI = Util.GetOrAddComponent<T>(go);
        _uiCache.Add(name, go);

        Init();
        go.transform.SetParent(_root.transform);

        Canvas canvas = Util.GetOrAddComponent<Canvas>(go);
        canvas.renderMode = RenderMode.ScreenSpaceOverlay;
        canvas.overrideSorting = true;

        int newOrder = Interlocked.Increment(ref _sceneOrder);
        canvas.sortingOrder = newOrder;

        return sceneUI;
    }

    //�̹� �����ϴ� UI�� ��� SetActive(false)
    public void DisableUI(string uiName) {
        //��ųʸ� �����̳ʿ���, uiName�� �ش��ϴ� object�� �����͸� ������.
        //�׸��� Active���� ����
        if (_uiCache.TryGetValue(uiName, out GameObject uiObj)) {
            uiObj.SetActive(false);
        }
        else {
            Debug.LogWarning($"{uiName}�� �ش��ϴ� UI�� ĳ�ÿ� ����");
        }
    }

    //�̹� �����ϸ鼭, SetActive(false)�� UI�� enable
    public void EnableUI(string uiName) {
        if (_uiCache.TryGetValue(uiName, out GameObject uiObj)) {
            uiObj.SetActive(true);
        }
        else {
            Debug.LogWarning($"{uiName}�� �ش��ϴ� UI�� ĳ�ÿ� ����");
        }
    }

    public void Clear() {
        //ĳ�ø� ����, sortOrder�� ����ȭ.
        foreach (var pair in _uiCache) {
            Managers.Resource.Destroy(pair.Value);
        }
        _uiCache.Clear();
        _sceneOrder = 0;
        _popupOrder = 10;
    }
}
