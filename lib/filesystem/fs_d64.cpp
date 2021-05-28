#include "fs_d64.h"

bool D64FileSystem::services(String name)
{
    return name.endsWith(".d64");
}

MFile D64FileSystem::create(String path)
{
    return D64File(path);
}