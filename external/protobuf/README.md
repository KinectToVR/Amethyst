# Amethyst API Protocol
## Compilation
- Create the output folder
```powershell
mkdir ./out
```
- Generate the protocol source
```powershell
protoc --proto_path=./ --cpp_out=dllexport_decl=KTVR_API:./out ./Amethyst_API.proto
```
- Edit the generated protocol source
```cpp
// Add at the top:
#include "pch.h"

...
#include <google/protobuf/port_def.inc>

// Add after the upper line:
#define PROTOBUF_CONSTINIT
```
- Edit the generated protocol header
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
- Copy (and replace) the existing protocol in  
  `[Amethyst Solution]\Amethyst_API\Generated Files\`