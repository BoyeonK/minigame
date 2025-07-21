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

        //UI�� �������� ���, ��Ȱ��ȭ.
        /*
        if (EventSystem.current.IsPointerOverGameObject())
            return;
        */

        if (MouseAction != null)
        {
            //��Ŭ��
            if (Input.GetMouseButton(0))
            {
                MouseAction.Invoke(Define.MouseEvent.Press);
                _mousePressed = true;
            }
            //���콺�� Ŭ�� ���°� �ƴҶ�, ���� ���°� press���
            //�̶� Ŭ������ ���� (�������� �ƴ϶� ����)
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
