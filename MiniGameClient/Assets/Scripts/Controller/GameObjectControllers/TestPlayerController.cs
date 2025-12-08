using UnityEngine;

public class TestPlayerController : GameObjectController {
    enum State {
        Standing,
        Moving,
        Jumping,
    }

    State _prestate = State.Standing;
    State _state = State.Standing;

    TestPlayerCamera _camera;
    private float _cameraDistance = 5f;
    private float sensitivity = 1000f; 
    private float xRotation = 0f;
    private float yRotation = 0f;

    private Vector3 _front = new(0f, 0f, 1f);
    private Vector3 _right = new();
    private Vector3 _viewFront = new(0f, 0f, 1f);
    private bool _w = false;
    private bool _a = false;
    private bool _s = false;
    private bool _d = false;
    private bool _jump = false;

    private Rigidbody _rigidBody;
    private const float _accelerationRate = 6f;
    private const float _horizonFrictionRatePerVelocity = 2f;
    private const float _gravityAccel = 12f;
    private const float _jumpSpeed = 5.2f;
    private const float MAX_VERTICAL_SPEED = 12f;

    private Vector3 _accelerationDir = new();

    public override void Init() {
        Cursor.lockState = CursorLockMode.Locked;
        SetObjectId((int)Define.ObjectType.TestPlayer);

        Transform camTransform = transform.Find("PlayerCamera");
        if (camTransform != null)
            _camera = camTransform.GetComponent<TestPlayerCamera>();
        if (_camera != null)
            _camera.Init(transform.gameObject);

        _right = Vector3.Cross(Vector3.up, _front).normalized;

        _rigidBody = GetComponent<Rigidbody>();

        Managers.Input.AddKeyListener(KeyCode.W, WDown, InputManager.KeyState.Down);
        Managers.Input.AddKeyListener(KeyCode.A, ADown, InputManager.KeyState.Down);
        Managers.Input.AddKeyListener(KeyCode.S, SDown, InputManager.KeyState.Down);
        Managers.Input.AddKeyListener(KeyCode.D, DDown, InputManager.KeyState.Down);
        Managers.Input.AddKeyListener(KeyCode.Space, TryJump, InputManager.KeyState.Down);
        Managers.Input.AddKeyListener(KeyCode.W, WUp, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.A, AUp, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.S, SUp, InputManager.KeyState.Up);
        Managers.Input.AddKeyListener(KeyCode.D, DUp, InputManager.KeyState.Up);
    } 

    void Update() {
        ProcessMouseMoveOnUpdate();
        
    }

    void FixedUpdate() {
        GetAccelerationDirOnFixedUpdate();
        CalculateVelocityOnFixedUpdate();
        LerpRotationOnUpdate();
    }

    private void ProcessMouseMoveOnUpdate() {
        float mouseX = Input.GetAxis("Mouse X") * sensitivity * Time.deltaTime;
        float mouseY = Input.GetAxis("Mouse Y") * sensitivity * Time.deltaTime;

        xRotation -= mouseY;
        xRotation = Mathf.Clamp(xRotation, -80f, 80f);

        yRotation += mouseX;

        Quaternion rotation = Quaternion.Euler(0f, yRotation, 0f);
        _front = rotation * Vector3.forward;
        _right = Vector3.Cross(Vector3.up, _front).normalized;

        Quaternion fullViewRotation = Quaternion.Euler(xRotation, yRotation, 0f);
        _viewFront = fullViewRotation * Vector3.forward;

        RaycastHit hitInfo;
        Vector3 cameraPos;

        if (Physics.Raycast(transform.position, -_viewFront, out hitInfo, _cameraDistance))
            cameraPos = hitInfo.point;
        else
            cameraPos = transform.position + (-_viewFront * _cameraDistance);

        if (_camera != null)
            _camera.ChangePositionOnUpdate(cameraPos);
    }
    private void GetAccelerationDirOnFixedUpdate() {
        _accelerationDir = Vector3.zero;
        if (_w)
            _accelerationDir += _front;
        if (_s)
            _accelerationDir -= _front;
        if (_d)
            _accelerationDir += _right;
        if (_a)
            _accelerationDir -= _right;

        if (_accelerationDir.sqrMagnitude > Vector3.kEpsilon)
            _accelerationDir = _accelerationDir.normalized;
        else
            _accelerationDir = Vector3.zero;
    }

    private void LerpRotationOnUpdate() {
        if (_accelerationDir.sqrMagnitude > 0.001f) {
            Quaternion targetRotation = Quaternion.LookRotation(_accelerationDir);

            transform.rotation = Quaternion.Slerp(
                transform.rotation,
                targetRotation,              
                4 * Time.deltaTime
            );
        }
    }

    private void SetStateOnUpdate() {

    }

    private void PlayAnimationOnUpdate() {
        _prestate = _state;
    }

    private void CalculateVelocityOnFixedUpdate() {
        if (_rigidBody == null) return;

        if (_accelerationDir.sqrMagnitude > 0.001f) {
            _rigidBody.AddForce(_accelerationDir * _accelerationRate, ForceMode.Acceleration);
        }
        _rigidBody.AddForce(Vector3.down * _gravityAccel, ForceMode.Acceleration);

        Vector3 horizontalVelocity = new(_rigidBody.linearVelocity.x, 0f, _rigidBody.linearVelocity.z);

        if (horizontalVelocity.sqrMagnitude > 0.001f) {
            _rigidBody.AddForce(-horizontalVelocity * _horizonFrictionRatePerVelocity, ForceMode.Acceleration);
        }

        Vector3 currentVelocity = _rigidBody.linearVelocity;
        if (_jump) {
            currentVelocity.y = _jumpSpeed;
            _rigidBody.linearVelocity = currentVelocity;
            _jump = false;
        }
        else {
            currentVelocity.y = Mathf.Clamp(currentVelocity.y, -MAX_VERTICAL_SPEED, MAX_VERTICAL_SPEED);
            _rigidBody.linearVelocity = currentVelocity;
        } 
    }

    private void MovePlayerOnUpdate() {
        
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

    private void TryJump() {
        _jump = true;
    }

    private void OnDestroy() {
        Managers.Input.RemoveKeyListener(KeyCode.W, WDown, InputManager.KeyState.Down);
        Managers.Input.RemoveKeyListener(KeyCode.A, ADown, InputManager.KeyState.Down);
        Managers.Input.RemoveKeyListener(KeyCode.S, SDown, InputManager.KeyState.Down);
        Managers.Input.RemoveKeyListener(KeyCode.D, DDown, InputManager.KeyState.Down);
        Managers.Input.RemoveKeyListener(KeyCode.Space, TryJump, InputManager.KeyState.Down);
        Managers.Input.RemoveKeyListener(KeyCode.W, WUp, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.A, AUp, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.S, SUp, InputManager.KeyState.Up);
        Managers.Input.RemoveKeyListener(KeyCode.D, DUp, InputManager.KeyState.Up);
    }
}
