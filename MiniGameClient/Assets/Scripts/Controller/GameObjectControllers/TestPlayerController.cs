using UnityEngine;

public class TestPlayerController : GameObjectController {
    GameObject _body;
    GameObject _camWrapper;
    Camera _camera;
    private float sensitivity = 1000f; 
    private float xRotation = 0f;
    private float yRotation = 0f;
    private Vector3 _viewFront = new();
    private Vector3 _viewRight = new();
    private bool _w = false;
    private bool _a = false;
    private bool _s = false;
    private bool _d = false;
    private const float _accelerationRate = 0.03f;
    private const float _frictionRate = 0.02f;
    private const float _maxVelocity = 0.015f;
    private const float _rotationSpeed = 4f;
    private Vector3 _characterFront = new();
    private Vector3 _accelerationDir = new();
    private Vector3 _velocity = new();

    public override void Init() {
        Cursor.lockState = CursorLockMode.Locked;
        SetObjectId((int)Define.ObjectType.TestPlayer);

        Transform bodyTransform = transform.Find("TestPlayerBody");
        if (bodyTransform != null)
            _body = bodyTransform.gameObject;
        Transform camTransform = transform.Find("PlayerCamera");
        if (camTransform != null) {
            _camWrapper = camTransform.gameObject;
            Transform camera = camTransform.Find("Cam");
            if (camera != null)
                _camera = camera.gameObject.GetComponent<Camera>();

            Vector3 currentRotation = _camWrapper.transform.localRotation.eulerAngles;
            xRotation = currentRotation.x;
            yRotation = currentRotation.y;
        }
        Managers.Input.AddKeyListener(KeyCode.W, WDown, InputManager.KeyState.Down);
        Managers.Input.AddKeyListener(KeyCode.A, ADown, InputManager.KeyState.Down);
        Managers.Input.AddKeyListener(KeyCode.S, SDown, InputManager.KeyState.Down);
        Managers.Input.AddKeyListener(KeyCode.D, DDown, InputManager.KeyState.Down);
        Managers.Input.AddKeyListener(KeyCode.W, WUp, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.A, AUp, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.S, SUp, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.D, DUp, InputManager.KeyState.Up);
    } 

    void Update() {
        MoveCamOnUpdate();
        GetViewFrontOnUpdate();
        GetAccelerationDirOnUpdate();
        LerpRotationOnUpdate();
        CalculateVelocityOnUpdate();
        MovePlayerOnUpdate();
    }

    private void MoveCamOnUpdate() {
        if (_camWrapper == null) return;
        float mouseX = Input.GetAxis("Mouse X") * sensitivity * Time.deltaTime;
        float mouseY = Input.GetAxis("Mouse Y") * sensitivity * Time.deltaTime;

        xRotation -= mouseY;
        xRotation = Mathf.Clamp(xRotation, -80f, 80f);

        yRotation += mouseX;
        _camWrapper.transform.localRotation = Quaternion.Euler(xRotation, yRotation, 0f);
    }

    private void GetViewFrontOnUpdate() {
        if (_camera != null) {
            Vector3 forwardXZ = _camera.transform.forward;
            forwardXZ.y = 0f;
            _viewFront = forwardXZ.normalized;
            _viewRight = Vector3.Cross(Vector3.up, _viewFront).normalized;
        }
    }

    private void GetAccelerationDirOnUpdate() {
        _accelerationDir = Vector3.zero;
        if (_w)
            _accelerationDir += _viewFront;
        if (_a)
            _accelerationDir -= _viewRight;
        if (_s)
            _accelerationDir -= _viewFront;
        if (_d)
            _accelerationDir += _viewRight;

        if(_accelerationDir.magnitude > 0.01f)
            _accelerationDir = _accelerationDir.normalized;
        else
            _accelerationDir = Vector3.zero;
    }

    private void LerpRotationOnUpdate() {
        //키보드로 인한 가속 방향으로 캐릭터의 정면이 향하도록 부드러운 러프로테이션
        if (_accelerationDir.sqrMagnitude > 0.001f) {
            Quaternion targetRotation = Quaternion.LookRotation(_accelerationDir);
            _body.transform.rotation = Quaternion.Lerp(_body.transform.rotation, targetRotation, Time.deltaTime * _rotationSpeed);
        }
    }

    private void CalculateVelocityOnUpdate() {
        _velocity += _accelerationDir * _accelerationRate * Time.deltaTime;
        if (_velocity.magnitude > _frictionRate * Time.deltaTime)
            _velocity -= _velocity.normalized * _frictionRate * Time.deltaTime;
        else
            _velocity = Vector3.zero;

        if (_velocity.magnitude > _maxVelocity)
            _velocity = _velocity.normalized * _maxVelocity;
    }

    private void MovePlayerOnUpdate() {
        transform.Translate(_velocity, Space.World);
    }

    private void WDown() {
        _w = true;
    }

    private void ADown() {
        _a = true;
    }

    private void SDown() {
        _s = true;
    }

    private void DDown() {
        _d = true;
    }

    private void WUp() {
        _w = false;
    }

    private void AUp() {
        _a = false;
    }

    private void SUp() {
        _s = false;
    }

    private void DUp() {
        _d = false;
    }

    private void OnDestroy() {
        Managers.Input.RemoveKeyListener(KeyCode.W, WDown, InputManager.KeyState.Down);
        Managers.Input.RemoveKeyListener(KeyCode.A, ADown, InputManager.KeyState.Down);
        Managers.Input.RemoveKeyListener(KeyCode.S, SDown, InputManager.KeyState.Down);
        Managers.Input.RemoveKeyListener(KeyCode.D, DDown, InputManager.KeyState.Down);
        Managers.Input.RemoveKeyListener(KeyCode.W, WUp, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.A, AUp, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.S, SUp, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.D, DUp, InputManager.KeyState.Up);
    }
}
