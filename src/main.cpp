#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <set>
#include <queue>
using namespace std;
using namespace sf;

ifstream s1in("saves/save_1");
ofstream s1out("saves/save_1", ios::app);
ifstream rin("saves/records");
ofstream rout("saves/records");

Vector2i windsize;
Vector2i lastClick;
int numMines;
vector<vector<int>> display; //stores the layout of displayed things (1-8, 0 - tile, -1 is nothing)
vector<vector<bool>> flags;  //stores the layout of flags
vector<vector<int>> layout;  //stores the layout of hidden things (1-8, 0 - tile, 9 - mine, -1 is nothing;

int curTime = 0;
int addTime = 0;

/* To Do:

1. Add ending screen - Done?
2. Add time - Done
3. Add saved games and leaderboards and stuff

*/

Texture tileTextures[10];
Sprite tileSprites[10];
Texture flagTexture;
Sprite flagSprite;
Texture errorTexture;
Sprite errorSprite;
Texture saveTexture;
Sprite saveSprite;
Font pixeboy;

bool isLost = false;

bool firstClick = true;

bool placedFlag = false;

bool firstWin = true;

bool loadedSave = false;

bool isWon()
{
    for (int i = 0; i < windsize.y; i++)
    {
        for (int j = 0; j < windsize.x; j++)
        {
            if (display[i][j] == 0 && layout[i][j] != 9)
                return false;
        }
    }
    return true;
}

void input()
{
    char saveRes;
    cout << "Would you like to load a save? (Y / N)" << endl;
    cin >> saveRes;
    saveRes = tolower(saveRes);
    while (saveRes != 'y' && saveRes != 'n')
    {
        cout << "Not a valid response." << endl;
        cin >> saveRes;
    }

    if (saveRes == 'y')
    {
        if (s1in.peek() == ifstream::traits_type::eof())
        {
            cout << "No save detected. Continuing with new game." << endl;
            loadedSave = false;
        }
        else
        {
            loadedSave = true;
        }
    }
    else if (saveRes == 'n')
    {
        loadedSave = false;
    }

    if (!loadedSave)
    {
        cout << "Please enter the map size. (Preferably below 60)" << endl;
        cout << "Height: ";
        cin >> windsize.y;
        cout << "Width: ";
        cin >> windsize.x;
        display.assign(windsize.y, vector<int>());
        layout.assign(windsize.y, vector<int>());
        flags.assign(windsize.y, vector<bool>());
        cout << "Please enter the number of mines." << endl;
        cin >> numMines;
        while (numMines >= 0 && numMines >= (windsize.x * windsize.y))
        {
            cout << "Please enter a number of mines less than the number of available squares" << endl;
            cin >> numMines;
        }
    }
}

void setup()
{
    //Setup textures
    pixeboy.loadFromFile("assets/pixeboy.ttf");
    for (int i = 1; i < 9; i++)
    {
        string path = "assets/" + to_string(i) + ".png";
        tileTextures[i].loadFromFile(path);
        tileSprites[i].setTexture(tileTextures[i]);
    }
    tileTextures[0].loadFromFile("assets/tile.png");
    tileSprites[0].setTexture(tileTextures[0]);
    tileTextures[9].loadFromFile("assets/mine.png");
    tileSprites[9].setTexture(tileTextures[9]);
    flagTexture.loadFromFile("assets/flag.png");
    flagSprite.setTexture(flagTexture);
    errorTexture.loadFromFile("assets/error.png");
    errorSprite.setTexture(errorTexture);

    saveTexture.loadFromFile("assets/save.png");
    saveSprite.setTexture(saveTexture);

    //My brain is fried from calc test

    //Set up display layout
    if (!loadedSave)
    {
        for (int i = 0; i < windsize.y; i++)
        {
            for (int j = 0; j < windsize.x; j++)
            {
                display[i].push_back(0);
            }
        }

        for (int i = 0; i < windsize.y; i++)
        {
            for (int j = 0; j < windsize.x; j++)
            {
                layout[i].push_back(-1);
            }
        }

        //Set up flags
        for (int i = 0; i < windsize.y; i++)
        {
            for (int j = 0; j < windsize.x; j++)
            {
                flags[i].push_back(false);
            }
        }
    }
    else
    {
        //import from save file
        s1in >> windsize.x >> windsize.y;
        s1in >> numMines;
        s1in >> firstClick;
        s1in >> addTime;

        display.assign(windsize.y, vector<int>());
        layout.assign(windsize.y, vector<int>());
        flags.assign(windsize.y, vector<bool>());

        for (int i = 0; i < windsize.y; i++)
        {
            for (int j = 0; j < windsize.x; j++)
            {
                int temp;
                s1in >> temp;
                layout[i].push_back(temp);
            }
        }

        for (int i = 0; i < windsize.y; i++)
        {
            for (int j = 0; j < windsize.x; j++)
            {
                int temp;
                s1in >> temp;
                display[i].push_back(temp);
            }
        }

        //Set up flags
        for (int i = 0; i < windsize.y; i++)
        {
            for (int j = 0; j < windsize.x; j++)
            {
                bool temp;
                s1in >> temp;
                flags[i].push_back(temp);
            }
        }
    }
}

