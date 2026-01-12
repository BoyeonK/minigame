using System;
using UnityEngine;

public class ObjectiveLineController : MonoBehaviour {
    Action DeligaterOnPlayerCollision;

    public void Init(Action func) {
        DeligaterOnPlayerCollision = func;
    }

    public void ArrivedInLine() {
        DeligaterOnPlayerCollision?.Invoke();
    }
}
