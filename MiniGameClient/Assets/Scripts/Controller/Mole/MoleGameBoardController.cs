using NUnit.Framework;
using System.Collections.Generic;
using UnityEngine;

public class MoleGameBoardController : MonoBehaviour {
    private List<SlotController> _slots = new();

    void Start() {
        Init();
    }

    void Init() {
        _slots.Clear();
        for (int i = 0; i <= 9; i++) {
            string slotName = $"Slot{i}";
            Transform slotTransform = transform.Find(slotName);

            if (slotTransform != null) {
                SlotController slotController = slotTransform.GetComponent<SlotController>();
                if (slotController != null) {
                    slotController.Init(KeyCode.Keypad0 + i, i);
                    _slots.Add(slotController);
                }  
            }
        }
    }

    private void OnDisable() {
        Clear();
    }

    void Clear() {
        foreach (SlotController slotController in _slots) { 
            slotController.Clear();
        }
    }

    void Update() {
        
    }
}
