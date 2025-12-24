@echo off
REM Download third-party dependencies (raylib and raygui)
cd /d %~dp0
if not exist ..\third_party mkdir ..\third_party
pushd ..\third_party

popd
nnecho Cloning raylib...
ngit clone https://github.com/raysan5/raylib.git
necho Cloning raygui...
ngit clone https://github.com/raygui/raygui.git
necho Done.