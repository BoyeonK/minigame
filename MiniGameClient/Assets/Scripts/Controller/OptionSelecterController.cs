using UnityEngine;

public class OptionSelecterController : MonoBehaviour {
    private int _opt = 5;
    
    private Vector3 _lookDirection;
    private Vector3 _eyePosition;
    private Camera _mainCamera;
    [SerializeField]
    private float _rotationSpeed = 5f;
    [SerializeField]
    private float _positionSpeed = 7f;
    void Awake() {
        _mainCamera = Camera.main;
    }

    public void SetOpt(int opt) {
        _opt = opt;
        ChangeLookDir();
    }

    private void ChangeLookDir() {
        switch (_opt) {
            case 0:
                _lookDirection = new Vector3(0.0f, 0.2f, 1.0f).normalized;
                _eyePosition = new Vector3(0.0f, 4f, 31f);
                break;
            case 1:
                _lookDirection = new Vector3(0.0f, 0.0f, 1.0f).normalized;
                _eyePosition = new Vector3(-4.6f, 2.5f, 36.7f);
                break;
            case 2:
                _lookDirection = new Vector3(1.0f, -0.445f, 0f).normalized;
                _eyePosition = new Vector3(9.6f, 3.0f, 27.6f);
                break;
            case 3:
                _lookDirection = new Vector3(-1.0f, -0.75f, 0.0f).normalized;
                _eyePosition = new Vector3(-8.6f, 3f, 26.7f);
                break;
            case 4:
                _lookDirection = new Vector3(0, -0.53f, -1.0f).normalized;
                _eyePosition = new Vector3(4.7f, 3.1f, 21.6f);
                break;
            case 5:
                _lookDirection = new Vector3(0.0f, 0.0f, 1.0f).normalized;
                _eyePosition = new Vector3(0.0f, 0.7f, 1.0f);
                break;
            case 6:
                _lookDirection = new Vector3(0.0f, 0.2f, 1.0f).normalized;
                _eyePosition = new Vector3(0.0f, 1f, 10.0f);
                break;
        }
    }

    //OnEnable과 Start는 비슷하면서도 다르다. 스크립트의 생명주기와 재사용성에 연관이 있다.
    private void OnEnable() {
        _opt = 5;
        ChangeLookDir(); 
    }

    //동일하게 OnDisable과 OnDestroy도 그렇다.
    private void OnDisable() {
        
    }

    void Update() {
        if (_mainCamera != null && _lookDirection != Vector3.zero) {
            Quaternion targetRotation = Quaternion.LookRotation(_lookDirection);

            //카메라 위치 이동
            _mainCamera.transform.position = Vector3.Lerp(
                _mainCamera.transform.position,
                _eyePosition,
                Time.deltaTime * _positionSpeed
            );

            //카메라가 바라보는 방향 벡터 회전
            _mainCamera.transform.rotation = Quaternion.Slerp(
                _mainCamera.transform.rotation,
                targetRotation,
                Time.deltaTime * _rotationSpeed
            );
        }
    }
}
