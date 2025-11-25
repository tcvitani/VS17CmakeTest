
openssl.exe pkcs12 -in "CoreitPecatSoft.pfx" -nocerts -out privateKey.pem

openssl pkcs8 -in privateKey.pem -topk8 -out p8key.pem
