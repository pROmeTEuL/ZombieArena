#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>
#include <fstream>
#include "player.h"
#include "zombie_arena.h"
#include "textureholder.h"
#include "bullet.h"
#include "pickup.h"

using namespace sf;

int main()
{
    enum class State {PAUSED, LEVELING_UP, GAME_OVER, PLAYING};
    State state = State::GAME_OVER;
    Vector2f resolution;
    resolution.x = VideoMode::getDesktopMode().width;
    resolution.y = VideoMode::getDesktopMode().height;
    RenderWindow window(VideoMode(resolution.x, resolution.y), "Zombie Arena", Style::Fullscreen);
    View mainView(FloatRect(0, 0, resolution.x, resolution.y));
    Clock clock;
    Time gameTimeTotal;
    Vector2f mouseWorldPosition;
    Vector2i mouseScreenPosition;
    Player player;
    IntRect arena;
    VertexArray background;
    Texture &textureBackground = TextureHolder::instance().GetTexture("graphics/background_sheet.png");
    int numZombies;
    int numZombiesAlive;
    Zombie* zombies = nullptr;
    Bullet bullets[100];
    int currentBullet = 0;
    int bulletsSpare = 24;
    int bulletsInClip = 6;
    int clipSize = 6;
    float fireRate = 1;
    Time lastPressed;
    window.setMouseCursorVisible(false);
    Sprite spriteCrosshair;
    Texture textureCrosshair = TextureHolder::instance().GetTexture("graphics/crosshair.png");
    spriteCrosshair.setTexture(textureCrosshair);
    spriteCrosshair.setOrigin(25, 25);
    Pickup healthPickup(1);
    Pickup ammoPickup(2);

    /*******************
     ********HUD********
     *******************/
    int score = 0, hiscore = 0;
    Sprite spriteGameOver = Sprite(TextureHolder::instance().GetTexture("graphics/background.png"));
    spriteGameOver.setPosition(0, 0);
    View hudView(FloatRect(0, 0, resolution.x, resolution.y));
    Sprite spriteAmmoIcon = Sprite(TextureHolder::instance().GetTexture("graphics/ammo_icon.png"));
    spriteAmmoIcon.setPosition(20, 980);
    Font font;
    font.loadFromFile("fonts/zombiecontrol.ttf");
    //pauza
    Text pausedText;
    pausedText.setFont(font);
    pausedText.setCharacterSize(155);
    pausedText.setFillColor(Color::White);
    pausedText.setPosition(400, 400);
    pausedText.setString("Press Enter \nto continue");
    //rip
    Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(125);
    gameOverText.setFillColor(Color::White);
    gameOverText.setPosition(250, 850);
    gameOverText.setString("Press Enter to play");
    //lvl up
    Text levelUpText;
    levelUpText.setFont(font);
    levelUpText.setCharacterSize(80);
    levelUpText.setFillColor(Color::White);
    levelUpText.setPosition(150, 250);
    std::stringstream levelUpStream;
    levelUpStream <<
                     "1- Increased rate of fire" <<
                     "\n2- Increased clip size(next reload)" <<
                     "\n3- Increased max health" <<
                     "\n4- Increased run speed" <<
                     "\n5- More and better health pickups" <<
                     "\n6- More and better ammo pickups";
    levelUpText.setString(levelUpStream.str());
    //munitie
    Text ammoText;
    ammoText.setFont(font);
    ammoText.setCharacterSize(55);
    ammoText.setFillColor(Color::White);
    ammoText.setPosition(100, 980);
    //scor
    Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(55);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition(20, 0);
    //cmm scor
    std::ifstream inputFile("gamedata/scores.txt");
    if (inputFile.is_open()) {
        inputFile >> hiscore;
        inputFile.close();
    }
    Text hiScoreText;
    hiScoreText.setFont(font);
    hiScoreText.setCharacterSize(55);
    hiScoreText.setFillColor(Color::Blue);
    hiScoreText.setPosition(1400, 0);
    std::stringstream hs;
    hs << "Hi Score:" << hiscore;
    hiScoreText.setString(hs.str());
    //zombi ramasi
    Text zombiesRemText;
    zombiesRemText.setFont(font);
    zombiesRemText.setCharacterSize(55);
    zombiesRemText.setFillColor(Color::Red);
    zombiesRemText.setPosition(1500, 980);
    zombiesRemText.setString("Zombies: 100");
    //val
    int wave = 0;
    Text waveNumberText;
    waveNumberText.setFont(font);
    waveNumberText.setCharacterSize(55);
    waveNumberText.setFillColor(Color::White);
    waveNumberText.setPosition(1250, 980);
    waveNumberText.setString("Wave: 0");
    //bara de viata
    RectangleShape healthBar;
    healthBar.setFillColor(Color::Green);
    healthBar.setPosition(450, 980);
    int framesSinceLastHUDUpdate = 0;
    int fpsMeasurementFrameInterval = 1000;
    /*******************
     *******AUDIO*******
     *******************/
    //lovin
    SoundBuffer hitBuffer;
    hitBuffer.loadFromFile("sound/hit.wav");
    Sound hit;
    hit.setBuffer(hitBuffer);
    //pleosc
    SoundBuffer splatBuffer;
    splatBuffer.loadFromFile("sound/splat.wav");
    Sound splat;
    splat.setBuffer(splatBuffer);
    //impuscatura
    SoundBuffer shootBuffer;
    shootBuffer.loadFromFile("sound/shoot.wav");
    Sound shoot;
    shoot.setBuffer(shootBuffer);
    //incarcare
    SoundBuffer reloadBuffer;
    reloadBuffer.loadFromFile("sound/reload.wav");
    Sound reload;
    reload.setBuffer(reloadBuffer);
    //incarcare esuata
    SoundBuffer reloadFailedBuffer;
    reloadFailedBuffer.loadFromFile("sound/reload_failed.wav");
    Sound reloadFailed;
    reloadFailed.setBuffer(reloadFailedBuffer);
    //imbunatatire
    SoundBuffer powerUpBuffer;
    powerUpBuffer.loadFromFile("sound/powerup.wav");
    Sound powerUp;
    powerUp.setBuffer(powerUpBuffer);
    //luat
    SoundBuffer pickupBuffer;
    pickupBuffer.loadFromFile("sound/pickup.wav");
    Sound pickup;
    pickup.setBuffer(pickupBuffer);
    /*******************
     *******WHILE*******
     *******************/
    while (window.isOpen()) {
        /*******************
         *******INPUT*******
         *******************/
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Return && state == State::PLAYING) {
                    state = State::PAUSED;
                } else if (event.key.code == Keyboard::Return && state == State::PAUSED) {
                    state = State::PLAYING;
                    clock.restart();
                } else if (event.key.code == Keyboard::Return && state == State::GAME_OVER) {
                    state = State::LEVELING_UP;
                    wave = 0;
                    score = 0;
                    currentBullet = 0;
                    bulletsSpare = 24;
                    bulletsInClip = 6;
                    clipSize = 6;
                    fireRate = 1;
                    player.resetPlayerStats();
                }
                if (state == State::PLAYING) {
                    if (event.key.code == Keyboard::R) {
                        if (bulletsSpare >= clipSize) {
                            bulletsInClip = clipSize;
                            bulletsSpare -= clipSize;
                            reload.play();
                        } else if (bulletsSpare > 0) {
                            bulletsInClip = bulletsSpare;
                            bulletsSpare = 0;
                            reload.play();
                        } else {
                            //De completat!!!
                            reloadFailed.play();
                        }
                    }
                }
            }
        }
        if (Keyboard::isKeyPressed(Keyboard::Escape))
            window.close();
        if (state == State::PLAYING) {
            if (Keyboard::isKeyPressed(Keyboard::W))
                player.moveUp();
            else
                player.stopUp();
            if (Keyboard::isKeyPressed(Keyboard::A))
                player.moveLeft();
            else
                player.stopLeft();
            if (Keyboard::isKeyPressed(Keyboard::S))
                player.moveDown();
            else
                player.stopDown();
            if (Keyboard::isKeyPressed(Keyboard::D))
                player.moveRight();
            else
                player.stopRight();
            if (Mouse::isButtonPressed(Mouse::Left)) {
                if (gameTimeTotal.asMilliseconds() - lastPressed.asMilliseconds() > 1000 / fireRate && bulletsInClip > 0) {
                    bullets[currentBullet].shoot(player.getCenter().x, player.getCenter().y, mouseWorldPosition.x, mouseWorldPosition.y);
                    ++currentBullet;
                    if (currentBullet > 99)
                        currentBullet = 0;
                    lastPressed = gameTimeTotal;
                    shoot.play();
                    --bulletsInClip;
                }
            }
        }
        if (state == State::LEVELING_UP) {
            if (event.key.code == Keyboard::Num1) {
                ++fireRate;
                state = State::PLAYING;
            }
            if (event.key.code == Keyboard::Num2) {
                ++clipSize;
                state = State::PLAYING;
            }
            if (event.key.code == Keyboard::Num3) {
                player.upgradeHealth();
                state = State::PLAYING;
            }
            if (event.key.code == Keyboard::Num4) {
                player.upgradeSpeed();
                state = State::PLAYING;
            }
            if (event.key.code == Keyboard::Num5) {
                healthPickup.upgrade();
                state = State::PLAYING;
            }
            if (event.key.code == Keyboard::Num6) {
                ammoPickup.upgrade();
                state = State::PLAYING;
            }
            if (state == State::PLAYING) {
                ++wave;
                arena.width = 500 * wave;
                arena.height = 500 * wave;
                arena.left = 0;
                arena.top = 0;
                int tileSize = createBackground(background, arena);
                player.spawn(arena, resolution, tileSize);
                healthPickup.setArena(arena);
                ammoPickup.setArena(arena);
                numZombies = 5 * wave;
                delete[] zombies;
                zombies = createHorde(numZombies, arena);
                numZombiesAlive = numZombies;
                powerUp.play();
                clock.restart();
            }
        }
        /********************
         *******UPDATE*******
         ********************/
        if (state == State::PLAYING) {
            Time dt = clock.restart();
            gameTimeTotal += dt;
            float dtAsSeconds = dt.asSeconds();
            mouseScreenPosition = Mouse::getPosition();
            mouseWorldPosition = window.mapPixelToCoords(Mouse::getPosition(), mainView);
            spriteCrosshair.setPosition(mouseWorldPosition);
            player.update(dtAsSeconds, Mouse::getPosition());
            Vector2f playerPosition(player.getCenter());
            mainView.setCenter(player.getCenter());
            for (int i = 0; i < numZombies; ++i) {
                if (zombies[i].isAlive())
                    zombies[i].update(dt.asSeconds(), playerPosition);
            }
            for (int i = 0; i < 100; ++i) {
                if (bullets[i].isInFlight())
                    bullets[i].update(dtAsSeconds);
            }
            healthPickup.update(dtAsSeconds);
            ammoPickup.update(dtAsSeconds);
            for (int i = 0; i < 100; ++i) {
                for (int j = 0; j < numZombies; ++j) {
                    if (bullets[i].isInFlight() && zombies[j].isAlive()) {
                        if (bullets[i].getPosition().intersects(zombies[j].getPosition())) {
                            bullets[i].stop();
                            if (zombies[j].hit()) {
                                score += 10;
                                if (score > hiscore)
                                    hiscore = score;
                                --numZombiesAlive;
                                if (numZombiesAlive == 0)
                                    state = State::LEVELING_UP;
                            }
                            splat.play();
                        }
                    }
                }
            }
            for (int i = 0; i < numZombies; ++i) {
                if (player.getPosition().intersects(zombies[i].getPosition())
                        && zombies[i].isAlive()) {
                    if (player.hit(gameTimeTotal)) {
                        // scriu mai tarziu
                        hit.play();
                    }
                    if (player.getHealth() <= 0) {
                        state = State::GAME_OVER;
                        std::ofstream outputFile("gamedata/scores.txt");
                        outputFile << hiscore;
                        outputFile.close();
                    }
                }
            }
            if (player.getPosition().intersects(healthPickup.getPosition()) && healthPickup.isSpawned()) {
                player.increaseHealthLevel(healthPickup.gotIt());
                pickup.play();
            }
            if (player.getPosition().intersects(ammoPickup.getPosition()) && ammoPickup.isSpawned()) {
                bulletsSpare += ammoPickup.gotIt();
                pickup.play();//reload sau pickup???
            }
            healthBar.setSize(Vector2f(player.getHealth() * 3, 50));
            framesSinceLastHUDUpdate++;
            if (framesSinceLastHUDUpdate > fpsMeasurementFrameInterval) {
                std::stringstream ssAmmo;
                std::stringstream ssScore;
                std::stringstream ssHiScore;
                std::stringstream ssWave;
                std::stringstream ssZombiesAlive;
                ssAmmo << bulletsInClip << "/" << bulletsSpare;
                ammoText.setString(ssAmmo.str());
                ssScore << "Score:" << score;
                scoreText.setString(ssScore.str());
                ssHiScore << "Hi Score:" << hiscore;
                hiScoreText.setString(ssHiScore.str());
                ssWave << "Wave:" << wave;
                waveNumberText.setString(ssWave.str());
                ssZombiesAlive << "Zombies:" << numZombiesAlive;
                zombiesRemText.setString(ssZombiesAlive.str());
                framesSinceLastHUDUpdate = 0;
            }
        }
        /*********************
         *******REFRESH*******
         *********************/
        if (state == State::PLAYING) {
            window.clear();
            window.setView(mainView);
            window.draw(background, &textureBackground);
            for (int i = 0; i < numZombies; ++i)
                window.draw(zombies[i].getSprite());
            for (int i = 0; i < 100; ++i) {
                if (bullets[i].isInFlight())
                    window.draw(bullets[i].getShape());
            }
            window.draw(player.getSprite());
            if (ammoPickup.isSpawned())
                window.draw(ammoPickup.getSprite());
            if (healthPickup.isSpawned())
                window.draw(healthPickup.getSprite());
            window.draw(spriteCrosshair);
            window.setView(hudView);
            window.draw(spriteAmmoIcon);
            window.draw(ammoText);
            window.draw(scoreText);
            window.draw(hiScoreText);
            window.draw(healthBar);
            window.draw(waveNumberText);
            window.draw(zombiesRemText);

        }
        if (state == State::LEVELING_UP) {
            window.draw(spriteGameOver);
            window.draw(levelUpText);
        }
        if (state == State::PAUSED) {
            window.draw(pausedText);
        }
        if (state == State::GAME_OVER) {
            window.draw(spriteGameOver);
            window.draw(gameOverText);
            window.draw(scoreText);
            window.draw(hiScoreText);
        }
        window.display();
    }
    delete[] zombies;
    return 0;
}
