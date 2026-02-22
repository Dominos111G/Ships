/******************************************************************************
 * Tytuł       : Statki
 * Autor       : Dominik Szczerbal
 * Klasa       : 3 TIP
 * Data        : 2025-10-25
 * Wersja      : 1.0
 * Opis        : Gra w statki oline dla dwóch graczy z wykorzystaniem SFML
 * Zależności  : C++98 lub nowszy
 * Kompilacja  : g++ main.cpp -o main -lsfml-graphics -lsfml-window 
 *             : -lsfml-system
 * Uruchomienie: ./main
 * Kontakt     : d.szczerbal@zset.leszno.pl
 ******************************************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Network.hpp>

using namespace std;

// USTAWIENIA
bool UdebugMode = false;
bool UshowBoard = false;
bool UshowEnemyBoard = false;
int maxShipsAmount = 20;
vector<string> alphabet = { "A","B","C","D","E","F","G","H","I","J" };

// ZMIENNE GRY
bool ready = false;
bool readyEnemy = false;
bool started = false;
bool turn = false;

class player {
private:
    int placed = 0;
	int shipsAmount = maxShipsAmount;
    int maxShips = maxShipsAmount;
    vector<vector<int>> table = vector<vector<int>>(10, vector<int>(10, 0));
    vector<vector<int>> shot = vector<vector<int>>(10, vector<int>(10, 0));

    bool setShip(int row, int col) {
		// Usuń statek
        if (this->table[row][col] == 1) {
            this->placed = this->placed - 1;
            this->table[row][col] = 0;
            return true;
        }

		// Dodaj statek
        if (this->placed < this->maxShips) {
            this->placed = this->placed + 1;
            this->table[row][col] = 1;
            return true;
        }

        return false;
    }

    void checkShip(int row, int col) {
        vector<pair<int, int>> shipCells;
        shipCells.push_back(make_pair(row, col));

        // Sprawdź poziomo
        int c = col - 1;
        while (c >= 0 && (this->table[row][c] == 1 || this->table[row][c] == 2 || this->table[row][c] == 3)) {
            shipCells.push_back(make_pair(row, c));
            c--;
        }
        c = col + 1;
        while (c < 10 && (this->table[row][c] == 1 || this->table[row][c] == 2 || this->table[row][c] == 3)) {
            shipCells.push_back(make_pair(row, c));
            c++;
        }

        // Sprawdź pionowo
        if (shipCells.size() == 1) {
            int r = row - 1;
            while (r >= 0 && (this->table[r][col] == 1 || this->table[r][col] == 2 || this->table[r][col] == 3)) {
                shipCells.push_back(make_pair(r, col));
                r--;
            }
            r = row + 1;
            while (r < 10 && (this->table[r][col] == 1 || this->table[r][col] == 2 || this->table[r][col] == 3)) {
                shipCells.push_back(make_pair(r, col));
                r++;
            }
        }

        // Jeżeli 1 nie jest zatopiony
        for (size_t i = 0; i < shipCells.size(); ++i) {
            int r = shipCells[i].first;
            int c2 = shipCells[i].second;
            if (this->table[r][c2] == 1) {
                return;
            }
        }

        // 2 lub 3 zatopiony
        for (size_t i = 0; i < shipCells.size(); ++i) {
            int r = shipCells[i].first;
            int c2 = shipCells[i].second;
            this->table[r][c2] = 3;
        }
    }
public:
    bool handlePlayerShot(int row, int col) {
		// Sprawdz czy juz strzelano
        if (this->shot[row][col] == 1) {
            if (UdebugMode) cout << "Już strzelałeś w to pole!" << "\n";
            return false;
        }

		// Sprawdz czy trafiono
        if (this->table[row][col] == 1) {
            if (UdebugMode) cout << "TRAFIONY!" << "\n";
            this->shot[row][col] = 1;
            this->table[row][col] = 2;
            this->shipsAmount -= 1;
            checkShip(row, col);
        } else {
            if (UdebugMode) cout << "PUDŁO!" << "\n";
            this->shot[row][col] = 1;
            this->table[row][col] = 4;
        }

        return true;
    }

	// Pobierz wartość pola
    int getValue(int row, int col) {
        return this->table[row][col];
    }

    bool placeShip(int row, int col) {
        for (int r = -1; r <= 1; r+=2) {
            for (int c = -1; c <= 1; c+=2) {
				// Sprawdź czy nie wychodzi poza
                if (row + r > 9 || row + r < 0) {
                    continue;
                } else if (col + c > 9 || col + c < 0) {
                    continue;
                }

				// Sprawdź zajęcie sąsiedniego pola
				bool result = getValue(row + r, col + c);

                if (UdebugMode) cout << "Checking: " << (row + r) << ", " << (col + c) << " = " << result << "\n";

                if (result == 1) {
					return false;
                }
            }
        }

		return setShip(row, col);
	}

    // Ilość statków
    int getShipsAmount() {
        return this->placed;
    }

	// Maksymalna ilość statków
    int getMaxShipsAmount() {
        return this->maxShips;
    }

    int getShipsLeft() {
        return this->shipsAmount;
	}
};

/*
 * 0 - puste
 * 1 - statek
 * 2 - trafiony statek
 * 3 - zatopiony statek
 * 4 - pudlo
 */

