using UnityEngine;
using System.Collections;

public class HammerController : MonoBehaviour {
    private Coroutine _swingCoroutine;

    void Start() {

    }

    void Update() {

    }

    public void SwingToLeft() {
        if (_swingCoroutine != null)
            StopCoroutine(_swingCoroutine);

        _swingCoroutine = StartCoroutine(RotateOverTime(new Vector3(-45f, 0f, 0f), 2f));
    }

    public void SwingToRight() {
        if (_swingCoroutine != null)
            StopCoroutine(_swingCoroutine);

        _swingCoroutine = StartCoroutine(RotateOverTime(new Vector3(45f, 0f, 0f), 2f));
    }

    private IEnumerator RotateOverTime(Vector3 targetEulerAngles, float duration) {
        Quaternion startRotation = transform.rotation;
        Quaternion targetRotation = Quaternion.Euler(targetEulerAngles);
        float elapsed = 0f;

        while (elapsed < duration) {
            float t = elapsed / duration;
            transform.rotation = Quaternion.Slerp(startRotation, targetRotation, t);

            elapsed += Time.deltaTime;
            yield return null;
        }

        transform.rotation = targetRotation;
        _swingCoroutine = null;
    }
}
