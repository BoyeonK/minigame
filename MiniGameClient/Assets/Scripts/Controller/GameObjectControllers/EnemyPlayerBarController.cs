using System;
using UnityEngine;

public class EnemyPlayerBarController : GameObjectController {
    Vector3 _realPosition;
    Vector3 _superficialPosition;

    Vector3 _startLerpPosition;
    private float _lerpDuration = 0.1f;
    private float _elapsedTime = 0f;
    private bool _isLerping = false;

    private void Start() {
        _realPosition = transform.position;
        _superficialPosition = transform.position;
        _startLerpPosition = transform.position;
    }

    public void SetObjectId(int id) {
        _objectId = id;
    }

    public void SetPosition(float x, float z) {
        Vector3 now = transform.position;
        now.x = x;
        now.z = z;
        transform.position = now;
    }

    public void NewSetPosition(float x, float z) {
        _realPosition = new Vector3(x, transform.position.y, z);

        _startLerpPosition = _superficialPosition;
        _elapsedTime = 0f;
        _isLerping = true;
    }

    private void Update() {
        if (!_isLerping) {
            return;
        }

        _elapsedTime += Time.deltaTime;
        float t = Mathf.Clamp01(_elapsedTime / _lerpDuration);

        _superficialPosition = Vector3.Lerp(_startLerpPosition, _realPosition, t);
        transform.position = _superficialPosition;

        if (t >= 1.0f) {
            _isLerping = false;
            transform.position = _realPosition;
            _superficialPosition = _realPosition;
        }
    }
}
