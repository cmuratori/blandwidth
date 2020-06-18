@echo off

set BaseName=blandwidth
set CLLinkFlags=-nodefaultlib -incremental:no -opt:ref -machine:x64 -STACK:0x100000,0x100000 -manifest:no -subsystem:console user32.lib kernel32.lib
set CLCompileFlags=-Zi -d2Zi+ -Gy -GF -Gs9999999 -GS- -GR- -EHs- -EHc- -EHa- -WX -W4 -nologo -FC -Gm- -diagnostics:column -fp:except- -fp:fast
set CLANGCompileFlags=-g -fno-autolink -nostdlib -nostdlib++ -mno-stack-arg-probe
set CLANGLinkFlags=-fuse-ld=lld -Wl,-subsystem:console,user32.lib,kernel32.lib

IF NOT EXIST build mkdir build
pushd build

echo -----------------
echo Building debug...
call cl -Fe%BaseName%_debug_msvc.exe -Od %CLCompileFlags% ../win32_%BaseName%.c /link %CLLinkFlags%
call clang++ %common% %CLANGCompileFlags% %CLANGLinkFlags% -x c ../win32_%BaseName%.c -o %BaseName%_debug_clang.exe

echo -----------------
echo Building release...
call cl -Fe%BaseName%_release_msvc.exe -Oi -Oxb2 -O2 %CLCompileFlags% ../win32_%BaseName%.c /link %CLLinkFlags% -RELEASE
call clang++ %common% -O3 %CLANGCompileFlags% %CLANGLinkFlags% -x c ../win32_%BaseName%.c -o %BaseName%_release_clang.exe

echo -----------------
echo Generating analysis...
call clang++ %common% -O3 -DLLVM_MCA=1 %CLANGCompileFlags% -x c ../win32_%BaseName%.c -mllvm -x86-asm-syntax=intel -S -o %BaseName%_release_clang.asm
call llvm-mca %BaseName%_release_clang.asm > %BaseName%_release_clang.mca

popd