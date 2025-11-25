
openssl.exe pkcs12 -in "CoreitPecatSoft.pfx" -nocerts -out privateKey.pem -passin pass:123456 -passout pass:123456

openssl pkcs8 -in privateKey.pem -topk8 -out p8key.pem -passin pass:123456 -passout pass:123456

REM For new Botan v.2.0.0 and openssl 1.0.2 and later we must use PKCS#5 v2.0 format of PEM file
openssl pkcs8 -v2 aes256 -topk8  -in privateKey.pem -out p8key3.pem  -passin pass:123456 -passout pass:123456