sf::Color getOutlineColor(int val) {
    if (val == 4) return sf::Color::White;
    if (val == 3) return sf::Color::Blue;
    if (val == 2) return sf::Color::Blue;
    if (val == 1) return sf::Color::Blue;
    return sf::Color::White;
}

sf::Color getFillColor(int val) {
    if (val == 4) return sf::Color(255, 0, 0, 120);
    if (val == 3) return sf::Color(255, 0, 0);
    if (val == 2) return sf::Color(255, 162, 0, 220);
    if (val == 1) return sf::Color::Transparent;
    return sf::Color::Transparent; 
}

int cell = 34;
// x = i * 34 + 50 = 84 -- y = i * 34 + 100 = 134
const int x0p = 50 + 34;
const int y0p = 100 + 34;
const int x0e = x0p + 440;
const int y0e = y0p;

// Sprawdza czy mysz jest nad planszą przeciwnika
bool isMouseOverEnemyBoard(int mouseX, int mouseY) {
    return (mouseX >= x0e && mouseX < x0e + 10 * cell &&
        mouseY >= y0e && mouseY < y0e + 10 * cell);
}

// Pobiera pozycję na planszy przeciwnika
pair<int, int> getEnemyBoardPosition(int mouseX, int mouseY) {
    int row = (mouseY - y0e) / cell; 
    int col = (mouseX - x0e) / cell;

    if (row < 0) row = 0;
    if (row > 9) row = 9;
    if (col < 0) col = 0;
    if (col > 9) col = 9;

    return make_pair(row, col);
}

// Sprawdza czy mysz jest nad planszą lokalną
bool isMouseOverLocalBoard(int mouseX, int mouseY) {
    return (mouseX >= x0p && mouseX < x0p + 10 * cell &&
        mouseY >= y0p && mouseY < y0p + 10 * cell);
}

// Pobiera pozycję na planszy lokalnej
pair<int, int> getLocalBoardPosition(int mouseX, int mouseY) {
    int row = (mouseY - y0p) / cell;
    int col = (mouseX - x0p) / cell;

    if (row < 0) row = 0;
    if (row > 9) row = 9;
    if (col < 0) col = 0;
    if (col > 9) col = 9;

    return make_pair(row, col);
}

// Środek tekstu w kwadracie
void centerTextInSquare(sf::Text& text, const sf::FloatRect& squareBounds) {
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setPosition(
        squareBounds.left + (squareBounds.width / 2.f) - (textBounds.width / 2.f),
        squareBounds.top + (squareBounds.height / 2.f) - (textBounds.height / 2.f)
    );
}

