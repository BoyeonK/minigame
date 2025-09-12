pushd %~dp0

set PROTOC_EXE_PATH=C:\vcpkg\installed\x64-windows\tools\protobuf\protoc.exe

"%PROTOC_EXE_PATH%" -I=./ --cpp_out=./ --csharp_out=./ ./S2C_Protocol.proto ./S2C_Protocol_Ingame.proto ./S2C_Protocol_Enum.proto
IF ERRORLEVEL 1 PAUSE