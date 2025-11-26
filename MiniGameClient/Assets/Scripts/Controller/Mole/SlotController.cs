using UnityEngine;

public class SlotController : MonoBehaviour {
    private GameObject _yellow, _crops;
    private GameObject _apple, _banana, _watermelon, _pumpkin;
    private Vector3 _localPos = new Vector3(0.0f, 0.3333333f, 0.0f);
    private Vector3 _finalPos = new Vector3(0.0f, 0.6666666f, 0.0f);
    private float _moveStartTime;
    private const float MOVE_DURATION = 0.5f;
    private GameObject _movingObject = null;
    private KeyCode _key;
    private int _index = 0;

    public void Init(KeyCode key, int index) {
        Transform yellow = transform.Find("Yellow");
        if (yellow != null)
            _yellow = yellow.gameObject;

        Transform crops = transform.Find("Crops");
        if (crops != null) {
            _crops = crops.gameObject;
            Transform apple = crops.Find("Apple");
            if (apple != null) 
                _apple = apple.gameObject;
            Transform banana = crops.Find("Banana");
            if (banana != null)
                _banana = banana.gameObject;
            Transform watermelon = crops.Find("Watermelon");
            if (watermelon != null)
                _watermelon = watermelon.gameObject;
            Transform pumpkin = crops.Find("Pumpkin");
            if (pumpkin != null)
                _pumpkin = pumpkin.gameObject;
        }
            
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
        float tick = Time.time;
        Managers.Network.Mole.TryHitSlot(_index, tick);
    }

    public void SetState(int state) {
        switch (state) {
            case 0:
                SetRed();
                break;
            case 1:
                SetYellow();
                break;
            case 2:
                SetGreen();
                break;
            default:
                return;
        }
    }

    public void SetRed() {
        CropsRed();
        _yellow.SetActive(false);
        _pumpkin.SetActive(false);
        StartMovement(_crops);
    }

    public void CropsRed() {
        int mod = Random.Range(0, 3);
        switch (mod) {
            case 0:
                _apple.SetActive(true);
                break;
            case 1:
                _banana.SetActive(true);
                break;
            case 2:
                _watermelon.SetActive(true);
                break;
        }
    }

    public void CropsNotRed() {
        _apple.SetActive(false);
        _banana.SetActive(false);
        _watermelon.SetActive(false);
    }

    public void SetYellow() {
        CropsNotRed();
        _yellow.SetActive(true);
        _pumpkin.SetActive(false);
    }

    public void SetGreen() {
        CropsNotRed();
        _yellow.SetActive(false);
        _pumpkin.SetActive(true);
        StartMovement(_crops);
    }

    private void StartMovement(GameObject targetObject) {
        _movingObject = targetObject;
        _moveStartTime = Time.time;
    }

    private void UpdatePositionMovement() {
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

    private void UpdateRotationMovement() {
        if (_crops != null) {
            float angle = 90 * Time.deltaTime;
            _crops.transform.Rotate(Vector3.up, angle, Space.Self);
        }
    }

    void Update() {
        UpdatePositionMovement();
        UpdateRotationMovement();
    }
}
