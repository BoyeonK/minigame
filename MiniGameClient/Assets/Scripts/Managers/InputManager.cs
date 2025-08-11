using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class InputManager {
    //키를 누른 시점에 1번, 키를 뗀 시점에 1번, 꾹누르기 가능 여부를 구별하기 위한 enum
    // 1 = 딸, 2 = 깍, 3 = 꾹
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
    //특정 Key키(Key가 키임 ㅋㅋㅋㅋㅋ엌ㅋㅋㅋㅋ)에 할당된 여러 델리게이터들을 담을 딕셔너리
    private Dictionary<KeyCode, List<ActionState>> _keyActions = new Dictionary<KeyCode, List<ActionState>>();
    bool _mousePressed = false;

    //구독
    public void AddKeyListener(KeyCode key, Action action, KeyState state = KeyState.Down) {
        if (!_keyActions.ContainsKey(key))
            _keyActions.Add(key, new List<ActionState>());

        _keyActions[key].Add(new ActionState { Action = action, State = state });
    }

    //구독 취소, 안하면 메모리 누수 생김. object가 파괴되었을 때, 해당 object로 하여금 반드시 실행할 것.
    public void RemoveKeyListener(KeyCode key, Action action, KeyState state) {
        if (_keyActions.ContainsKey(key)) {
            var list = _keyActions[key];
            // 전달받은 action과 state가 정확히 일치하는 항목을 찾아서 제거
            var itemToRemove = list.Find(x => x.Action == action && x.State == state);
            if (itemToRemove != null) {
                list.Remove(itemToRemove);
            }
        }
    }

    public void OnUpdate() {
        // 해당 키에 등록된 모든 액션을 순회
        foreach (var pair in _keyActions) {
            KeyCode key = pair.Key;
            var actionList = pair.Value;

            //해당 KeyCode에 등록된 모든 List를 순회, 차례로 딸깍꾹
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

        //UI에서 자체적으로 마우스 액션을 다룰 예정이기 때문에, 마우스로 UI를 조작중인 경우 마우스 액션 비활성화.
        if (EventSystem.current.IsPointerOverGameObject())
            return;

        if (MouseAction != null) {
            //왼클릭
            if (Input.GetMouseButton(0)) {
                MouseAction.Invoke(Define.MouseEvent.Press);
                _mousePressed = true;
            }
            //마우스가 클릭 상태가 아닐때, 이전 상태가 press라면
            //이때 클릭으로 인정 (누를때가 아니라 뗄때)
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
