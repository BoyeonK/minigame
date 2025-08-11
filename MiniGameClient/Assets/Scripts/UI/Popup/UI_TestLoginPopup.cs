using UnityEngine;

public class UI_TestLoginPopup : UI_Popup {
    /*
    enum Texts {
        AddButtonText,
        ScoreText
    }

    enum Buttons {
        AddButton
    }

    enum GameObjects {
        TestObj
    }

    enum Images {
        ItemIcon,
    }

    int _score = 0;
    private void Start() {
        Init();
    }
    */

    public override void Init() {
        base.Init();
        /*
        Bind<Button>(typeof(Buttons));
        Bind<Text>(typeof(Texts));
        Bind<GameObject>(typeof(GameObjects));
        Bind<Image>(typeof(Images));

        GameObject Btn = GetButton((int)Buttons.AddButton).gameObject;
        AddUIEvent(Btn, OnButtonClicked, Define.UIEvent.Click);

        GameObject go = GetImage((int)Images.ItemIcon).gameObject;
        AddUIEvent(go, (PointerEventData data) => { go.transform.position = data.position; }, Define.UIEvent.Drag);
        */
    }

    /*
    public void OnButtonClicked(PointerEventData data)
    {
        _score++;
        Get<Text>((int)Texts.ScoreText).text = $"{_score}";
    }
    */
}
