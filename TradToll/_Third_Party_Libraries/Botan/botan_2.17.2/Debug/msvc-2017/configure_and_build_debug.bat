python configure.py --cc=msvc --os=windows --cpu x86_32 --debug-mode --with-debug-info --library-suffix "2_d"

nmake
nmake check
nmake install