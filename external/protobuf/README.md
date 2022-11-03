# Amethyst API Protocol
Kinda idk why would you want to do this, but here you go.  
Now have fun or something. (Note: this is an explicit order)
## Preparations
- Install gRPC and proto tools (I'd recommend WSL to do that)  
  https://grpc.io/docs/languages/cpp/quickstart/#install-grpc
> Note: you can also find a [pure-windows guide for these steps here](https://sanoj.in/2020/05/07/working-with-grpc-in-windows.html)
## Generation
- Generate the protocol and gRPC source (still done on WSL)
```sh
protoc -I . --cpp_out=dllexport_decl=KTVR_API:. Amethyst_API.proto
protoc -I . --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` Amethyst_API.proto
```
> Note: you can also find a [pure-windows guide for these steps here](https://sanoj.in/2020/05/07/working-with-grpc-in-windows.html)
## Patches
- Add `#include "pch.h"` to the top of the generated `.cc` files
- Null the `PROTOBUF_CONSTINIT` define of the generated protocol `.cc`
```cpp
...
#include <google/protobuf/port_def.inc>

// Add after the upper line:
#define PROTOBUF_CONSTINIT
```
- Edit the generated protocol `.h`
```cpp
#ifndef GOOGLE_PROTOBUF_INCLUDED_Amethyst_5fAPI_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_Amethyst_5fAPI_2eproto

// Add after the upper boiler lines:
#ifdef AMETHYST_API_EXPORTS
#define KTVR_API __declspec(dllexport)
#else
#define KTVR_API __declspec(dllimport)
#endif
```
- Inline the generated gRPC source (from `.cc`) into the `.h`, delete the source `.cc`
- Explicitly delete `ktvr::Service`'s copy constructor and assignment operator
```cpp
// https://github.com/grpc/grpc/issues/15653
Service(const Service&) = delete;
Service& operator =(const Service&) = delete;
```
## Deployment
- Copy (and replace) the existing protocol in  
  `[Amethyst Solution]\Amethyst_API\Generated Files\`