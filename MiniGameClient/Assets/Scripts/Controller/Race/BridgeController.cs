using UnityEngine;
using System.Collections;

public class BridgeController : MonoBehaviour {
    MeshRenderer _bridgeRenderer1;
    MeshRenderer _bridgeRenderer2;
    MeshRenderer _bridgeRenderer3;
    GameObject _bridgeColliderObject;
    Coroutine _currentDisableCoroutine;
    float _flickerTick = 0.3f;
    float _flickerTimer = 0f;
    bool _isFlickering = false;
    bool _isCollidable = false;
    bool _isRenderOnFlicker = true;

    public void Init() {
        Transform brTrans1 = transform.Find("ModularWood1");
        if (brTrans1 != null) _bridgeRenderer1 = brTrans1.GetComponent<MeshRenderer>();

        Transform brTrans2 = transform.Find("ModularWood2");
        if (brTrans2 != null) _bridgeRenderer2 = brTrans2.GetComponent<MeshRenderer>();

        Transform brTrans3 = transform.Find("ModularWood3");
        if (brTrans3 != null) _bridgeRenderer3 = brTrans3.GetComponent<MeshRenderer>();

        Transform brColliderTrans = transform.Find("TranslucentMatCollider");
        if (brColliderTrans != null) _bridgeColliderObject = brColliderTrans.gameObject;
    }

    void Update() {
        if (_isFlickering) {
            _flickerTimer += Time.deltaTime;
            if (_flickerTimer >= _flickerTick) {
                _flickerTimer = 0f;
                _isRenderOnFlicker = !_isRenderOnFlicker;
                if (_isRenderOnFlicker) {
                    ShowRenderer();
                } else {
                    HideRenderer();
                }
            }
        }
    }

    void HideRenderer() {
        if (_bridgeRenderer1 != null)
            _bridgeRenderer1.enabled = false;
        if (_bridgeRenderer2 != null)
            _bridgeRenderer2.enabled = false;
        if (_bridgeRenderer3 != null)
            _bridgeRenderer3.enabled = false;
    }

    void ShowRenderer() {
        if (_bridgeRenderer1 != null)
            _bridgeRenderer1.enabled = true;
        if (_bridgeRenderer2 != null)
            _bridgeRenderer2.enabled = true;
        if (_bridgeRenderer3 != null)
            _bridgeRenderer3.enabled = true;
    }

    void EnableCollider() {
        if (_bridgeColliderObject != null) {
            _bridgeColliderObject.SetActive(true);
            _isCollidable = true;
        }
            
    }

    void DisableCollider() {
        if (_bridgeColliderObject != null) {
            _bridgeColliderObject.SetActive(false);
            _isCollidable = false;
        }
    }

    public void DisableBridgeColliderAfterSecond(float second) {
        if (_currentDisableCoroutine != null) {
            StopCoroutine(_currentDisableCoroutine);
        }
        _currentDisableCoroutine = StartCoroutine(DisableSequenceRoutine(second));
    }

    public void EnableBridgeCollider()  {
        if (_currentDisableCoroutine != null) {
            StopCoroutine(_currentDisableCoroutine);
            _currentDisableCoroutine = null;
        }
        TurnOffFlicker(true);
        ShowRenderer();
        EnableCollider();
    }

    private IEnumerator DisableSequenceRoutine(float delaySecond) {
        if( _isCollidable) {
            TurnOnFlicker();
            yield return new WaitForSeconds(delaySecond);
            TurnOffFlicker(false);
            HideRenderer();
            DisableCollider();
        }
        _currentDisableCoroutine = null;
    }

    private void TurnOnFlicker() {       
        _isFlickering = true;
        //_flickerTimer = 0f;
    }

    private void TurnOffFlicker(bool isRender) {
        _isFlickering = false;
        _flickerTimer = 0f;
        _isRenderOnFlicker = isRender;
    }
}
