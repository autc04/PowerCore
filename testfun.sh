export RETRO68=$HOME/Projects/Retro68-build/toolchain/bin

$RETRO68/powerpc-apple-macos-gcc -O -Wl,-e -Wl,start testfun.c -o testfun.xcoff -finhibit-size-directive
$RETRO68/powerpc-apple-macos-objdump -d testfun.xcoff
$RETRO68/powerpc-apple-macos-objcopy -S -j.text -Obinary testfun.xcoff testfun.bin

