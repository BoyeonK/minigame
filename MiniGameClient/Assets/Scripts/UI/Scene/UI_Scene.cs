using UnityEngine;

public class UI_Scene : UI_Base {
    //�̰� �� �ȵǴ��� �˾� �� ��!
    /*
    void Start() {
        Managers.UI.SetCanvas(gameObject, false);
    }
    */
    public virtual void Init() {
        Managers.UI.SetCanvas(gameObject, false);
    }
}
