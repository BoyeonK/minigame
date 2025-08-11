using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class InputManager {
    //Ű�� ���� ������ 1��, Ű�� �� ������ 1��, �ڴ����� ���� ���θ� �����ϱ� ���� enum
    // 1 = ��, 2 = ��, 3 = ��
    public enum KeyState {
        Down = 0,
        Up = 1,
        Press = 2,
    }

    private class ActionState {
        public KeyState State;
        public Action Action;
    }

    public Action<Define.MouseEvent> MouseAction = null;
    //Ư�� KeyŰ(Key�� Ű�� ��������������������)�� �Ҵ�� ���� ���������͵��� ���� ��ųʸ�
    private Dictionary<KeyCode, List<ActionState>> _keyActions = new Dictionary<KeyCode, List<ActionState>>();
    bool _mousePressed = false;

    //����
    public void AddKeyListener(KeyCode key, Action action, KeyState state = KeyState.Down) {
        if (!_keyActions.ContainsKey(key))
            _keyActions.Add(key, new List<ActionState>());

        _keyActions[key].Add(new ActionState { Action = action, State = state });
    }

    //���� ���, ���ϸ� �޸� ���� ����. object�� �ı��Ǿ��� ��, �ش� object�� �Ͽ��� �ݵ�� ������ ��.
    public void RemoveKeyListener(KeyCode key, Action action, KeyState state) {
        if (_keyActions.ContainsKey(key)) {
            var list = _keyActions[key];
            // ���޹��� action�� state�� ��Ȯ�� ��ġ�ϴ� �׸��� ã�Ƽ� ����
            var itemToRemove = list.Find(x => x.Action == action && x.State == state);
            if (itemToRemove != null) {
                list.Remove(itemToRemove);
            }
        }
    }

    public void OnUpdate() {
        // �ش� Ű�� ��ϵ� ��� �׼��� ��ȸ
        foreach (var pair in _keyActions) {
            KeyCode key = pair.Key;
            var actionList = pair.Value;

            //�ش� KeyCode�� ��ϵ� ��� List�� ��ȸ, ���ʷ� �����
            if (Input.GetKeyDown(key)) {
                foreach (var actionState in actionList) {
                    if (actionState.State == KeyState.Down)
                        actionState.Action.Invoke();
                }
            }
            else if (Input.GetKeyUp(key)) {
                foreach (var actionState in actionList) {
                    if (actionState.State == KeyState.Up)
                        actionState.Action.Invoke();
                }
            }
            else if (Input.GetKey(key)) {
                foreach (var actionState in actionList) {
                    if (actionState.State == KeyState.Press)
                        actionState.Action.Invoke();
                }
            }
        }

        //UI���� ��ü������ ���콺 �׼��� �ٷ� �����̱� ������, ���콺�� UI�� �������� ��� ���콺 �׼� ��Ȱ��ȭ.
        if (EventSystem.current.IsPointerOverGameObject())
            return;

        if (MouseAction != null) {
            //��Ŭ��
            if (Input.GetMouseButton(0)) {
                MouseAction.Invoke(Define.MouseEvent.Press);
                _mousePressed = true;
            }
            //���콺�� Ŭ�� ���°� �ƴҶ�, ���� ���°� press���
            //�̶� Ŭ������ ���� (�������� �ƴ϶� ����)
            else {
                if (_mousePressed)
                    MouseAction.Invoke(Define.MouseEvent.Click);
                _mousePressed = false;
            }
        }
    }

    public void Clear() {
        _keyActions.Clear();
        MouseAction = null;
    }
}
