using System.Collections.Generic;
using UnityEngine;

public class UIManager {
    int _order = 10;

    Stack<UI_Popup> _popupStack = new Stack<UI_Popup>();
    UI_Scene _sceneUI = null;

    public void SetCanvas(GameObject go, bool sort = true) {
        Canvas canvas = Util.GetOrAddComponent<Canvas>(go);
        canvas.renderMode = RenderMode.ScreenSpaceOverlay;
        canvas.overrideSorting = true;

        if (sort) {
            canvas.sortingOrder = _order;
            _order++;
        } else {
            canvas.sortingOrder = 0;
        }
    }

    //name -> prefab의 이름
    public T ShowPopupUI<T>(string name = null) where T : UI_Popup {
        if (string.IsNullOrEmpty(name))
            name = typeof(T).Name;

        GameObject go = Managers.Resource.Instantiate($"UI/Popup/{name}");
        T popup = Util.GetOrAddComponent<T>(go);
        _popupStack.Push(popup);
        //_order++;

        //popup UI를 만들고 "@UI_Root"라는 object를 만들어서 그 아래의 컴포넌트로서 둔다.
        GameObject root = GameObject.Find("@UI_Root");
        if (root == null)
            root = new GameObject { name = "@UI_Root" };
        go.transform.SetParent(root.transform);

        return popup;
    }

    public T ShowSceneUI<T>(string name = null) where T : UI_Scene {
        if (string.IsNullOrEmpty(name))
            name = typeof(T).Name;

        GameObject go = Managers.Resource.Instantiate($"UI/Scene/{name}");
        T sceneUI = Util.GetOrAddComponent<T>(go);
        _sceneUI = sceneUI;

        //_order++;
        GameObject root = GameObject.Find("@UI_Root");
        if (root == null)
            root = new GameObject { name = "@UI_Root" };
        go.transform.SetParent(root.transform);

        return sceneUI;
    }

    public void ClosePopupUI() {
        if (_popupStack.Count == 0)
            return;

        UI_Popup popup = _popupStack.Pop();
        Managers.Resource.Destroy(popup.gameObject);
        popup = null;
        //_order--;
    }

    public void ClosePopupUI(UI_Popup popup) {
        if (_popupStack.Count == 0)
            return;

        if (_popupStack.Peek() != popup) {
            Debug.Log("Close Popup Failed");
            return;
        }

        ClosePopupUI();
    }

    public void CloseAllPopupUI() {
        //이거 Thread 안전함??
        while (_popupStack.Count > 0)
            ClosePopupUI();
    }

    public void Clear() {
        CloseAllPopupUI();
        _sceneUI = null;
    }
}
