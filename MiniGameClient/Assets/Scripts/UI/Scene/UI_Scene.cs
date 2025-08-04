using UnityEngine;

public class UI_Scene : UI_Base {
    //이거 왜 안되는지 알아 둘 것!
    /*
    void Start() {
        Managers.UI.SetCanvas(gameObject, false);
    }
    */
    public virtual void Init() {
        Managers.UI.SetCanvas(gameObject, false);
    }
}
