using UnityEngine;

public class StunBlur : MonoBehaviour {
    private float _stunActionTick = 0.0f;
    private bool _isStun = false;
    private float _period = 1f;

    void Update() {
        StunEffectOnUpdate();
    }

    public void Init() {
        gameObject.SetActive(false);
    }

    public void Stun() {
        _isStun = true;
        gameObject.SetActive(true);
        _stunActionTick = Time.time;
    }

    private void StunEffectOnUpdate() {
        if (!_isStun)
            return;

        if (Time.time > (_stunActionTick + _period)) {
            _isStun = false;
            gameObject.SetActive(false);
            return;
        }
    }
}
