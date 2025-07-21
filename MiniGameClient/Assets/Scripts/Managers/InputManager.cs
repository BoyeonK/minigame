using System;
using UnityEngine;
using UnityEngine.EventSystems;

public class InputManager
{
    public Action KeyAction = null;
    public Action<Define.MouseEvent> MouseAction = null;

    bool _mousePressed = false;

    public void OnUpdate()
    {
        if (Input.anyKey != false && KeyAction != null)
            KeyAction.Invoke();

        //UI를 조작중인 경우, 비활성화.
        /*
        if (EventSystem.current.IsPointerOverGameObject())
            return;
        */

        if (MouseAction != null)
        {
            //왼클릭
            if (Input.GetMouseButton(0))
            {
                MouseAction.Invoke(Define.MouseEvent.Press);
                _mousePressed = true;
            }
            //마우스가 클릭 상태가 아닐때, 이전 상태가 press라면
            //이때 클릭으로 인정 (누를때가 아니라 뗄때)
            else
            {
                if (_mousePressed)
                    MouseAction.Invoke(Define.MouseEvent.Click);
                _mousePressed = false;
            }
        }
    }

    public void Clear()
    {
        KeyAction = null;
        MouseAction = null;
    }
}
