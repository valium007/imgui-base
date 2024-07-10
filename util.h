#include <stdio.h>
#include "imgui.h"

ImVec4 HexToIv4(int hex){
    float r = ((hex >> 16) & 0xff) / 255.0f;
    float g = ((hex >> 8) & 0xff) / 255.0f;
    float b = (hex & 0xff) / 255.0f;
    return ImVec4(r,g,b,1.0f);
}