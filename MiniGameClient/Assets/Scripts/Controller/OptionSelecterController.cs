using UnityEngine;

public class OptionSelecterController : MonoBehaviour {
    private int _counter = 0;
    private Vector3 _lookDirection;
    private Vector3 _eyePosition;
    private Camera _mainCamera;
    [SerializeField]
    private float _rotationSpeed = 5f;
    [SerializeField]
    private float _positionSpeed = 5f;

    private void NxtOpt() {
        _counter = (_counter + 1) % 4;
        Debug.Log("Next");
        ChangeLookDir();
    }

    private void PrOpt() {
        _counter = (_counter - 1 + 4) % 4;
        Debug.Log("Pre");
        ChangeLookDir();
    }

    void Awake() {
        _mainCamera = Camera.main;
    }

    private void ChangeLookDir() {
        switch (_counter) {
            case 0:
                _lookDirection = new Vector3(1.0f, 0.0f, 0f).normalized;
                _eyePosition = new Vector3(1.0f, 0.0f, 0.0f);
                break;
            case 1:
                _lookDirection = new Vector3(0.0f, 0.0f, 1.0f).normalized;
                _eyePosition = new Vector3(0.0f, 0.0f, 1.0f);
                break;
            case 2:
                _lookDirection = new Vector3(-1.0f, 0.0f, 0f).normalized;
                _eyePosition = new Vector3(-1.0f, 0.0f, 0.0f);
                break;
            case 3:
                _lookDirection = new Vector3(0.0f, 0.0f, -1.0f).normalized;
                _eyePosition = new Vector3(0.0f, 0.0f, -1.0f);
                break;
        }
    }

    //OnEnable과 Start는 비슷하면서도 다르다. 스크립트의 생명주기와 재사용성에 연관이 있다.
    private void OnEnable() {
        _lookDirection = transform.forward;
        _eyePosition = new Vector3(0.0f, 0.0f, 0.0f);
        Managers.Input.AddKeyListener(KeyCode.UpArrow, PrOpt, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.DownArrow, NxtOpt, InputManager.KeyState.Up);
    }

    //동일하게 OnDisable과 OnDestroy도 그렇다.
    private void OnDisable() {
        Managers.Input.RemoveKeyListener(KeyCode.UpArrow, PrOpt, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.DownArrow, NxtOpt, InputManager.KeyState.Up);
    }

    void Update() {
        if (_mainCamera != null && _lookDirection != Vector3.zero) {
            Quaternion targetRotation = Quaternion.LookRotation(_lookDirection);

            // 현재 회전에서 목표 회전으로 부드럽게 이동
            _mainCamera.transform.rotation = Quaternion.Slerp(
                _mainCamera.transform.rotation,
                targetRotation,
                Time.deltaTime * _rotationSpeed
            );

            _mainCamera.transform.position = Vector3.Lerp(
                _mainCamera.transform.position,
                _eyePosition,
                Time.deltaTime * _positionSpeed
            );
        }
    }
}
