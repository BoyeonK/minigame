using Google.Protobuf.Protocol;
using UnityEngine;

public class RacePlayerController : GameObjectController {
    enum State {
        Standing,
        Moving,
        Jumping,
    }

    State _prestate = State.Standing;
    State _state = State.Standing;
    bool _isGrounded = true;

    private LayerMask _groundLayer;
    private float _groundCheckRadius = 0.3f;
    private float _groundCheckDistance = 0.5f;

    TestPlayerCamera _camera;
    private float _cameraDistance = 5f;
    private float sensitivity = 1000f; 
    private float xRotation = 0f;
    private float yRotation = 0f;

    private Vector3 _front = new(0f, 0f, 1f);
    private Vector3 _right = new();
    private Vector3 _viewFront = new(0f, 0f, 1f);
    private Vector3 _offset = new(0f, 0.5f, 0f);
    private bool _w = false;
    private bool _a = false;
    private bool _s = false;
    private bool _d = false;
    private bool _jump = false;

    private Rigidbody _rigidBody;
    private const float _accelerationRate = 5f;
    private const float _horizonFrictionRatePerVelocity = 2f;
    private const float _gravityAccel = 15f;
    private const float _jumpSpeed = 4.8f;
    private const float MAX_VERTICAL_SPEED = 9f;

    private Vector3 _accelerationDir = new();

    private Vector3 _collisionVector = new();
    private float _collisionPeriod = 0f;
    private Vector3 GetCollisionVector() {
        if (_collisionPeriod > Time.time)
            return Vector3.zero;
        return _collisionVector;
    }

    Animator _animator;
    private bool _isStanding = true;

    public override void Init() {
        Cursor.lockState = CursorLockMode.Locked;
        SetObjectId((int)Define.ObjectType.RacePlayer);

        Transform camTransform = transform.Find("PlayerCamera");
        if (camTransform != null)
            _camera = camTransform.GetComponent<TestPlayerCamera>();
        if (_camera != null)
            _camera.Init(transform.gameObject);

        _rigidBody = transform.GetComponent<Rigidbody>();

        _right = Vector3.Cross(Vector3.up, _front).normalized;

        Transform unityChan = transform.Find("SDUnityChan");
        if (unityChan != null) {
            _animator = unityChan.GetComponent<Animator>();
        }

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
        CheckGroundStatusOnFixedUpdate();
        SetStateOnFixedUpdate();
        CalculateVelocityOnFixedUpdate();
        LerpRotationOnUpdate();
        PlayAnimationOnFixedUpdate();
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

        Vector3 startPos = transform.position + _offset;
        if (Physics.Raycast(startPos, -_viewFront, out hitInfo, _cameraDistance))
            cameraPos = hitInfo.point;
        else
            cameraPos = startPos + (-_viewFront * _cameraDistance);

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

        if (_accelerationDir.sqrMagnitude > Vector3.kEpsilon) {
            _accelerationDir = _accelerationDir.normalized;
            _isStanding = false;
        }
        else {
            _accelerationDir = Vector3.zero;
            _isStanding = true;
        }    
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

    private void CheckGroundStatusOnFixedUpdate() {
        Vector3 rayStart = transform.position + _offset;

        RaycastHit hitInfo;
        bool hasHit = Physics.SphereCast(rayStart, _groundCheckRadius, Vector3.down, out hitInfo, _groundCheckDistance);

        _isGrounded = hasHit;

        Color debugColor = _isGrounded ? Color.green : Color.red;
        Debug.DrawRay(rayStart, Vector3.down * _groundCheckDistance, debugColor);
    }

    private void SetStateOnFixedUpdate() {
        if (!_isGrounded)
            _state = State.Jumping;
        else if (_accelerationDir.sqrMagnitude > Vector3.kEpsilon)
            _state = State.Moving;
        else
            _state = State.Standing;
    }

    private void PlayAnimationOnFixedUpdate() {
        if (_animator == null) return;
        _animator.SetBool("StandOrRun", _isStanding);

        if (_prestate != _state && _state == State.Jumping) {
            _animator.SetTrigger("JumpStart");
        }

        if (_state == State.Jumping) {
            if (_rigidBody.linearVelocity.y > 0f)
                _animator.SetBool("IsJumpingUp", true);
            else
                _animator.SetBool("IsJumpingUp", false);
        }

        if (_isGrounded && _prestate == State.Jumping) {
            _animator.SetTrigger("Land");
            _animator.SetBool("IsJumpingUp", true);
        }

        _prestate = _state;
    }

    private void CalculateVelocityOnFixedUpdate() {
        if (_rigidBody == null) return;

        //키보드 무빙 (수평)
        if (_accelerationDir.sqrMagnitude > 0.001f) {
            if (_state == State.Jumping)
                _rigidBody.AddForce(_accelerationDir * _accelerationRate / 3, ForceMode.Acceleration);
            else
                _rigidBody.AddForce(_accelerationDir * _accelerationRate, ForceMode.Acceleration);
        }

        //중력 (수직)
        if (_state == State.Jumping) {
            _rigidBody.AddForce(Vector3.down * _gravityAccel, ForceMode.Acceleration);
        }
        else {
            _rigidBody.AddForce(Vector3.down * 4f, ForceMode.Acceleration);
        }

        //충돌에 의한 힘
        _rigidBody.AddForce(GetCollisionVector(), ForceMode.Acceleration);

        //수평방향 저항
        Vector3 horizontalVelocity = new(_rigidBody.linearVelocity.x, 0f, _rigidBody.linearVelocity.z);
        if (horizontalVelocity.sqrMagnitude > 0.001f) {
            _rigidBody.AddForce(-horizontalVelocity * _horizonFrictionRatePerVelocity, ForceMode.Acceleration);
        }

        Vector3 currentVelocity = _rigidBody.linearVelocity;

        //점프 시동
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

    public void ApplyCollisionForceVector(Vector3 nestedForce) {
        _collisionVector = nestedForce;
        _collisionPeriod = Time.time + 0.1f;
    }

    public GameObjectMovementInfo SerializeMyMovementInfo() {
        Vector3 rvel = _rigidBody.linearVelocity;

        XYZ pos = new() {
            X = transform.position.x,
            Y = transform.position.y,
            Z = transform.position.z,
        };

        XYZ front = new() {
            X = _front.x,
            Y = _front.y,
            Z = _front.z,
        };

        XYZ vel = new() {
            X = rvel.x,
            Y = rvel.y,
            Z = rvel.z,
        };

        GameObjectMovementInfo serializedInfo = new() {
            Position = pos,
            Front = front,
            Velocity = vel,
            State = (int)_state,
        };
        return serializedInfo;
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
        if (_isGrounded)
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
