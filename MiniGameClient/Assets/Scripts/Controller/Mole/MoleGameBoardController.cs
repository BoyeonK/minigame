using NUnit.Framework;
using System;
using System.Collections.Generic;
using UnityEngine;

public class MoleGameBoardController : MonoBehaviour {
    private List<SlotController> _slots = new();

    public void Init() {
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

    public void SetSlotState(int slotIdx, int state) {
        if (slotIdx < 0 || slotIdx > 9)
            return;

        _slots[slotIdx].SetState(state);
    }

    public void ShowPointText(int slotIdx, int point, bool isMine) {
        if (slotIdx < 0 || slotIdx > 9)
            return;

        _slots[slotIdx].ShowPText(point, isMine);
    }

    void Clear() {
        foreach (SlotController slotController in _slots) { 
            slotController.Clear();
        }
    }

    void Update() {
        
    }
}
