TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lsfml-audio -lsfml-graphics -lsfml-network -lsfml-system -lsfml-window

SOURCES += \
        bullet.cpp \
        create_background.cpp \
        main.cpp \
        pickup.cpp \
        player.cpp \
        textureholder.cpp \
        zombie.cpp \
        zombie_arena.cpp

DISTFILES += \
    fonts/zombiecontrol.ttf \
    graphics/ammo_icon.png \
    graphics/ammo_pickup.png \
    graphics/background.png \
    graphics/background_sheet.png \
    graphics/bloater.png \
    graphics/blood.png \
    graphics/chaser.png \
    graphics/crawler.png \
    graphics/crosshair.png \
    graphics/health_pickup.png \
    graphics/player.png \
    graphics/sample.png \
    sound/hit.wav \
    sound/pickup.wav \
    sound/powerup.wav \
    sound/reload.wav \
    sound/reload_failed.wav \
    sound/shoot.wav \
    sound/splat.wav


HEADERS += \
    bullet.h \
    pickup.h \
    player.h \
    textureholder.h \
    zombie.h \
    zombie_arena.h