// Tworzenie planszy
void createBoards(sf::RenderWindow& window, sf::Text text, player localTab, player enemyTab) {
    text.setPosition(120.f, 25.f);
    text.setString("Twoja plansza");
    text.setFillColor(sf::Color::White);
    text.setCharacterSize(34);
    window.draw(text);

    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            if (i == 0) {
                if (j == 0) continue;

                text.setString(alphabet[j - 1]);
                text.setCharacterSize(19);

                sf::FloatRect letterSquare(j * 34.f + 50.f, 70.f + 16.f, 30.f, 30.f);
                centerTextInSquare(text, letterSquare);
                window.draw(text);

            } else if (j == 0) {
                text.setString(to_string(i));
                text.setCharacterSize(19);

                sf::FloatRect numberSquare(30.f + 16.f, i * 34.f + 100.f, 30.f, 30.f);
                centerTextInSquare(text, numberSquare);
                window.draw(text);

            } else {
                int cellCol = i - 1;
                int cellRow = j - 1;

                sf::RectangleShape square({ 30.f, 30.f });
                square.setPosition(i * 34.f + 50.f, j * 34.f + 100.f);
                square.setOutlineThickness(2.f);
                square.setOutlineColor(getOutlineColor(localTab.getValue(cellRow, cellCol)));
                square.setFillColor(getFillColor(localTab.getValue(cellRow, cellCol)));

                window.draw(square);
            }
        }
    }

    text.setPosition(510.f, 25.f);
    text.setString("Plansza Przeciwnika");
    text.setCharacterSize(34);
    text.setFillColor(sf::Color::White);
    window.draw(text);

    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            if (i == 0) {
                if (j == 0) continue;

                text.setString(alphabet[j - 1]);
                text.setCharacterSize(19);

                sf::FloatRect letterSquare(j * 34.f + 50.f + 440.f, 70.f + 16.f, 30.f, 30.f);
                centerTextInSquare(text, letterSquare);
                window.draw(text);

            } else if (j == 0) {
                text.setString(to_string(i));
                text.setCharacterSize(19);

                sf::FloatRect numberSquare(30.f + 440.f + 16.f, i * 34.f + 100.f, 30.f, 30.f);
                centerTextInSquare(text, numberSquare);
                window.draw(text);

            } else {
                int cellCol = i - 1;
                int cellRow = j - 1;

                sf::RectangleShape square({ 30.f, 30.f });
                square.setPosition(i * 34.f + 50.f + 440.f, j * 34.f + 100.f);
                square.setOutlineThickness(2.f);

                if (enemyTab.getValue(cellRow, cellCol) != 1) {
                    square.setOutlineColor(getOutlineColor(enemyTab.getValue(cellRow, cellCol)));
                    square.setFillColor(getFillColor(enemyTab.getValue(cellRow, cellCol)));
                }
                else {
                    square.setOutlineColor(sf::Color::White);
                    square.setFillColor(sf::Color::Transparent);
                }
                window.draw(square);
            }
        }
    }
}

