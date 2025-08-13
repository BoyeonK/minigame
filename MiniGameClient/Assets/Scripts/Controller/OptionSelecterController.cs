using UnityEngine;

public class OptionSelecterController : MonoBehaviour {
    private int _opt = 0;
    
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
                _lookDirection = new Vector3(0.0f, 0.0f, 1.0f).normalized;
                _eyePosition = new Vector3(0.0f, 0.5f, 25.0f);
                break;
            case 1:
                _lookDirection = new Vector3(-0.2f, 0.0f, 1.0f).normalized;
                _eyePosition = new Vector3(0.0f, 0.5f, 25.0f);
                break;
            case 2:
                _lookDirection = new Vector3(1.0f, 0.0f, 0f).normalized;
                _eyePosition = new Vector3(0.0f, 0.5f, 25.0f);
                break;
            case 3:
                _lookDirection = new Vector3(-1.0f, 0.0f, -0.2f).normalized;
                _eyePosition = new Vector3(0.0f, 0.5f, 25.0f);
                break;
            case 4:
                _lookDirection = new Vector3(0.0f, 0.0f, 1.0f).normalized;
                _eyePosition = new Vector3(0.0f, 0.5f, 1.0f);
                break;
            case 5:
                _lookDirection = new Vector3(0.0f, 0.3f, 1.0f).normalized;
                _eyePosition = new Vector3(0.0f, 0.5f, 10.0f);
                break;
        }
    }

    //OnEnable�� Start�� ����ϸ鼭�� �ٸ���. ��ũ��Ʈ�� �����ֱ�� ���뼺�� ������ �ִ�.
    private void OnEnable() {
        _opt = 4;
        ChangeLookDir(); 
    }

    //�����ϰ� OnDisable�� OnDestroy�� �׷���.
    private void OnDisable() {
        
    }

    void Update() {
        if (_mainCamera != null && _lookDirection != Vector3.zero) {
            Quaternion targetRotation = Quaternion.LookRotation(_lookDirection);

            //ī�޶� ��ġ �̵�
            _mainCamera.transform.position = Vector3.Lerp(
                _mainCamera.transform.position,
                _eyePosition,
                Time.deltaTime * _positionSpeed
            );

            //ī�޶� �ٶ󺸴� ���� ���� ȸ��
            _mainCamera.transform.rotation = Quaternion.Slerp(
                _mainCamera.transform.rotation,
                targetRotation,
                Time.deltaTime * _rotationSpeed
            );

            
        }
    }
}
