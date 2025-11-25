
openssl.exe pkcs12 -in "CoreitPecatSoft.pfx" -nocerts -out privateKey.pem -passin pass:123456 -passout pass:123456

openssl pkcs8 -in privateKey.pem -topk8 -out p8key.pem -passin pass:123456 -passout pass:123456