void createBottomInfo(sf::RenderWindow& window, sf::Text text, player localTab, player enemyTab) {
    if (!started) {
        if (!ready) {
            text.setPosition(50.f, 500.f);
            text.setString("Kliknij na plansze aby ustawic statek.");
            text.setFillColor(sf::Color::White);
            text.setCharacterSize(19);
            window.draw(text);
        }
    } else {
        text.setPosition(490.f, 500.f);
        text.setString("Pozostalo " + to_string(enemyTab.getShipsLeft()) + " wrogich jednostek.");
        text.setCharacterSize(19);
        text.setFillColor(sf::Color::White);
        window.draw(text);

        if (turn) {
            text.setPosition(490.f, 530.f);
            text.setString("Kliknij na plansze przeciwnika aby strzelic.");
            text.setCharacterSize(19);
            text.setFillColor(sf::Color::White);
            window.draw(text);
        }

    }

    if (!started) {
        if (localTab.getShipsAmount() == localTab.getMaxShipsAmount()) {
            // Status lokoalny
            text.setPosition(490.f, 500.f);
            text.setCharacterSize(19);
            if (ready) {
                text.setString("[G] - Status: Gotowy");
                text.setFillColor(sf::Color::Green);
            } else {
                text.setString("[G] - Status: Niegotowy");
                text.setFillColor(sf::Color::Red);
            }
            window.draw(text);

            // Status przeciwnika
            text.setPosition(490.f, 530.f);
            text.setCharacterSize(19);
            if (readyEnemy) {
                text.setString("Status przeciwnika: Gotowy");
                text.setFillColor(sf::Color::Green);
            } else {
                text.setString("Status przeciwnika: Niegotowy");
                text.setFillColor(sf::Color::Red);
            }
            window.draw(text);
        } else {
            text.setPosition(490.f, 500.f);
            text.setString("Postaw jeszcze " + to_string(localTab.getMaxShipsAmount() - localTab.getShipsAmount()) + " jednostek.");
            text.setFillColor(sf::Color::Red);
            text.setCharacterSize(20);
            window.draw(text);
        }
    } else {
        text.setPosition(50.f, 500.f);
        text.setCharacterSize(19);
        if (turn) {
            text.setString(L"Gra rozpoczęta!\nTwój ruch");
            text.setFillColor(sf::Color::Yellow);
        } else {
            text.setString(L"Gra rozpoczęta!\nOczekiwanie na ruch przeciwnika");
            text.setFillColor(sf::Color::White);
        }
        window.draw(text);
    }
}

