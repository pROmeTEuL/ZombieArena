#include "textureholder.h"

TextureHolder &TextureHolder::instance()
{
    static TextureHolder th;
    return th;
}

Texture& TextureHolder::GetTexture(const string &filename)
{
    auto it = m_Textures.find(filename);
    if (it == m_Textures.end()) {
        m_Textures[filename].loadFromFile(filename);
        return m_Textures[filename];
    }
    return it->second;
}
