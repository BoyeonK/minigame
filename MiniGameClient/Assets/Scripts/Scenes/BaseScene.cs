using UnityEngine;
using UnityEngine.EventSystems;

public class BaseScene : MonoBehaviour {
    public Define.Scene SceneType { get; protected set; } = Define.Scene.Unknown;

    private void Awake() {
        Init();
    }

    protected virtual void Init() {
        Object obj = GameObject.FindFirstObjectByType(typeof(EventSystem));
        if (obj == null) {
            Debug.Log("Make EventSys");
            Managers.Resource.Instantiate("UI/EventSystem").name = "@EventSyetem";
        }
        else {
            Debug.Log("asdf");
        }
    }

    public virtual void Clear() { }
}
