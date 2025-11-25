CertMgr C je za sada 32-bit wrapper lib za Botan. 
Botan: https://botan.randombit.net/ - Source: https://github.com/randombit/botan/.

Trenutno se linkamo na Botan verziju 1.10.17 bildanu u VS2017. Binary se nalazi na l:\_Third_Party_Libraries\Botan\botan_1.10.17\
******************************************************************
!!!PAZI - ne mogu se miksati debug i release verzija botan.dll. 
Ako se koristi debug verzija CertMgr (csr_certmgr.dll)- mora se koristiti debug dll (l:\_Third_Party_Libraries\Botan\botan_1.10.17\Debug\msvc-2017\botan.dll)
Ako se koristi release verzija CertMgr (csr_certmgr_r.dll) - mora se koristiti release dll (l:\_Third_Party_Libraries\Botan\botan_1.10.17\Release\msvc-2017\botan.dll ) 
******************************************************************

-Botan koristi PKCS8 PEM  ili BER format ključeva. Prije korištenja treba ih konvertiratio koristeći openssl 1.0.1m i više.

-Za extrakciju privatnog ključa iz pfx ključeva koristimo:
openssl.exe pkcs12 -in "CoreitPecatSoft.pfx" -nocerts -out "privateKey.pem" -passin pass:123456 -passout pass:123456

-Za konverziju privatnog ključa u PKCS8 PEM format koristimo:
openssl.exe pkcs8 -in privateKey.pem -topk8 -out "p8key.pem" -passin pass:123456 -passout pass:123456

-CertMgr koristi Visual studio toolkit v.141 Za koristiti na Win7 treba instalirat vc_redist.x86.exe (https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads)



