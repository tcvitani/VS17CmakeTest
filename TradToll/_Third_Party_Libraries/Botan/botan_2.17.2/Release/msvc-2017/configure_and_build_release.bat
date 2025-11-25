python configure.py --cc=msvc --os=windows --cpu x86_32 --library-suffix "2_r"

nmake
nmake check
nmake install