python configure.py --cc=msvc --os=windows --cpu x86_32 --enable-debug

nmake
nmake check
nmake install