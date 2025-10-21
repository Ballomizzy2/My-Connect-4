#include "Connect4.h"

Connect4::Connect4() : Game() {
    _grid = new Grid(7, 6);
    _mustContinueJumping = false;
    _jumpingPiece = nullptr;
    _redPieces = 21;
    _yellowPieces = 21;
}

Connect4::~Connect4() {
    delete _grid;
}

void Connect4::setUpBoard() {
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;

    // Initialize all squares
    _grid->initializeSquares(75, "square.png");
    startGame();
}

Bit* Connect4::createPiece(int pieceType) {
    Bit* bit = new Bit();
    bool isRed = (pieceType == RED_PIECE || pieceType == RED_KING);
    bit->LoadTextureFromFile(isRed ? "red.png" : "yellow.png");
    bit->setOwner(getPlayerAt(isRed ? RED_PLAYER : YELLOW_PLAYER));
    bit->setGameTag(pieceType);
    if (pieceType == RED_KING || pieceType == YELLOW_KING)
        bit->setScale(1.3f);
    return bit;
}

bool Connect4::actionForEmptyHolder(BitHolder &holder) {

    // make sure we are only touching top row
    if(holder.getPosition().y > _grid->getSquareByIndex(0)->getPosition().y)
        return false;

    int currPlayerNum = getCurrentPlayer()->playerNumber();
    Bit* piece = createPiece(currPlayerNum); //depending on turn create a new piece
    
    // we need to look for the most available holder in that column
    // get the x value of clicked holder, loop through grid x column checking for holder.empty
    BitHolder *appropriateHolder = &holder;
    ChessSquare *c = (ChessSquare*)appropriateHolder;
    currPlayerNum == 0 ? 
        _yellowPieces-- :
        _redPieces--; // reduce peicez after each turmb
    appropriateHolder->setGameTag(currPlayerNum); // so we knbow which color is in holder yellow = 0, red = 1
    std::cout << "Curr GameTag for index" << _grid->getIndex(appropriateHolder->getPosition().x, appropriateHolder->getPosition().y) << " is " << appropriateHolder->gameTag() << std::endl;
    int xValue = c->getColumn();
    for (int i = _grid->getHeight() - 1; i > 0; i--) // start from bottom to up 
    {
        BitHolder *h = _grid->getSquare(xValue, i); 
        if (h->empty()) //find first empty spot
        {
            h->setGameTag(currPlayerNum);
            appropriateHolder = h;
            c = (ChessSquare*)h;
            break;
        }


    }
    piece->setPosition(appropriateHolder->getPosition()); 
    appropriateHolder->setBit(piece);
    lastBitCreated = piece;

    // No need to handle connections here anymore as we check for winning lines directly
    endTurn();
    return false; // Checkers doesn't place new pieces
}

bool Connect4::canBitMoveFrom(Bit &bit, BitHolder &src) {
    if (!src.bit() || bit.getOwner() != getCurrentPlayer()) return false;
    if (_mustContinueJumping && &src != _jumpingPiece) return false;

    ChessSquare* square = static_cast<ChessSquare*>(&src);
    int x = square->getColumn();
    int y = square->getRow();

    // Must jump if available
    if (hasJumpAvailable(bit.getOwner())) {
        return canJumpFrom(*square);
    }
    return true;
}