int main()
{
    srand(time(NULL));

    input();
    setup();

    const clock_t begin_time = clock();

    RenderWindow window(VideoMode(windsize.x * 16, windsize.y * 16 + 32), "Minesweeper");

    while (window.isOpen())
    {
        Event evnt;
        while (window.pollEvent(evnt))
        {
            if (evnt.type == Event::Closed)
                window.close();
        }

        //Get click (remember if first click you need to do stuff for making sure game doesnt end immediately)
        if (Mouse::isButtonPressed(Mouse::Left) && !isLost && !isWon())
        {
            Vector2i position = Mouse::getPosition(window);
            position.x--;
            position.y--;

            //Check for save button
            if (position.x <= windsize.x * 8 + 16 && position.x >= windsize.x * 8 - 16 && position.y <= windsize.y * 16 + 24 && position.y >= windsize.y * 16 + 8)
            {
                ofstream ofs;
                ofs.open("saves/save_1", ofstream::out | ofstream::trunc);
                ofs.close();
                cout << "Game saved." << endl;
                placedFlag = true; //for a delay, but that's not really working

                s1out << windsize.x << " " << windsize.y << endl;
                s1out << numMines << endl;
                s1out << firstClick << endl; //tells whether or not the map was already generated.
                s1out << (curTime + addTime) << endl;

                for (int i = 0; i < layout.size(); i++)
                {
                    for (int j = 0; j < layout[i].size(); j++)
                    {
                        s1out << layout[i][j] << " ";
                    }
                    s1out << endl;
                }

                for (int i = 0; i < display.size(); i++)
                {
                    for (int j = 0; j < display[i].size(); j++)
                    {
                        s1out << display[i][j] << " ";
                    }
                    s1out << endl;
                }

                for (int i = 0; i < flags.size(); i++)
                {
                    for (int j = 0; j < flags[i].size(); j++)
                    {
                        s1out << flags[i][j] << " ";
                    }
                    s1out << endl;
                }
            }

            if (position.x >= windsize.x * 16 || position.y >= windsize.y * 16 || position.x < 0 || position.y < 0)
                continue;
            if (!flags[position.y / 16][position.x / 16])
            {
                if (firstClick)
                {
                    //Create the board
                    firstClick = false;
                    //Generate random mines
                    set<pair<int, int>> mines;
                    mines.insert(make_pair(position.x / 16, position.y / 16)); //just so no mine goes onto the square initially clicked
                    while (mines.size() <= numMines)
                    {
                        //prob not most efficient but idc ngl
                        pair<int, int> mcoords;
                        mcoords.first = rand() % windsize.x;
                        mcoords.second = rand() % windsize.y;
                        mines.insert(mcoords);
                    }

                    //why do i store mines as x, y but store layout as y, x

                    for (auto it = mines.begin(); it != mines.end(); it++)
                    {
                        layout[(*it).second][(*it).first] = 9;
                    }
                    layout[position.y / 16][position.x / 16] = -1; //has to be an empty square
                    //determine the rest of the board from the mines

                    // cout << "got here" << endl;

                    int dx[8] = {0, 1, -1, 0, 1, -1, 1, -1};
                    int dy[8] = {1, 0, 0, -1, 1, 1, -1, -1};
                    for (int i = 0; i < windsize.y; i++)
                    {
                        for (int j = 0; j < windsize.x; j++)
                        {
                            if (layout[i][j] == 9)
                                continue;
                            int neighborMines = 0;
                            for (int k = 0; k < 8; k++)
                            {
                                int cx = dx[k] + j;
                                int cy = dy[k] + i;
                                if (cx < 0 || cx >= windsize.x || cy < 0 || cy >= windsize.y)
                                    continue;
                                if (layout[cy][cx] == 9)
                                    neighborMines++;
                            }
                            layout[i][j] = neighborMines;
                            if (neighborMines == 0)
                                layout[i][j] = -1;
                        }
                    }
                    //surprised this works honestly; knowing me I would somehow blow up my computer
                }
                //determine how the squares work out from the click, can end the game
                //need to floodfill to open up all squares = -1, also adjacent squares
                queue<pair<int, int>> q;
                q.push(make_pair(position.y / 16, position.x / 16));
                display[position.y / 16][position.x / 16] = layout[position.y / 16][position.x / 16];
                if (layout[position.y / 16][position.x / 16] == 9)
                {
                    lastClick.x = position.x / 16;
                    lastClick.y = position.y / 16;
                    isLost = true;
                    cout << "You lost" << endl;
                }
                //visited array will just be revealed squares (!0) on the display

                int dx[8] = {1, 0, 0, -1, 1, -1, 1, -1};
                int dy[8] = {0, 1, -1, 0, -1, 1, 1, -1};

                while (!q.empty())
                {
                    int cy = q.front().first;
                    int cx = q.front().second;
                    q.pop();
                    if (layout[cy][cx] != -1)
                        continue;
                    for (int k = 0; k < 8; k++)
                    {
                        int ny = cy + dy[k];
                        int nx = cx + dx[k];
                        if (nx < 0 || nx >= windsize.x || ny < 0 || ny >= windsize.y)
                            continue;
                        if (display[ny][nx] == 0)
                        {
                            flags[ny][nx] = 0;
                            display[ny][nx] = layout[ny][nx];
                            q.push(make_pair(ny, nx));
                        }
                    }
                }
            }
        }
        if (Mouse::isButtonPressed(Mouse::Right) && !isLost && !isWon())
        {
            //create flag
            Vector2i position = Mouse::getPosition(window);
            //just to remove corner stuff idk
            position.x--;
            position.y--;
            if (position.x >= windsize.x * 16 || position.y >= windsize.y * 16 || position.x < 0 || position.y < 0)
                continue;
            //cout << position.y / 16 << " " << position.x / 16 << endl;
            if (display[position.y / 16][position.x / 16] == 0)
            {
                flags[position.y / 16][position.x / 16] = !flags[position.y / 16][position.x / 16];

                placedFlag = true;
            }
        }

        if (isWon() && firstWin)
        {
            firstWin = false;
            cout << "Congratulations, you won!" << endl;
            cout << "Your time was: " << curTime + addTime << " seconds!" << endl;
        }

        if (!isLost)
        {
            window.clear();
            //Display display board (when game is still running)
            for (int i = 0; i < windsize.y; i++)
            {
                for (int j = 0; j < windsize.x; j++)
                {
                    // cout << display[i][j] << " ";
                    if (display[i][j] == -1)
                    {
                        //draw white triangle?? rectangle

                        Texture blankTexture;
                        Sprite blankSprite;
                        blankTexture.loadFromFile("assets/empty.png");
                        blankSprite.setTexture(blankTexture);
                        blankSprite.setPosition(j * 16, i * 16);
                        window.draw(blankSprite);
                        continue;
                    }
                    Sprite curSprite = tileSprites[display[i][j]];
                    curSprite.setPosition(j * 16, i * 16);
                    window.draw(curSprite);
                    if (flags[i][j])
                    {
                        curSprite = flagSprite;
                        curSprite.setPosition(j * 16, i * 16);
                        window.draw(curSprite);
                    }
                }
                // cout << endl;
            }

            //Draw the time and save button
            RectangleShape background;
            background.setFillColor(Color::White);
            background.setSize(Vector2f(windsize.x * 16, 32));
            background.setPosition(Vector2f(0, windsize.y * 16));
            window.draw(background);

            saveSprite.setPosition(windsize.x * 8 - 16, windsize.y * 16 + 8);
            window.draw(saveSprite);

            if (!isWon() && !isLost)
            {
                curTime = (int)(clock() - begin_time) / CLOCKS_PER_SEC;
            }

            Text timeText;
            timeText.setFont(pixeboy);
            timeText.setFillColor(Color::Black);
            timeText.setString(to_string(curTime + addTime));
            timeText.setPosition(Vector2f(8, windsize.y * 16 - 8 + (32 - timeText.getCharacterSize()) / 2));
            window.draw(timeText);
        }

        if (isLost)
        {
            //want to add x's over place clicked with bomb
            //want to add x's over flags that are incorrect
            //want to show bombs that were not flagged
            window.clear();
            Sprite curSprite;

            for (int i = 0; i < windsize.y; i++)
            {
                for (int j = 0; j < windsize.x; j++)
                {
                    if (layout[i][j] == 9 && !flags[i][j])
                    {
                        curSprite = tileSprites[9];
                        curSprite.setPosition(j * 16, i * 16);
                        window.draw(curSprite);
                    }
                    else if (layout[i][j] != 9 && flags[i][j])
                    {
                        curSprite = errorSprite;
                        curSprite.setPosition(j * 16, i * 16);
                        window.draw(curSprite);
                    }
                    else
                    {
                        if (display[i][j] == -1)
                        {
                            Texture blankTexture;
                            Sprite blankSprite;
                            blankTexture.loadFromFile("assets/empty.png");
                            blankSprite.setTexture(blankTexture);
                            blankSprite.setPosition(j * 16, i * 16);
                            window.draw(blankSprite);
                        }
                        else
                        {
                            curSprite = tileSprites[display[i][j]];
                            curSprite.setPosition(j * 16, i * 16);
                            window.draw(curSprite);
                        }
                    }
                    if (flags[i][j])
                    {
                        curSprite = flagSprite;
                        curSprite.setPosition(j * 16, i * 16);
                        window.draw(curSprite);
                        if (layout[i][j] != 9)
                        {
                            curSprite = errorSprite;
                            curSprite.setPosition(j * 16, i * 16);
                            window.draw(curSprite);
                        }
                    }
                }
            }

            curSprite = tileSprites[9];
            curSprite.setPosition(Vector2f(lastClick.x * 16, lastClick.y * 16));
            window.draw(curSprite);

            curSprite = errorSprite;
            curSprite.setPosition(Vector2f(lastClick.x * 16, lastClick.y * 16));
            window.draw(curSprite);

            RectangleShape background;
            background.setFillColor(Color::White);
            background.setSize(Vector2f(windsize.x * 16, 32));
            background.setPosition(Vector2f(0, windsize.y * 16));
            window.draw(background);

            saveSprite.setPosition(windsize.x * 8 - 16, windsize.y * 16 + 8);
            window.draw(saveSprite);

            Text timeText;
            timeText.setFont(pixeboy);
            timeText.setFillColor(Color::Black);
            timeText.setString(to_string(curTime));
            timeText.setPosition(Vector2f(8, windsize.y * 16 - 8 + (32 - timeText.getCharacterSize()) / 2));
            window.draw(timeText);
        }

        window.display();

        if (placedFlag)
        {
            sleep(milliseconds(400));
            placedFlag = false;
        }
    }
    return 0;
}