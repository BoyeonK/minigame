pushd %~dp0

set PROTOC_EXE_PATH=C:\vcpkg\installed\x64-windows\tools\protobuf\protoc.exe
set GRPC_PLUGIN_EXE_PATH=C:\vcpkg\installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe

"%PROTOC_EXE_PATH%" -I=./ --cpp_out=./ --grpc_out=./ --plugin=protoc-gen-grpc="%GRPC_PLUGIN_EXE_PATH%" ./S2D_Protocol.proto
IF ERRORLEVEL 1 PAUSE