using UnityEngine;

public class UI_Popup : UI_Base
{
    //�̰� �� �ȵǴ��� �˾� �� ��
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