using System.Collections;
using System.Drawing;
using TMPro;
using UnityEngine;

public class SlotController : MonoBehaviour {
    GameObject _yellow, _crops;
    GameObject _apple, _banana, _watermelon, _pumpkin;
    GameObject _pTextObj;
    TextMeshPro _pText;
    Coroutine _textAnimRoutine;
    Vector3 _localPos = new Vector3(0.0f, 0.3333333f, 0.0f);
    Vector3 _finalPos = new Vector3(0.0f, 0.6666666f, 0.0f);
    float _moveStartTime;
    const float MOVE_DURATION = 0.5f;
    GameObject _movingObject = null;
    KeyCode _key;
    int _state = 1;
    int _index = 0;

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
        Transform pText = transform.Find("PText");
        if (pText != null) {
            _pTextObj = pText.gameObject;
            _pTextObj.SetActive(false);
            _pText = pText.GetComponent<TextMeshPro>();
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

    void PlaySFX() {
        int randomIdx = UnityEngine.Random.Range(0, 4);
        Managers.Sound.Play($"MoleGetPoint{randomIdx}");
    }

    public void SetRed() {
        _state = 0;
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
        _state = 1;
        if (_state != 1) PlaySFX();
        CropsNotRed();
        _yellow.SetActive(true);
        _pumpkin.SetActive(false);
    }

    public void SetGreen() {
        _state = 2;
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

    public void ShowPText(int point, bool isMine) {
        if (_textAnimRoutine != null)
            StopCoroutine(_textAnimRoutine);
        _textAnimRoutine = StartCoroutine(ShowPTextRoutine(point, isMine));
    }

    IEnumerator ShowPTextRoutine(int point, bool isMine) {
        _pTextObj.SetActive(true);

        Color32 col;
        if (isMine)
            col = new Color32(255, 0, 0, 255);
        else
            col = new Color32(0, 0, 255, 255);

        if (point >= 0)
            _pText.text = "+" + point.ToString();
        else
            _pText.text = point.ToString();

        Vector3 startPos = new Vector3(0.0f, 1.4f, 0.0f);
        Vector3 endPos = new Vector3(0.0f, 1.7f, 0.0f);

        float duration = 0.5f;
        float startTime = Time.time;
        while (true) {
            float elapsed = Time.time - startTime;
            float t = elapsed / duration;
            if (t < 1.0f) {
                _pTextObj.transform.localPosition = Vector3.Lerp(startPos, endPos, t);
                _pText.color = Color32.Lerp(col, new Color32(col.r, col.g, col.b, 120), t);
                yield return null;
            }
            else
                break;
        }
        _pTextObj.SetActive(false);
        _textAnimRoutine = null;
    }

    void Update() {
        UpdatePositionMovement();
        UpdateRotationMovement();
    }
}
