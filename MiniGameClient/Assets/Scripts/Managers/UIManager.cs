using System.Collections.Generic;
using System.Threading;
using System.Xml.Linq;
using UnityEngine;

public class UIManager {
    //대전제 : 같은 이름을 가지는 UI는 만들면 안 된다.
           //: SceneUI의 개수는 합해도 10이 넘지 않으며, PopupUI는 SceneUI보다 앞에 출력된다.
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

    //name -> prefab의 이름
    //name이라는 이름을 가진 UI_Popup에 해당하는 GameObject를 생성하고 <T> 컴포넌트를 부착.
    public T ShowPopupUI<T>(string name = null) where T : UI_Popup {
        if (string.IsNullOrEmpty(name))
            name = typeof(T).Name;

        if (_uiCache.TryGetValue(name, out GameObject ext)) {
            Debug.Log($"이미 존재하는 UI를 활성화합니다: {name}");
            ext.SetActive(true);
            return ext.GetComponent<T>();
        }

        GameObject go = Managers.Resource.Instantiate($"UI/Popup/{name}");
        T popup = Util.GetOrAddComponent<T>(go);

        Init();
        go.transform.SetParent(_root.transform);
        Debug.Log($"{name}에 저장");

        Canvas canvas = Util.GetOrAddComponent<Canvas>(go);
        canvas.renderMode = RenderMode.ScreenSpaceOverlay;
        canvas.overrideSorting = true;

        int newOrder = Interlocked.Increment(ref _popupOrder);
        canvas.sortingOrder = newOrder;

        //go의 참조값을(포인터라고 생각하면 편함) 들고 있기 때문에, 
        //위의 딕셔너리 컨테이너에서 해당 object를 꺼내 컨트롤 할 수 있다.
        _uiCache.Add(name, go);

        return popup;
    }

    //name이라는 이름을 가진 UI_Scene에 해당하는 GameObject를 생성하고 <T> 컴포넌트를 부착.
    public T ShowSceneUI<T>(string name = null) where T : UI_Scene {
        if (string.IsNullOrEmpty(name))
            name = typeof(T).Name;

        if (_uiCache.TryGetValue(name, out GameObject ext)) {
            Debug.Log($"이미 존재하는 UI를 활성화합니다: {name}");
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

    //이미 존재하는 UI를 잠시 SetActive(false)
    public void DisableUI(string uiName) {
        //딕셔너리 컨테이너에서, uiName에 해당하는 object의 포인터를 꺼낸다.
        //그리고 Active값을 변경
        Debug.Log($"{uiName}에 해당하는 친구를 삭제하겠다.");
        if (_uiCache.TryGetValue(uiName, out GameObject uiObj)) {
            if (uiObj != null) {
                uiObj.SetActive(false);
                Debug.Log($"{uiName} UI를 비활성화했습니다.");
            } else  {
                _uiCache.Remove(uiName);
                Debug.LogWarning($"{uiName} 참조가 캐시에 남아있었지만, 오브젝트는 이미 파괴되었습니다. 캐시에서 제거합니다.");
            }
        }
        else {
            Debug.LogWarning($"{uiName}에 해당하는 UI가 캐시에 없음");
        }
    }

    //이미 존재하면서, SetActive(false)인 UI를 enable
    public void EnableUI(string uiName) {
        if (_uiCache.TryGetValue(uiName, out GameObject uiObj)) {
            uiObj.SetActive(true);
        }
        else {
            Debug.LogWarning($"{uiName}에 해당하는 UI가 캐시에 없음");
        }
    }

    public void Clear() {
        //캐시를 비우고, sortOrder를 정상화.
        foreach (var pair in _uiCache) {
            Managers.Resource.Destroy(pair.Value);
        }
        _uiCache.Clear();
        _sceneOrder = 0;
        _popupOrder = 10;
    }

    public UI_Error ShowErrorUI(string errorDetail, bool isQuit = true) {
        //에러 발생 UI는 캐싱하지 않음.
        //정상적인 상황은 아니기 때문에 보수적으로 접근한다.
        GameObject go = Managers.Resource.Instantiate("UI/Popup/UI_Error");
        UI_Error uiError = Util.GetOrAddComponent<UI_Error>(go);

        Init();
        go.transform.SetParent(_root.transform);

        Canvas canvas = Util.GetOrAddComponent<Canvas>(go);
        canvas.renderMode = RenderMode.ScreenSpaceOverlay;
        canvas.overrideSorting = true;

        //최상단에 위치하도록
        int order = _popupOrder + 1000;
        canvas.sortingOrder = order;

        //에러 내용 + 종료 여부를 담아 초기화 함수 실행.
        uiError.Init(errorDetail, isQuit);

        return uiError;
    }
}