bool Connect4::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) {
    if (!src.bit() || dst.bit()) return false;

    ChessSquare* srcSquare = static_cast<ChessSquare*>(&src);
    ChessSquare* dstSquare = static_cast<ChessSquare*>(&dst);

    int srcX = srcSquare->getColumn();
    int srcY = srcSquare->getRow();

    if (!_grid->isEnabled(dstSquare->getColumn(), dstSquare->getRow())) return false;

    bool isKing = (bit.gameTag() == RED_KING || bit.gameTag() == YELLOW_KING);
    bool isRed = (bit.getOwner() == getPlayerAt(RED_PLAYER));

    // Simple moves (if no jumps required)
    if (!_mustContinueJumping && !hasJumpAvailable(bit.getOwner())) {
        if (isKing) {
            return dstSquare == _grid->getFL(srcX, srcY) || dstSquare == _grid->getFR(srcX, srcY) ||
                   dstSquare == _grid->getBL(srcX, srcY) || dstSquare == _grid->getBR(srcX, srcY);
        }
        return isRed ? (dstSquare == _grid->getBL(srcX, srcY) || dstSquare == _grid->getBR(srcX, srcY)) :
                       (dstSquare == _grid->getFL(srcX, srcY) || dstSquare == _grid->getFR(srcX, srcY));
    }

    // Jump moves
    if (_mustContinueJumping && &src != _jumpingPiece) return false;

    // Check all jump directions
    auto checkJump = [&](ChessSquare* middle, ChessSquare* target) -> bool {
        if (!middle || !target || !middle->bit()) return false;
        if (middle->bit()->getOwner() == bit.getOwner()) return false;
        return dstSquare == target;
    };

    if (isKing || !isRed) {
        if (checkJump(_grid->getFL(srcX, srcY), _grid->getFLFL(srcX, srcY))) return true;
        if (checkJump(_grid->getFR(srcX, srcY), _grid->getFRFR(srcX, srcY))) return true;
    }
    if (isKing || isRed) {
        if (checkJump(_grid->getBL(srcX, srcY), _grid->getBLBL(srcX, srcY))) return true;
        if (checkJump(_grid->getBR(srcX, srcY), _grid->getBRBR(srcX, srcY))) return true;
    }

    return false;
}

void Connect4::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
    ChessSquare* srcSquare = static_cast<ChessSquare*>(&src);
    ChessSquare* dstSquare = static_cast<ChessSquare*>(&dst);

    int srcX = srcSquare->getColumn();
    int srcY = srcSquare->getRow();
    int dstX = dstSquare->getColumn();
    int dstY = dstSquare->getRow();

    // Check for jump
    ChessSquare* jumped = nullptr;
    if (dstSquare == _grid->getFLFL(srcX, srcY)) jumped = _grid->getFL(srcX, srcY);
    else if (dstSquare == _grid->getFRFR(srcX, srcY)) jumped = _grid->getFR(srcX, srcY);
    else if (dstSquare == _grid->getBLBL(srcX, srcY)) jumped = _grid->getBL(srcX, srcY);
    else if (dstSquare == _grid->getBRBR(srcX, srcY)) jumped = _grid->getBR(srcX, srcY);

    if (jumped && jumped->bit()) {
        // Capture
        (jumped->bit()->getOwner() == getPlayerAt(RED_PLAYER)) ? _redPieces-- : _yellowPieces--;
        jumped->destroyBit();

        // Promotion check
        if ((bit.gameTag() == RED_PIECE && dstY == 7) || (bit.gameTag() == YELLOW_PIECE && dstY == 0)) {
            bit.setGameTag(bit.gameTag() == RED_PIECE ? RED_KING : YELLOW_KING);
            bit.setScale(1.3f);
        }

        // Check for more jumps
        if (canJumpFrom(*dstSquare)) {
            _mustContinueJumping = true;
            _jumpingPiece = &dst;
            return;
        }
    } else {
        // Regular move - promotion check
        if ((bit.gameTag() == RED_PIECE && dstY == 7) || (bit.gameTag() == YELLOW_PIECE && dstY == 0)) {
            bit.setGameTag(bit.gameTag() == RED_PIECE ? RED_KING : YELLOW_KING);
            bit.setScale(1.3f);
        }
    }

    _mustContinueJumping = false;
    _jumpingPiece = nullptr;
    endTurn();
}

bool Connect4::canJumpFrom(ChessSquare& square) const {
    Bit* piece = square.bit();
    if (!piece) return false;

    int x = square.getColumn();
    int y = square.getRow();
    bool isKing = (piece->gameTag() == RED_KING || piece->gameTag() == YELLOW_KING);
    bool isRed = (piece->getOwner() == getPlayerAt(RED_PLAYER));
    Player* player = piece->getOwner();

    auto checkJumpDir = [&](ChessSquare* middle, ChessSquare* target) -> bool {
        return middle && middle->bit() && middle->bit()->getOwner() != player &&
               target && !target->bit();
    };

    if (isKing || !isRed) {
        if (checkJumpDir(_grid->getFL(x, y), _grid->getFLFL(x, y))) return true;
        if (checkJumpDir(_grid->getFR(x, y), _grid->getFRFR(x, y))) return true;
    }
    if (isKing || isRed) {
        if (checkJumpDir(_grid->getBL(x, y), _grid->getBLBL(x, y))) return true;
        if (checkJumpDir(_grid->getBR(x, y), _grid->getBRBR(x, y))) return true;
    }
    return false;
}

