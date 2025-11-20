using UnityEngine;
using UnityEngine.InputSystem;

public class SlotController : MonoBehaviour {
    private GameObject _red, _yellow, _green;
    private Vector3 _localPos = Vector3.zero;
    private Vector3 _finalPos = new Vector3(0.0f, 0.05f, 0.0f);
    private float _moveStartTime;
    private const float MOVE_DURATION = 0.5f;
    private GameObject _movingObject = null;
    private KeyCode _key;
    private int _index = 0;

    public void Init(KeyCode key, int index) {
        Transform red = transform.Find("Red");
        if (red != null)
            _red = red.gameObject;

        Transform yellow = transform.Find("Yellow");
        if (yellow != null)
            _yellow = yellow.gameObject;

        Transform green = transform.Find("Green");
        if (green != null) 
            _green = green.gameObject;

        _key = key;
        _index = index;
        Managers.Input.AddKeyListener(_key, OnKeyAction, InputManager.KeyState.Down);
        SetYellow();
    }

    public void Clear() {
        Managers.Input.RemoveKeyListener(_key, OnKeyAction, InputManager.KeyState.Down);
    }

    private void OnKeyAction() {
        Debug.Log(_index);
        Managers.Network.Mole.TryHitSlot(_index);
    }

    public void SetRed() {
        _red.SetActive(true);
        _yellow.SetActive(false);
        _green.SetActive(false);
        ResetPos();
        StartMovement(_red);
    }

    public void SetYellow() {
        _red.SetActive(false);
        _yellow.SetActive(true);
        _green.SetActive(false);
        ResetPos();
    }

    public void SetGreen() {
        _red.SetActive(false);
        _yellow.SetActive(false);
        _green.SetActive(true);
        ResetPos();
        StartMovement(_green);
    }

    private void ResetPos() {
        _movingObject = null;
        if (_red != null) _red.transform.localPosition = Vector3.zero;
        if (_yellow != null) _yellow.transform.localPosition = Vector3.zero;
        if (_green != null) _green.transform.localPosition = Vector3.zero;
    }

    private void StartMovement(GameObject targetObject) {
        _movingObject = targetObject;
        _moveStartTime = Time.time;
    }

    private void UpdateMovement() {
        if (_movingObject == null)
            return;

        float elapsedTime = Time.time - _moveStartTime;
        float t = elapsedTime / MOVE_DURATION;

        if (t < 1.0f)
            _movingObject.transform.localPosition = Vector3.Lerp(_localPos, _finalPos, t);
        else {
            _movingObject.transform.localPosition = _finalPos;
            _movingObject = null;
        }
    }

    void Update() {
        UpdateMovement();
    }
}
