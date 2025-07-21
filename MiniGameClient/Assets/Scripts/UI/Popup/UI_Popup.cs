using UnityEngine;

public class UI_Popup : UI_Base
{
    //이거 왜 안되는지 알아 둘 것
    /*
    void Start()
    {
        Managers.UI.SetCanvas(gameObject, true);
    }
    */
    public virtual void Init()
    {
        Managers.UI.SetCanvas(gameObject, true);
    }

    public virtual void ClosePopupUI()
    {
        Managers.UI.ClosePopupUI(this);
    }
}