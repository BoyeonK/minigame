using UnityEditor;
using UnityEngine;
using System.IO;

public class CustomBuildScript
{
    // 1. MenuItem: Tools/Run Multiplayer/4 Players
    [MenuItem("Tools/Run Muitiplayer/4 Players")] // 오타: "Muitiplayer" -> "Multiplayer"
    static void PerformWin64Build4()
    {
        PerformWin64Build(4);
    }

    // 2. PerformWin64Build 함수
    static void PerformWin64Build(int playerCount)
    {
        // 빌드 타겟 그룹 전환
        // BuildTarget.StandaloneWindows는 32비트/64비트 모두 포함하지만, BuildTarget.StandaloneWindows64로 명시하는 것이 좋습니다.
        // 이미 아래 BuildPlayer에서 StandaloneWindows64를 지정하고 있으므로, 이 라인 자체는 없어도 작동할 수 있습니다.
        // 하지만 특정 플랫폼으로 전환하는 습관은 좋습니다.
        EditorUserBuildSettings.SwitchActiveBuildTarget(
            BuildTargetGroup.Standalone, BuildTarget.StandaloneWindows64); // BuildTarget.StandaloneWindows -> BuildTarget.StandaloneWindows64

        string projectName = GetProjectName();
        string baseBuildPath = "Builds/Win64/"; // 모든 클라이언트 빌드의 상위 폴더

        for (int i = 0; i < playerCount; i++)
        {
            // 각 클라이언트의 빌드 경로 설정
            // Path.Combine을 사용하여 OS별 경로 구분자를 처리하는 것이 안전하고 좋습니다.
            string clientFolderName = $"{projectName}{i.ToString()}"; // 예: MyGame0, MyGame1
            string specificBuildPath = Path.Combine(baseBuildPath, clientFolderName);
            string executablePath = Path.Combine(specificBuildPath, $"{projectName}{i.ToString()}.exe");

            // 빌드 폴더가 없으면 생성 (필수적이지는 않지만, 확실성을 위해 좋음)
            if (!Directory.Exists(specificBuildPath))
            {
                Directory.CreateDirectory(specificBuildPath);
            }

            Debug.Log($"빌드 시작: {executablePath}");

            BuildPipeline.BuildPlayer(GetScenePaths(),
                                     executablePath,
                                     BuildTarget.StandaloneWindows64, // BuildTargetGroup과 일치
                                     BuildOptions.AutoRunPlayer // BuildOptions.None으로 변경 고려
                                    );
            Debug.Log($"빌드 완료: {executablePath}");
        }
        Debug.Log("모든 클라이언트 빌드가 완료되었습니다!");
        EditorUtility.RevealInFinder(baseBuildPath); // 빌드 완료 후 빌드 폴더 열기
    }

    // 3. GetProjectName 함수
    static string GetProjectName()
    {
        // Application.dataPath는 Assets 폴더의 경로입니다.
        // 예를 들어 "C:/UnityProjects/MyGame/Assets"
        // Split('/') 하면 {"C:", "UnityProjects", "MyGame", "Assets"}
        // s[s.Length - 2]는 "MyGame"을 반환합니다. 정확한 프로젝트 이름 추출 방식입니다.
        string[] s = Application.dataPath.Split('/');
        return s[s.Length - 2];
    }

    // 4. GetScenePaths 함수
    static string[] GetScenePaths()
    {
        // Build Settings에 있는 활성화된 씬만 가져오는 것이 더 안전합니다.
        var scenePaths = new System.Collections.Generic.List<string>();
        foreach (EditorBuildSettingsScene scene in EditorBuildSettings.scenes)
        {
            if (scene.enabled)
            { // 활성화된 씬만 포함
                scenePaths.Add(scene.path);
            }
        }
        return scenePaths.ToArray();
    }
}