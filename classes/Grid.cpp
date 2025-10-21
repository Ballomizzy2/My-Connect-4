#include "Grid.h"

Grid::Grid(int width, int height) : _width(width), _height(height)
{
    // Initialize 2D vectors
    _squares.resize(height);
    _enabled.resize(height);

    for (int y = 0; y < height; y++) {
        _squares[y].resize(width);
        _enabled[y].resize(width);

        for (int x = 0; x < width; x++) {
            _squares[y][x] = new ChessSquare();
            _enabled[y][x] = true; // All squares enabled by default
        }
    }
}

Grid::~Grid()
{
    for (int y = 0; y < _height; y++) {
        for (int x = 0; x < _width; x++) {
            delete _squares[y][x];
        }
    }
}

ChessSquare* Grid::getSquare(int x, int y)
{
    if (!isValid(x, y)) return nullptr;
    return _squares[y][x];
}

ChessSquare* Grid::getSquareByIndex(int index)
{
    int x, y;
    getCoordinates(index, x, y);
    return getSquare(x, y);
}

bool Grid::isValid(int x, int y) const
{
    return x >= 0 && x < _width && y >= 0 && y < _height;
}

bool Grid::isEnabled(int x, int y) const
{
    if (!isValid(x, y)) return false;
    return _enabled[y][x];
}

void Grid::setEnabled(int x, int y, bool enabled)
{
    if (isValid(x, y)) {
        _enabled[y][x] = enabled;
    }
}

void Grid::getCoordinates(int index, int& x, int& y) const
{
    x = index % _width;
    y = index / _width;
}

// Directional helpers
ChessSquare* Grid::getFL(int x, int y)
{
    // Front-left: up and left (row decreases, column decreases)
    return getSquare(x - 1, y - 1);
}

ChessSquare* Grid::getFR(int x, int y)
{
    // Front-right: up and right (row decreases, column increases)
    return getSquare(x + 1, y - 1);
}

ChessSquare* Grid::getBL(int x, int y)
{
    // Back-left: down and left (row increases, column decreases)
    return getSquare(x - 1, y + 1);
}

ChessSquare* Grid::getBR(int x, int y)
{
    // Back-right: down and right (row increases, column increases)
    return getSquare(x + 1, y + 1);
}

ChessSquare* Grid::getN(int x, int y) // north
{
    return getSquare(x, y - 1);
}

ChessSquare* Grid::getS(int x, int y) // south
{
    return getSquare(x, y + 1);
}

ChessSquare* Grid::getE(int x, int y) // east
{
    return getSquare(x + 1, y);
}

ChessSquare* Grid::getW(int x, int y) // west
{
    return getSquare(x - 1, y);
}

std::unordered_set<ChessSquare*> Grid::getImmediateSurrounding(int x, int y)
{
    std::unordered_set<ChessSquare*> set;
    set.insert(getN(x,y));
    set.insert(getE(x,y));
    set.insert(getS(x,y));
    set.insert(getW(x,y));

    set.insert(getFR(x,y));
    set.insert(getFL(x,y));
    set.insert(getBR(x,y));
    set.insert(getBL(x,y));
    
    return set;
}

// Graph connections
void Grid::addConnection(int fromIndex, int toIndex)
{
    if(toIndex == -1)
        return;

    // just insert to the set part of the map normally
    
    
    _connections[fromIndex].insert(toIndex);
    _connections[fromIndex].insert(fromIndex);
    

    //_connections[fromIndex].push_back(toIndex);
    // if toIndex points to more indexes, use list addition -> addConnection with set paramenter
    auto it = _connections.find(toIndex);
    if(it != _connections.end())
    {
        addConnection(fromIndex, it->second);
    }
}

void Grid::addConnection(int fromIndex, std::unordered_set<int> nextIndexes)
{
    if(nextIndexes.empty())
        return;

    // we need to make sure it is in a valid direction before inserting
    for (int index : nextIndexes)
    {
        _connections[fromIndex].insert(index);    
    }
}

void Grid::addConnection(int fromX, int fromY, int toX, int toY)
{
    addConnection(getIndex(fromX, fromY), getIndex(toX, toY));
}

