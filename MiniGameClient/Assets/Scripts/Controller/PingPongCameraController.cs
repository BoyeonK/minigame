using UnityEngine;

public class PingPongCameraController : MonoBehaviour {
    int _playerIdx = -1;
    Vector3 _pointer = new(0, -1.7f, 0f);
    Vector3 _Spos = new(0f, 25f, 0f);
    Vector3 _Epos = new(0f, 25f, 0f);
  
    private float _transDuration = 2f;
    private float _elapsedTime = 0f;
    private bool _isOnTransition = false;

    public void SetPlayerIdx(int playerIdx) {
        Debug.Log("작동은 해요");
        _playerIdx = playerIdx;
        switch (_playerIdx) {
            case 0:
                _Epos = new Vector3(14.5f, 9.2f, 0f);
                break;
            case 1:
                _Epos = new Vector3(-14.5f, 9.2f, 0f);
                break;
            case 2:
                _Epos = new Vector3(0f, 9.2f, -14.5f);
                break;
            case 3:
                _Epos = new Vector3(0f, 9.2f, 14.5f);
                break;
            default:
                return;
        }

        _elapsedTime = 0f;
        _isOnTransition = true;
    }

    private void DisableMySideWall() {
        GameObject wall = GameObject.Find("Wall");
        if (wall == null)
            return;

        switch (_playerIdx) {
            case 0:
                Transform eastWallTransform = wall.transform.Find("EastWall");
                if (eastWallTransform != null)
                    eastWallTransform.gameObject.SetActive(false);
                break;
            case 1:
                Transform westWallTransform = wall.transform.Find("WestWall");
                if (westWallTransform != null)
                    westWallTransform.gameObject.SetActive(false);
                break;
            case 2:
                Transform southWallTransform = wall.transform.Find("SouthWall");
                if (southWallTransform != null)
                    southWallTransform.gameObject.SetActive(false);
                break;
            case 3:
                Transform northWallTransform = wall.transform.Find("NorthWall");
                if (northWallTransform != null)
                    northWallTransform.gameObject.SetActive(false);
                break;
            default:
                return;
        }
    }

    private void Update() {
        transform.LookAt(_pointer);
        if (!_isOnTransition) return;

        _elapsedTime += Time.deltaTime;
        float t = Mathf.Clamp01(_elapsedTime / _transDuration);

        transform.position = Vector3.Lerp(_Spos, _Epos, t);

        if (t >= 1.0f) {
            _isOnTransition = false;
            Debug.Log("카메라 이동 전환이 완료되었습니다.");
            transform.position = _Epos;

            DisableMySideWall();
        }
    }
}
