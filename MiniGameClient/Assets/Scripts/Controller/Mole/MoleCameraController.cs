using UnityEngine;
using UnityEngine.UIElements;

public class MoleCameraController : MonoBehaviour {
    private readonly Vector3 _cameraPos = new(0.0f, 3.5f, -3.6f);

    private float _stunActionTick = 0.0f;
    private bool _isStun = false;
    private float _period = 0.25f;
    private float _max = 0.1f;

    public void Init() {
        gameObject.transform.position = _cameraPos;
    }

    void Update() {
        StunEffectOnUpdate();
    }

    public void Stun() {
        _isStun = true;
        _stunActionTick = Time.time;
    }

    private void StunEffectOnUpdate() {
        if (!_isStun)
            return;

        if (Time.time > (_stunActionTick + _period)) {
            _isStun = false;
            gameObject.transform.position = _cameraPos;
            return;
        }

        float Y = _cameraPos.y + GetDeltaPos(Time.time - _stunActionTick, _period);
        gameObject.transform.position = new Vector3(_cameraPos.x, Y, _cameraPos.z);
    }

    private float GetDeltaPos(float t, float T) {
        float damp = Mathf.Exp(-t/T);
        float vib = Mathf.Sin(2 * Mathf.PI * t / T);
        return _max * damp * vib;
    }
}