std::vector<ChessSquare*> Grid::getConnectedSquares(int x, int y)
{
    std::vector<ChessSquare*> connected;
    int gameTag = getSquare(x, y)->gameTag();
    
    // Check horizontal (left to right)
    int count = 0;
    for (int dx = -3; dx <= 3; dx++) {
        if (isValid(x + dx, y)) {
            ChessSquare* square = getSquare(x + dx, y);
            if (square && square->gameTag() == gameTag) {
                count++;
                if (count >= 4) {
                    // Found 4 in a row horizontally
                    return std::vector<ChessSquare*>{square, getSquare(x + dx - 1, y),
                                                   getSquare(x + dx - 2, y), getSquare(x + dx - 3, y)};
                }
            } else {
                count = 0;
            }
        }
    }

    // Check vertical (bottom to top)
    count = 0;
    for (int dy = -3; dy <= 3; dy++) {
        if (isValid(x, y + dy)) {
            ChessSquare* square = getSquare(x, y + dy);
            if (square && square->gameTag() == gameTag) {
                count++;
                if (count >= 4) {
                    // Found 4 in a row vertically
                    return std::vector<ChessSquare*>{square, getSquare(x, y + dy - 1),
                                                   getSquare(x, y + dy - 2), getSquare(x, y + dy - 3)};
                }
            } else {
                count = 0;
            }
        }
    }

    // Check diagonal (bottom-left to top-right)
    count = 0;
    for (int d = -3; d <= 3; d++) {
        if (isValid(x + d, y + d)) {
            ChessSquare* square = getSquare(x + d, y + d);
            if (square && square->gameTag() == gameTag) {
                count++;
                if (count >= 4) {
                    // Found 4 in a row diagonally
                    return std::vector<ChessSquare*>{square, getSquare(x + d - 1, y + d - 1),
                                                   getSquare(x + d - 2, y + d - 2), getSquare(x + d - 3, y + d - 3)};
                }
            } else {
                count = 0;
            }
        }
    }

    // Check diagonal (top-left to bottom-right)
    count = 0;
    for (int d = -3; d <= 3; d++) {
        if (isValid(x + d, y - d)) {
            ChessSquare* square = getSquare(x + d, y - d);
            if (square && square->gameTag() == gameTag) {
                count++;
                if (count >= 4) {
                    // Found 4 in a row diagonally
                    return std::vector<ChessSquare*>{square, getSquare(x + d - 1, y - d + 1),
                                                   getSquare(x + d - 2, y - d + 2), getSquare(x + d - 3, y - d + 3)};
                }
            } else {
                count = 0;
            }
        }
    }

    return connected;
}

bool Grid::areConnected(int fromX, int fromY, int toX, int toY)
{
    int fromIndex = getIndex(fromX, fromY);
    int toIndex = getIndex(toX, toY);

    if (_connections.find(fromIndex) != _connections.end()) {
        const auto& connections = _connections[fromIndex];
        return std::find(connections.begin(), connections.end(), toIndex) != connections.end();
    }

    return false;
}
bool Grid::areConnected(int fromIndex, int toIndex)
{

    if (_connections.find(fromIndex) != _connections.end()) {
        const auto& connections = _connections[fromIndex];
        return std::find(connections.begin(), connections.end(), toIndex) != connections.end();
    }

    return false;
}

// Iterator support
void Grid::forEachSquare(std::function<void(ChessSquare*, int x, int y)> func)
{
    for (int y = 0; y < _height; y++) {
        for (int x = 0; x < _width; x++) {
            func(_squares[y][x], x, y);
        }
    }
}

void Grid::forEachEnabledSquare(std::function<void(ChessSquare*, int x, int y)> func)
{
    for (int y = 0; y < _height; y++) {
        for (int x = 0; x < _width; x++) {
            func(_squares[y][x], x, y);
            // if (_enabled[y][x]) {
            //     func(_squares[y][x], x, y);
            // }
        }
    }
}

// Initialize squares
void Grid::initializeSquares(float squareSize, const char* spriteName)
{
    for (int y = 0; y < _height; y++) {
        for (int x = 0; x < _width; x++) {
            initializeSquare(x, y, squareSize, spriteName);
        }
    }
}

void Grid::initializeSquare(int x, int y, float squareSize, const char* spriteName)
{
    if (isValid(x, y)) {
        ImVec2 position(squareSize * x + squareSize/2, squareSize * y + squareSize/2);
        _squares[y][x]->initHolder(position, spriteName, x, y);
    }
}

// State management
std::string Grid::getStateString() const
{
    std::string state;

    for (int y = 0; y < _height; y++) {
        for (int x = 0; x < _width; x++) {
            if (_enabled[y][x]) {
                Bit* bit = _squares[y][x]->bit();
                if (bit) {
                    state += std::to_string(bit->gameTag());
                } else {
                    state += '0';
                }
            }
        }
    }

    return state;
}

void Grid::setStateString(const std::string& state)
{
    size_t index = 0;

    for (int y = 0; y < _height && index < state.length(); y++) {
        for (int x = 0; x < _width && index < state.length(); x++) {
            if (_enabled[y][x]) {
                char pieceChar = state[index++];

                // Clear existing piece
                _squares[y][x]->destroyBit();

                // This method just sets the state - games need to create their own pieces
                // when loading from state string based on the piece type
            }
        }
    }
}