int main() {
    srand(time(0));

    sf::Font font;
    if (!font.loadFromFile("Bitter.ttf"))
    {
        cout << "Font error!";
        return -1;
    }

    // Plansze
    player localTab;
    player enemyTab;

	// ŁĄCZENIE SIECIOWE
    bool isHost = false;
    sf::TcpListener listener;
    sf::TcpSocket socket;
    socket.setBlocking(false);
    sf::IpAddress otherIp;

    while (true) {
        cout << "Host (h) czy Join (j)? ";

        string input; cin >> input;
	    char opt = tolower(input[0]);

        if (opt == 'h') {
            isHost = true;

            cout << "Oczekiwanie na gracza... (Port 53000)" << "\n";
            if (listener.listen(53000) != sf::Socket::Done) {
                cout << "Nie mozna nasłuchiwać na porcie 53000!" << "\n";
                return -1;
            }

            listener.setBlocking(false);
            break;

        } else if (opt == 'z') {
            while (opt == 'z') {
                cout << "\nUstawienia"
                     << "\n1.debugMode = " << UdebugMode 
                     << "\n2. showBoard = " << UshowBoard 
                     << "\n3. showEnemyBoard = " << UshowEnemyBoard 
                     << "\n4. Wyjście"
                     << "\nWyb: ";
				char Uopt; cin >> Uopt;
                switch (Uopt) {
                case '1':
                    UdebugMode = !UdebugMode;
                    continue;
                case '2':
                    UshowBoard = !UshowBoard;
                    continue;
                case '3':
                    UshowEnemyBoard = !UshowEnemyBoard;
                    continue;
                case '4':
                    opt = ' ';
                    cout << "\n";
                }
            }

        } else if (opt == 'j') {
            cout << "Podaj IP hosta: ";
            string ip; cin >> ip;
            otherIp = ip;
            cout << "Probuje polaczyc..." << "\n";

            if (socket.connect(otherIp, 53000, sf::seconds(10)) != sf::Socket::Done) {
                cout << "Ponawiam łączenie..." << "\n"; // nadal kontynuujemy, uzytkownik moze sprobowac pozniej

            } else {
                cout << "Polaczono z hostem: " << otherIp << "\n";
                socket.setBlocking(false);
            }

            break;

        } else {
			cout << "Nieprawidłowa opcja!" << "\n\n";
            continue;
        }
    }

    if (isHost) {
        bool connected = false;

        while (true) {
            if (socket.getRemoteAddress() == sf::IpAddress::None) {
                sf::Socket::Status status = listener.accept(socket);
                if (status == sf::Socket::Done) {
                    cout << "Klient polaczony: " << socket.getRemoteAddress() << "\n";
                    socket.setBlocking(false);
                    connected = true;
                    break;
                }
            }
        }

        if (!connected) {
            cout << "Upłynął limit czasu oczekiwania!" << "\n";
            return 0;
        }

        turn = rand() % 2 == 0; // Losowanie kto zaczyna
        if (!turn) {
            if (UdebugMode) cout << "Przeciwnik zaczyna!" << "\n";
            sf::Packet out; out << string("TURN");
            socket.send(out);
        } else {
            if (UdebugMode) cout << "Ty zaczynasz!" << "\n";
        }

        sf::RenderWindow window(sf::VideoMode(950, 600), "Ships (network host)");

        while (window.isOpen()) {
            // odbiór pakietów (non-blocking)
            if (socket.getRemoteAddress() != sf::IpAddress::None || (!isHost && socket.getRemoteAddress() != sf::IpAddress::None)) {
                sf::Packet packet;
                sf::Socket::Status recvStatus = socket.receive(packet);
                if (recvStatus == sf::Socket::Done) {
                    string cmd; packet >> cmd;
                    if (cmd == "READY") {
                        readyEnemy = true;

                        if (ready && readyEnemy) {
                            started = true;
                            if (UdebugMode) cout << "Started!" << "\n";

                            sf::Packet out; out << string("START") << true;
                            socket.send(out);
                        } else {
                            sf::Packet out; out << string("RESULT") << true;
                            socket.send(out);
                        }

                    } else if (cmd == "NOTREADY") {
                        readyEnemy = false;

                        sf::Packet out; out << string("RESULT") << true;
                        socket.send(out);

                    } else if (cmd == "START") {
                        readyEnemy = true;
                        ready = true;
                        started = true;

                    } else if (cmd == "PLACE") {
                        int col, row; packet >> col >> row;
                        if (UdebugMode) cout << "Otrzymano: " << col << "," << row << "\n";

                        enemyTab.placeShip(row, col);

                        sf::Packet out; out << string("RESULT") << true;
                        socket.send(out);

                    } else if (cmd == "SHOT") {
                        int col, row; packet >> col >> row;
                        turn = !turn;
                        if (UdebugMode) cout << "Otrzymano strzał: " << col << "," << row << "\n";

                        localTab.handlePlayerShot(row, col);

                        sf::Packet out; out << string("RESULT") << true;
                        socket.send(out);

                    } else if (cmd == "TURN") {
                        if (UdebugMode) cout << "OTRZYMANO DANE RUCHU";
                        turn = true;

                    } else if (cmd == "DISCONECTED") {
                        cout << "Player disconected!" << "\n";
                        return 0;

                    } else if (cmd == "RESULT" && UdebugMode) {
						cout << "Otrzymano potwierdzenie!" << "\n";

                    } else {
                        if (UdebugMode) cout << "cmdE: " << cmd;
                    }
                }
            }

            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    sf::Packet out; out << string("DISCONECTED");
                    socket.send(out);

                    window.close();
                }

                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        int mouseX = event.mouseButton.x;
                        int mouseY = event.mouseButton.y;

                        if (isMouseOverEnemyBoard(mouseX, mouseY)) {
                            pair<int, int> pos = getEnemyBoardPosition(mouseX, mouseY);
                            int row = pos.first;
                            int col = pos.second;

                            if (row >= 0 && row < 10 && col >= 0 && col < 10) {
                                if (started && turn) {
                                    bool result = enemyTab.handlePlayerShot(row, col);

                                    if (result) {
                                        turn = !turn;
                                        if (UdebugMode) cout << "SHOT SENDED!!!" << "\n";

                                        sf::Packet out; out << string("SHOT") << col << row;
                                        if (socket.getRemoteAddress() != sf::IpAddress::None) socket.send(out);
                                    }
                                }
                            }
                        }

                        if (isMouseOverLocalBoard(mouseX, mouseY)) {
                            pair<int, int> pos = getLocalBoardPosition(mouseX, mouseY);
                            int row = pos.first;
                            int col = pos.second;

                            if (row >= 0 && row < 10 && col >= 0 && col < 10) {
                                if (!started && !ready) {
                                    localTab.placeShip(row, col);
                                    sf::Packet out; out << string("PLACE") << col << row;
                                    if (socket.getRemoteAddress() != sf::IpAddress::None) socket.send(out);
                                }
                            }
                        }
                    }
                }

                if (event.type == sf::Event::KeyPressed) {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) {
                        if (ready == false) {
                            if (localTab.getShipsAmount() == localTab.getMaxShipsAmount()) {
                                ready = true;
                                if (UdebugMode) cout << "Gotowy!" << "\n";

                                sf::Packet out; out << string("READY") << true;
                                socket.send(out);

                                if (ready && readyEnemy) {
                                    started = true;
                                    if (UdebugMode) cout << "Start!" << "\n";

                                    sf::Packet out; out << string("START");
                                    socket.send(out);
                                }
                            }
                        } else if (ready && !started) {
                            ready = false;
                            if (UdebugMode) cout << "Niegotowy!" << "\n";

                            sf::Packet out; out << string("NOTREADY") << true;
                            socket.send(out);
                        }
                    }
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
						if (UshowBoard == true) {
                            cout << "\n\n\nPLAYER\n";
                            for (int i = 0; i < 10; i++) {
                                for (int j = 0; j < 10; j++) {
                                    cout << " " << localTab.getValue(i, j) << " ";
                                }
                                cout << "\n";
                            }
                        }

                        if (UshowEnemyBoard == true) {
                            cout << "\n\nENEMY\n";
                            for (int i = 0; i < 10; i++) {
                                for (int j = 0; j < 10; j++) {
                                    cout << " " << enemyTab.getValue(i, j) << " ";
                                }
                                cout << "\n";
                            }
                            cout << "\n\n";
                        }
                    }
                }
            }

            window.clear(sf::Color::Black);

            sf::Text text;
            text.setFont(font);
            createBoards(window, text, localTab, enemyTab);
            createBottomInfo(window, text, localTab, enemyTab);

            window.display();
        }
    } else {
        sf::RenderWindow window(sf::VideoMode(950, 600), "Ships (network)");

        while (window.isOpen()) {
            // odbiór pakietów (non-blocking)
            if (socket.getRemoteAddress() != sf::IpAddress::None || (!isHost && socket.getRemoteAddress() != sf::IpAddress::None)) {
                sf::Packet packet;
                sf::Socket::Status recvStatus = socket.receive(packet);
                if (recvStatus == sf::Socket::Done) {
                    string cmd; packet >> cmd;
                    if (cmd == "READY") {
                        readyEnemy = true;

                        if (ready && readyEnemy) {
                            started = true;
                            if (UdebugMode) cout << "Started!";

                            sf::Packet out; out << string("START") << true;
                            socket.send(out);
                        } else {
                            sf::Packet out; out << string("RESULT") << true;
                            socket.send(out);
                        }

                    } else if (cmd == "NOTREADY") {
                        readyEnemy = false;
                        
                        sf::Packet out; out << string("RESULT") << true;
                        socket.send(out);

                    } else if (cmd == "START") {
                        readyEnemy = true;
                        ready = true;
                        started = true;

                    } else if (cmd == "PLACE") {
                        int col, row; packet >> col >> row;
                        if (UdebugMode) cout << "Otrzymano: " << col << "," << row << "\n";

                        enemyTab.placeShip(row, col);

                        sf::Packet out; out << string("RESULT") << true;
                        socket.send(out);

                    } else if (cmd == "SHOT") {
                        int col, row; packet >> col >> row;
                        turn = !turn;
                        if (UdebugMode) cout << "Otrzymano strzał: " << col << "," << row << "\n";

                        localTab.handlePlayerShot(row, col);

                        sf::Packet out; out << string("RESULT") << true;
                        socket.send(out);

                    } else if (cmd == "TURN") {
                        if (UdebugMode) cout << "OTRZYMANO DANE RUCHU";
						turn = true;

                    } else if (cmd == "DISCONECTED") {
                        cout << "Player disconected!" << "\n";
                        return 0;

                    } else if (cmd == "RESULT") {
                        // RESULT - TRUE

                    } else {
                        if (UdebugMode) cout << "cmdE: " << cmd;
                    }
                }
            }

            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    sf::Packet out; out << string("DISCONECTED");
                    socket.send(out);

                    window.close();
                };

                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        int mouseX = event.mouseButton.x;
                        int mouseY = event.mouseButton.y;


                        if (isMouseOverEnemyBoard(mouseX, mouseY)) {
                            pair<int, int> pos = getEnemyBoardPosition(mouseX, mouseY);
                            int row = pos.first;
                            int col = pos.second;

                            if (row >= 0 && row < 10 && col >= 0 && col < 10) {
                                if (started && turn) {
                                    bool result = enemyTab.handlePlayerShot(row, col);

                                    if (result) {
                                        turn = !turn;
                                        if (UdebugMode) cout << "SHOT SENDED!!!" << "\n";

                                        sf::Packet out; out << string("SHOT") << col << row;
                                        if (socket.getRemoteAddress() != sf::IpAddress::None) socket.send(out);
                                    }
                                }
                            }
                        }

                        if (isMouseOverLocalBoard(mouseX, mouseY)) {
                            pair<int, int> pos = getLocalBoardPosition(mouseX, mouseY);
                            int row = pos.first;
                            int col = pos.second;

                            if (row >= 0 && row < 10 && col >= 0 && col < 10) {
                                if (!started && !ready) {
                                    localTab.placeShip(row, col);
                                    sf::Packet out; out << string("PLACE") << col << row;
                                    if (socket.getRemoteAddress() != sf::IpAddress::None) socket.send(out);
                                }
                            }
                        }
                    }
                }

                if (event.type == sf::Event::KeyPressed) {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) {
                        if (ready == false) {
                            if (localTab.getShipsAmount() == localTab.getMaxShipsAmount()) {
                                ready = true;
                                if (UdebugMode) cout << "Gotowy!" << "\n";

                                sf::Packet out; out << string("READY") << true;
                                socket.send(out);

                                if (ready && readyEnemy) {
                                    started = true;
                                    if (UdebugMode) cout << "Start!" << "\n";

                                    sf::Packet out; out << string("START");
                                    socket.send(out);
                                }
                            }
                        }
                        else if (ready && !started) {
                            ready = false;
                            if (UdebugMode) cout << "Niegotowy!" << "\n";

                            sf::Packet out; out << string("NOTREADY") << true;
                            socket.send(out);
                        }
                    }
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
                        if (UshowBoard == true) {
                            cout << "\n\n\nPLAYER\n";
                            for (int i = 0; i < 10; i++) {
                                for (int j = 0; j < 10; j++) {
                                    cout << " " << localTab.getValue(i, j) << " ";
                                }
                                cout << "\n";
                            }
                        }

                        if (UshowEnemyBoard == true) {
                            cout << "\n\nENEMY\n";
                            for (int i = 0; i < 10; i++) {
                                for (int j = 0; j < 10; j++) {
                                    cout << " " << enemyTab.getValue(i, j) << " ";
                                }
                                cout << "\n";
                            }
                            cout << "\n\n";
                        }
                    }
                }
            }

            window.clear(sf::Color::Black);

            sf::Text text;
            text.setFont(font);
            createBoards(window, text, localTab, enemyTab);
            createBottomInfo(window, text, localTab, enemyTab);

            window.display();
        }
    }

    return 0;
}