bool Connect4::hasJumpAvailable(Player* player) const {    
    return false;
}

Player* Connect4::checkForWinner() {
    ChessSquare* currentHolder = (ChessSquare*)lastBitCreated->getHolder();
    
    // Check for winning lines in any direction
    auto connectedSquares = _grid->getConnectedSquares(currentHolder->getColumn(), currentHolder->getRow());
    if (!connectedSquares.empty()) {
        return getPlayerAt(!(getCurrentTurnNo()%2));
    }
    return nullptr;
}

bool Connect4::checkForDraw() {
    return _yellowPieces <= 0 && _redPieces <= 0 && !checkForWinner();
}

void Connect4::stopGame() {
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
    _mustContinueJumping = false;
    _jumpingPiece = nullptr;
    _redPieces = 21;
    _yellowPieces = 21;
}

std::string Connect4::initialStateString() {
    return "0000000000000000000000000000000000000000000000000"; 
    // we are fricking using bits for this, very low level so we ai can be more performant
}

std::string Connect4::stateString() {
    return _grid->getStateString();
}

void Connect4::setStateString(const std::string &s) {
    if (s.length() != 42) return; // make sure it is an appropriate size for the board

    _redPieces = 0;
    _yellowPieces = 0;

    _grid->setStateString(s);

    // Recreate pieces from state
    size_t index = 0;
    _grid->forEachEnabledSquare([&](ChessSquare* square, int x, int y) {
        if (index < s.length()) {
            int pieceType = s[index++] - '0';
            if (pieceType != 0) {
                Bit* piece = createPiece(pieceType);
                piece->setPosition(square->getPosition());
                square->setBit(piece);
                (pieceType == RED_PIECE || pieceType == RED_KING) ? _redPieces++ : _yellowPieces++;
            }
        }
    });
}

