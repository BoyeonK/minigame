using System;
using System.Collections.Generic;
using UnityEngine;

public class LobbyScreenRenderer : MonoBehaviour {
    private List<Texture2D> _gamePreviewImage = new();
    private Renderer _renderer;
    
    public void Init() {
        Texture2D[] loadedTextures = Resources.LoadAll<Texture2D>("Images");
        _gamePreviewImage.AddRange(loadedTextures);
        _renderer = GetComponent<Renderer>();
        ApplyPicture(0);
    }

    public void HideThis() {
        gameObject.SetActive(false);
    }

    public void ActiveThis() {
        gameObject.SetActive(true);
    }

    public void ApplyPicture(int idx) {
        if (idx < 0 || idx >= _gamePreviewImage.Count) 
            return;
        _renderer.material.SetTexture("_BaseMap", _gamePreviewImage[idx]);
    }
}
