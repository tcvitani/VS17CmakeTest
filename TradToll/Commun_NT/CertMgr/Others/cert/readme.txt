To prepare correct pem files for CertMngr:

For CertMgr linked to old version of Botan v.1.10.17 use openssl-0.9.8y-i386-win32.zip.
-Extract the arhive content to a folder
-Copy convert_pfx_PrivateKey.bat and pfx file to convert to the same folder
-Update the input key password "-passin pass:123456" and target key pasword "-passout pass:123456" in bat file if different
-Execute convert_pfx_PrivateKey.bat script to get new p8key.pem file (old PKCS#4 format)


For CertMgr linked to newer versions of Botan v.2.17.2 use openssl-1.0.2p-i386-win32.zip or latter. Should also work with x64 binaries versions, but was not tested yet.
-Extract the arhive content to a folder
-Copy convert_pfx_PrivateKeyNew.bat and pfx file to convert  to the same folder
-Update the input key password "-passin pass:123456" and target key pasword "-passout pass:123456" in bat file if different
-Execute convert_pfx_PrivateKeyNew.bat script to get new p8key3.pem file  (in PKCS#5 v2.0 format)
