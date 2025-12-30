using UnityEngine;

public class RaceOpponentSuperficialBody : MonoBehaviour {
    private Vector3 _front = new(0f, 0f, 1f);
    private Vector3 _superficialPosition = new();
    private Vector3 _storedPos = new();
    private int _objectIdx = -1;
    private float _turnSpeed = 10f;
    private float _serverUpdateTick = 0.1f;
    private float _teleportDistance = 3f;
    private Vector3 _moveVelocityForSmoothDamp;
    private Animator _animator;
    private int _state = 0;
    private int _prestate = 0;
    private bool _wasJumpingUpState = true;

    public void Init(int objectIdx, Vector3 pos) {
        _objectIdx = objectIdx;
        _superficialPosition = pos;
        _animator = GetComponent<Animator>();

        _animator.SetBool("IsJumpingUp", true);
    }

    public int GetObjectIdx() { 
        return _objectIdx;
    }

    public void SetFrontVector(Vector3 front) {
        _front = front;
    }

    public void StoreMyPos() {
        _storedPos = _superficialPosition;
    }

    public void ApplyMyPos() {
        _superficialPosition = _storedPos;
    }

    public void RotateOnUpdate() {
        if (_front.sqrMagnitude < 0.001f)
            return;

        Quaternion targetRotation = Quaternion.LookRotation(_front);
        transform.rotation = Quaternion.Slerp(
            transform.rotation,
            targetRotation,
            _turnSpeed * Time.deltaTime
        );
    }

    public void MoveSuperficialPositionOnUpdate(Vector3 realPosition) {
        float disparity = (realPosition - _superficialPosition).sqrMagnitude;
        if (disparity > _teleportDistance) {
            _superficialPosition = realPosition;
            transform.position = _superficialPosition;
            _moveVelocityForSmoothDamp = Vector3.zero;
            return;
        }

        _superficialPosition = Vector3.SmoothDamp(
            _superficialPosition,
            realPosition,
            ref _moveVelocityForSmoothDamp,
            _serverUpdateTick
        );

        bool isJumpingUpNow = _superficialPosition.y < realPosition.y;
        if (_wasJumpingUpState != isJumpingUpNow) {
            _animator.SetBool("IsJumpingUp", isJumpingUpNow);
            _wasJumpingUpState = isJumpingUpNow;
        }
        transform.position = _superficialPosition;
    }

    public void SetState(int state) {
        _state = state;
        if (_prestate != _state) {
            UpdateAnimatorState();
            _prestate = _state;
        }
    }

    private void UpdateAnimatorState() {
        if (_animator == null)
            return;
        if (_prestate == 2) {
            _animator.SetTrigger("IsntJumping");
        }
        _animator.SetInteger("State", _state);
    }
}

