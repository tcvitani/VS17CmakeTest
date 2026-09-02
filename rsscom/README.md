# RssCom

Asynchronous communication lib

## Remaining test to perform
- [ ] Connection brokers : sans et avec support TLS 
- [ ] Connection brokers authent: PLAINTEXT et support SASL (et/ou certificat client) 
- [ ] Producer :Gestion des acks : ack=[0,1,all]
- [ ] Producer : Definition des clés 
- [ ] Producer : Definition de la partition. uniquement pour test a éviter sinon. 
- [ ] Producer : On pourrait aussir définir un partitionneur custom, mais je pense aussi qu’on a pas besoin de ça maintenant.
- [ ] Consumer : Définition d’un group-id
- [ ] Consumer : commit sync et async

To BUILD:

Install Conan for win version >2.25 

Start "x64 Native Tools Command Prompt for VS 2022 LTSC 17.4.lnk" 

cmake -B build -S . -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=.\cmake-conan\conan_provider.cmake -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release 

cmake -B build -S . -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=.\cmake-conan\conan_provider.cmake -DCMAKE_BUILD_TYPE=Debug

cmake --build build --config Debug