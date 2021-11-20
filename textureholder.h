#pragma once
#include<SFML/Graphics.hpp>
#include <map>

using namespace sf;
using namespace std;

class TextureHolder
{
    map<string, Texture> m_Textures;
    TextureHolder() = default;
public:
    static TextureHolder & instance();
    Texture& GetTexture(string const& filename);
};