void Connect4::updateAI() {
    // Constants for alpha-beta algorithm
    const int MAX_DEPTH = 6;
    const int INFINITY_SCORE = 1000000;

    // Helper function to evaluate a line of 4 squares
    auto evaluateLine = [this](std::vector<ChessSquare*>& line) -> int {
        int aiCount = 0;
        int playerCount = 0;
        int empty = 0;
        
        for (ChessSquare* square : line) {
            if (!square || !square->bit()) {
                empty++;
            } else {
                if (square->gameTag() == getCurrentPlayer()->playerNumber()) {
                    aiCount++;
                } else {
                    playerCount++;
                }
            }
        }
        
        // If both players have pieces in the line, it's not valuable
        if (aiCount > 0 && playerCount > 0) return 0;
        
        // Score based on number of pieces
        auto getScore = [](int count, int empty) -> int {
            switch(count) {
                case 4: return 1000000;  // Winning position
                case 3: return empty > 0 ? 100 : 0;  // Three in a row with space
                case 2: return empty > 1 ? 10 : 0;   // Two in a row with spaces
                case 1: return empty > 2 ? 1 : 0;    // One with spaces
                default: return 0;
            }
        };
        
        if (aiCount > 0) return getScore(aiCount, empty);
        if (playerCount > 0) return -getScore(playerCount, empty);
        return 0;
    };

    // Get all possible lines of 4 squares
    auto getAllLines = [this]() -> std::vector<std::vector<ChessSquare*>> {
        std::vector<std::vector<ChessSquare*>> lines;
        
        // Horizontal lines
        for (int y = 0; y < _grid->getHeight(); y++) {
            for (int x = 0; x <= _grid->getWidth() - 4; x++) {
                std::vector<ChessSquare*> line;
                for (int i = 0; i < 4; i++) {
                    line.push_back(_grid->getSquare(x + i, y));
                }
                lines.push_back(line);
            }
        }
        
        // Vertical lines
        for (int x = 0; x < _grid->getWidth(); x++) {
            for (int y = 0; y <= _grid->getHeight() - 4; y++) {
                std::vector<ChessSquare*> line;
                for (int i = 0; i < 4; i++) {
                    line.push_back(_grid->getSquare(x, y + i));
                }
                lines.push_back(line);
            }
        }
        
        // Diagonal lines (bottom-left to top-right)
        for (int y = 3; y < _grid->getHeight(); y++) {
            for (int x = 0; x <= _grid->getWidth() - 4; x++) {
                std::vector<ChessSquare*> line;
                for (int i = 0; i < 4; i++) {
                    line.push_back(_grid->getSquare(x + i, y - i));
                }
                lines.push_back(line);
            }
        }
        
        // Diagonal lines (top-left to bottom-right)
        for (int y = 0; y <= _grid->getHeight() - 4; y++) {
            for (int x = 0; x <= _grid->getWidth() - 4; x++) {
                std::vector<ChessSquare*> line;
                for (int i = 0; i < 4; i++) {
                    line.push_back(_grid->getSquare(x + i, y + i));
                }
                lines.push_back(line);
            }
        }
        
        return lines;
    };

    // Evaluate current board position
    auto evaluatePosition = [&]() -> int {
        int score = 0;
        auto lines = getAllLines();
        for (auto& line : lines) {
            score += evaluateLine(line);
        }
        return score;
    };

    // Make temporary move
    auto makeTemporaryMove = [this](int column, int playerNum) -> bool {
        for (int y = _grid->getHeight() - 1; y >= 0; y--) {
            ChessSquare* square = _grid->getSquare(column, y);
            if (square && !square->bit()) {
                square->setGameTag(playerNum);
                return true;
            }
        }
        return false;
    };

    // Undo temporary move
    auto undoTemporaryMove = [this](int column) {
        for (int y = 0; y < _grid->getHeight(); y++) {
            ChessSquare* square = _grid->getSquare(column, y);
            if (square && square->gameTag() != -1) {
                square->setGameTag(-1);
                break;
            }
        }
    };

    // Get valid moves
    auto getValidMoves = [this]() -> std::vector<int> {
        std::vector<int> moves;
        for (int x = 0; x < _grid->getWidth(); x++) {
            if (!_grid->getSquare(x, 0)->bit()) {
                moves.push_back(x);
            }
        }
        return moves;
    };

    // Alpha-beta algorithm
    std::function<int(int, int, int, bool)> alphaBeta = [&](int depth, int alpha, int beta, bool maximizingPlayer) -> int {
        if (depth == 0) return evaluatePosition();
        
        auto validMoves = getValidMoves();
        if (validMoves.empty()) return 0;
        
        if (maximizingPlayer) {
            int maxEval = -INFINITY_SCORE;
            for (int column : validMoves) {
                if (makeTemporaryMove(column, getCurrentPlayer()->playerNumber())) {
                    int eval = alphaBeta(depth - 1, alpha, beta, false);
                    undoTemporaryMove(column);
                    maxEval = std::max(maxEval, eval);
                    alpha = std::max(alpha, eval);
                    if (beta <= alpha) break;
                }
            }
            return maxEval;
        } else {
            int minEval = INFINITY_SCORE;
            for (int column : validMoves) {
                if (makeTemporaryMove(column, !getCurrentPlayer()->playerNumber())) {
                    int eval = alphaBeta(depth - 1, alpha, beta, true);
                    undoTemporaryMove(column);
                    minEval = std::min(minEval, eval);
                    beta = std::min(beta, eval);
                    if (beta <= alpha) break;
                }
            }
            return minEval;
        }
    };

    // Find and make the best move
    auto validMoves = getValidMoves();
    int bestMove = validMoves[0];
    int bestScore = -INFINITY_SCORE;
    bool isMaximizingPlayer = true;
    
    for (int column : validMoves) {
        if (makeTemporaryMove(column, getCurrentPlayer()->playerNumber())) {
            int score = alphaBeta(MAX_DEPTH - 1, -INFINITY_SCORE, INFINITY_SCORE, !isMaximizingPlayer);
            undoTemporaryMove(column);
            
            if (score > bestScore) {
                bestScore = score;
                bestMove = column;
            }
        }
    }
    
    // Make the best move
    BitHolder* holder = _grid->getSquare(bestMove, 0);
    if (holder) {
        actionForEmptyHolder(*holder);
    }
}

