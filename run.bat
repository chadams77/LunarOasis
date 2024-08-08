@del /Q build\LunarOasis.exe
@CL /Os /Iinclude main.cpp /link opengl32.lib lib\sfml-window.lib lib\sfml-system.lib lib\sfml-network.lib lib\sfml-graphics.lib lib\sfml-audio.lib lib\openal32.lib lib\flac.lib lib\freetype.lib lib\ogg.lib lib\vorbis.lib lib\vorbisfile.lib lib\vorbisenc.lib /out:build/LunarOasis.exe
@del main.obj
@del /Q build\sprites
@del /Q build\sfx
@xcopy sprites build\sprites /i /E
@xcopy sfx build\sfx /i /E
@cd build/
@LunarOasis.exe
@cd ..