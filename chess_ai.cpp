#include <stdlib.h>
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <random>
#include <list>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cmath>

#include <cstddef>
#include <type_traits>
#include <utility>

#include <ctime>

using namespace std;

//typedef char player_indicator;
//typedef char coord;
// g++ -std=c++11 chess_test.cpp -o chess_test

#define NONE 0
#define DRAW 3

namespace std {
    template<class T> struct _Unique_if {
        typedef unique_ptr<T> _Single_object;
    };

    template<class T> struct _Unique_if<T[]> {
        typedef unique_ptr<T[]> _Unknown_bound;
    };

    template<class T, size_t N> struct _Unique_if<T[N]> {
        typedef void _Known_bound;
    };

    template<class T, class... Args>
        typename _Unique_if<T>::_Single_object
        make_unique(Args&&... args) {
            return unique_ptr<T>(new T(std::forward<Args>(args)...));
        }

    template<class T>
        typename _Unique_if<T>::_Unknown_bound
        make_unique(size_t n) {
            typedef typename remove_extent<T>::type U;
            return unique_ptr<T>(new U[n]());
        }

    template<class T, class... Args>
        typename _Unique_if<T>::_Known_bound
        make_unique(Args&&...) = delete;
}

struct Move
{
    string piece;
	int x;
	int y;
};


//======================================================================================================================================================================================



class GamePiece
{
public:
    GamePiece(char PieceColor, string Code) : mPieceColor(PieceColor), code(Code) {}
    ~GamePiece() {}
    virtual char GetPiece() = 0;
    char GetColor() {
        return mPieceColor;
    }
    string GetCode() {
        return code;
    }
    bool IsLegalMove(int iSrcRow, int iSrcCol, int iDestRow, int iDestCol, GamePiece* GameBoard[8][8]) {
        GamePiece* qpDest = GameBoard[iDestRow][iDestCol];
        if ((qpDest == 0) || (mPieceColor != qpDest->GetColor())) {
            return AreSquaresLegal(iSrcRow, iSrcCol, iDestRow, iDestCol, GameBoard);
        }
        return false;
    }
private:
    virtual bool AreSquaresLegal(int iSrcRow, int iSrcCol, int iDestRow, int iDestCol, GamePiece* GameBoard[8][8]) = 0;
    char mPieceColor;
    string code;
};

class PawnPiece : public GamePiece
{
public:
    PawnPiece(char PieceColor, string Code) : GamePiece(PieceColor, Code), firstMove(true) {}
    ~PawnPiece() {}
private:
    bool firstMove;
    virtual char GetPiece() {
        return 'P';
    }
    bool AreSquaresLegal(int iSrcRow, int iSrcCol, int iDestRow, int iDestCol, GamePiece* GameBoard[8][8]) {
        GamePiece* qpDest = GameBoard[iDestRow][iDestCol];
        if (qpDest == 0) {
            // Destination square is unoccupied
            if (iSrcCol == iDestCol) {
                if (GetColor() == 'W') {
                    if ((firstMove && iDestRow == iSrcRow + 2)){
                        firstMove = false;
                        return true;
                    }else if (iDestRow == iSrcRow + 1) {
                        return true;
                    }
                } else {
                    if ((firstMove && iDestRow == iSrcRow + 2)){
                        firstMove = false;
                        return true;
                    }else if (iDestRow == iSrcRow - 1) {
                        return true;
                    }
                }
            }
        } else {
            // Dest holds piece of opposite color
            if ((iSrcCol == iDestCol + 1) || (iSrcCol == iDestCol - 1)) {
                if (GetColor() == 'W') {
                    if (iDestRow == iSrcRow + 1) {
                        return true;
                    }
                } else {
                    if (iDestRow == iSrcRow - 1) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

class KnightPiece : public GamePiece
{
public:
    KnightPiece(char PieceColor, string Code) : GamePiece(PieceColor, Code) {}
    ~KnightPiece() {}
private:
    virtual char GetPiece() {
        return 'N';
    }
    bool AreSquaresLegal(int iSrcRow, int iSrcCol, int iDestRow, int iDestCol, GamePiece* GameBoard[8][8]) {
        // Destination square is unoccupied or occupied by opposite color
        if ((iSrcCol == iDestCol + 1) || (iSrcCol == iDestCol - 1)) {
            if ((iSrcRow == iDestRow + 2) || (iSrcRow == iDestRow - 2)) {
                return true;
            }
        }
        if ((iSrcCol == iDestCol + 2) || (iSrcCol == iDestCol - 2)) {
            if ((iSrcRow == iDestRow + 1) || (iSrcRow == iDestRow - 1)) {
                return true;
            }
        }
        return false;
    }
};

class BishopPiece : public GamePiece
{
public:
    BishopPiece(char PieceColor, string Code) : GamePiece(PieceColor, Code) {}
    ~BishopPiece() {}
private:
    virtual char GetPiece() {
        return 'B';
    }
    bool AreSquaresLegal(int iSrcRow, int iSrcCol, int iDestRow, int iDestCol, GamePiece* GameBoard[8][8]) {
        if ((iDestCol - iSrcCol == iDestRow - iSrcRow) || (iDestCol - iSrcCol == iSrcRow - iDestRow)) {
            // Make sure that all invervening squares are empty
            int iRowOffset = (iDestRow - iSrcRow > 0) ? 1 : -1;
            int iColOffset = (iDestCol - iSrcCol > 0) ? 1 : -1;
            int iCheckRow;
            int iCheckCol;
            for (iCheckRow = iSrcRow + iRowOffset, iCheckCol = iSrcCol + iColOffset;
                iCheckRow !=  iDestRow;
                iCheckRow = iCheckRow + iRowOffset, iCheckCol = iCheckCol + iColOffset)
            {
                if (GameBoard[iCheckRow][iCheckCol] != 0) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
};

class RookPiece : public GamePiece
{
public:
    RookPiece(char PieceColor, string Code) : GamePiece(PieceColor, Code) {}
    ~RookPiece() {}
private:
    virtual char GetPiece() {
        return 'R';
    }
    bool AreSquaresLegal(int iSrcRow, int iSrcCol, int iDestRow, int iDestCol, GamePiece* GameBoard[8][8]) {
        if (iSrcRow == iDestRow) {
            // Make sure that all invervening squares are empty
            int iColOffset = (iDestCol - iSrcCol > 0) ? 1 : -1;
            for (int iCheckCol = iSrcCol + iColOffset; iCheckCol !=  iDestCol; iCheckCol = iCheckCol + iColOffset) {
                if (GameBoard[iSrcRow][iCheckCol] != 0) {
                    return false;
                }
            }
            return true;
        } else if (iDestCol == iSrcCol) {
            // Make sure that all invervening squares are empty
            int iRowOffset = (iDestRow - iSrcRow > 0) ? 1 : -1;
            for (int iCheckRow = iSrcRow + iRowOffset; iCheckRow !=  iDestRow; iCheckRow = iCheckRow + iRowOffset) {
                if (GameBoard[iCheckRow][iSrcCol] != 0) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
};

class QueenPiece : public GamePiece
{
public:
    QueenPiece(char PieceColor, string Code) : GamePiece(PieceColor, Code) {}
    ~QueenPiece() {}
private:
    virtual char GetPiece() {
        return 'Q';
    }
    bool AreSquaresLegal(int iSrcRow, int iSrcCol, int iDestRow, int iDestCol, GamePiece* GameBoard[8][8]) {
        if (iSrcRow == iDestRow) {
            // Make sure that all invervening squares are empty
            int iColOffset = (iDestCol - iSrcCol > 0) ? 1 : -1;
            for (int iCheckCol = iSrcCol + iColOffset; iCheckCol !=  iDestCol; iCheckCol = iCheckCol + iColOffset) {
                if (GameBoard[iSrcRow][iCheckCol] != 0) {
                    return false;
                }
            }
            return true;
        } else if (iDestCol == iSrcCol) {
            // Make sure that all invervening squares are empty
            int iRowOffset = (iDestRow - iSrcRow > 0) ? 1 : -1;
            for (int iCheckRow = iSrcRow + iRowOffset; iCheckRow !=  iDestRow; iCheckRow = iCheckRow + iRowOffset) {
                if (GameBoard[iCheckRow][iSrcCol] != 0) {
                    return false;
                }
            }
            return true;
        } else if ((iDestCol - iSrcCol == iDestRow - iSrcRow) || (iDestCol - iSrcCol == iSrcRow - iDestRow)) {
            // Make sure that all invervening squares are empty
            int iRowOffset = (iDestRow - iSrcRow > 0) ? 1 : -1;
            int iColOffset = (iDestCol - iSrcCol > 0) ? 1 : -1;
            int iCheckRow;
            int iCheckCol;
            for (iCheckRow = iSrcRow + iRowOffset, iCheckCol = iSrcCol + iColOffset;
                iCheckRow !=  iDestRow;
                iCheckRow = iCheckRow + iRowOffset, iCheckCol = iCheckCol + iColOffset)
            {
                if (GameBoard[iCheckRow][iCheckCol] != 0) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
};

class KingPiece : public GamePiece
{
public:
    KingPiece(char PieceColor, string Code) : GamePiece(PieceColor, Code) {}
    ~KingPiece() {}
private:
    virtual char GetPiece() {
        return 'K';
    }
    bool AreSquaresLegal(int iSrcRow, int iSrcCol, int iDestRow, int iDestCol, GamePiece* GameBoard[8][8]) {
        int iRowDelta = iDestRow - iSrcRow;
        int iColDelta = iDestCol - iSrcCol;
        if (((iRowDelta >= -1) && (iRowDelta <= 1)) &&
            ((iColDelta >= -1) && (iColDelta <= 1)))
        {
            return true;
        }
        return false;
    }
};




//======================================================================================================================================================================================




class CBoard
{
public:
    unordered_map<string, Move> trackerW;
    unordered_map<string, Move> trackerB;

    CBoard() {
        for (int iRow = 0; iRow < 8; ++iRow) {
            for (int iCol = 0; iCol < 8; ++iCol) {
                MainGameBoard[iRow][iCol] = 0;
            }
        }
        // Allocate and place black pieces
        for (int iCol = 0; iCol < 8; ++iCol) {
            string id = to_string(iCol+1) + "BP";
            MainGameBoard[6][iCol] = new PawnPiece('B', id);
            //trackerB[id]={id, iCol, 6};
            trackerB[id].piece=id;
            trackerB[id].x=iCol;
            trackerB[id].y=6;
        }
        MainGameBoard[7][0] = new RookPiece('B', "1BR");
        //trackerB["1BR"]={"1BR", 0,7};
        trackerB["1BR"].piece="1BR";
        trackerB["1BR"].x=0;
        trackerB["1BR"].y=7;
        MainGameBoard[7][1] = new KnightPiece('B', "1BN");
        //trackerB["1BN"]={"1BN", 1,7};
        trackerB["1BN"].piece="1BN";
        trackerB["1BN"].x=1;
        trackerB["1BN"].y=7;
        MainGameBoard[7][2] = new BishopPiece('B', "1BB");
        //trackerB["1BB"]={"1BB", 2,7};
        trackerB["1BB"].piece="1BB";
        trackerB["1BB"].x=2;
        trackerB["1BB"].y=7;
        MainGameBoard[7][3] = new KingPiece('B', "BK");
        //trackerB["BK"]={"BK", 3,7};
        trackerB["BK"].piece="BK";
        trackerB["BK"].x=3;
        trackerB["BK"].y=7;
        MainGameBoard[7][4] = new QueenPiece('B', "BQ");
        //trackerB["BQ"]={"BQ", 4,7};
        trackerB["BQ"].piece="BQ";
        trackerB["BQ"].x=4;
        trackerB["BQ"].y=7;
        MainGameBoard[7][5] = new BishopPiece('B', "2BB");
        //trackerB["2BB"]={"2BB", 5,7};
        trackerB["2BB"].piece="2BB";
        trackerB["2BB"].x=5;
        trackerB["2BB"].y=7;
        MainGameBoard[7][6] = new KnightPiece('B', "2BN");
        //trackerB["2BN"]={"2BN", 6,7};
        trackerB["2BN"].piece="2BN";
        trackerB["2BN"].x=6;
        trackerB["2BN"].y=7;
        MainGameBoard[7][7] = new RookPiece('B', "2BR");
        //trackerB["2BR"]={"2BR", 7,7};
        trackerB["2BR"].piece="2BR";
        trackerB["2BR"].x=7;
        trackerB["2BR"].y=7;
        // Allocate and place white pieces
        for (int iCol = 0; iCol < 8; ++iCol) {
            string id = to_string(iCol+1) + "WP";
            MainGameBoard[1][iCol] = new PawnPiece('W', id);
            //trackerW[id]={id, iCol,1};
            trackerW[id].piece=id;
            trackerW[id].x=iCol;
            trackerW[id].y=1;
        }
        MainGameBoard[0][0] = new RookPiece('W', "1WR");
        //trackerW["1WR"]={"1WR", 0,0};
        trackerW["1WR"].piece="1WR";
        trackerW["1WR"].x=0;
        trackerW["1WR"].y=0;
        MainGameBoard[0][1] = new KnightPiece('W', "2WN");
        //trackerW["2WN"]={"2WN", 1,0};
        trackerW["2WN"].piece="2WN";
        trackerW["2WN"].x=1;
        trackerW["2WN"].y=0;
        MainGameBoard[0][2] = new BishopPiece('W', "3WB");
        //trackerW["3WB"]={"3WB", 2,0};
        trackerW["3WB"].piece="3WB";
        trackerW["3WB"].x=2;
        trackerW["3WB"].y=0;
        MainGameBoard[0][3] = new KingPiece('W', "WK");
        //trackerW["WK"]={"WK", 3,0};
        trackerW["WK"].piece="WK";
        trackerW["WK"].x=3;
        trackerW["WK"].y=0;
        MainGameBoard[0][4] = new QueenPiece('W', "WQ");
        //trackerW["WQ"]={"WQ", 4,0};
        trackerW["WQ"].piece="WQ";
        trackerW["WQ"].x=4;
        trackerW["WQ"].y=0;
        MainGameBoard[0][5] = new BishopPiece('W', "6WB");
        //trackerW["6WB"]={"6WB", 5,0};
        trackerW["6WB"].piece="6WB";
        trackerW["6WB"].x=5;
        trackerW["6WB"].y=0;
        MainGameBoard[0][6] = new KnightPiece('W', "7WN");
        //trackerW["7WN"]={"7WN", 6,0};
        trackerW["7WN"].piece="7WN";
        trackerW["7WN"].x=6;
        trackerW["7WN"].y=0;
        MainGameBoard[0][7] = new RookPiece('W', "8WR");
        //trackerW["8WR"]={"8WR", 7,0};
        trackerW["8WR"].piece="8WR";
        trackerW["8WR"].x=7;
        trackerW["8WR"].y=0;
    }
    ~CBoard() {
        for (int iRow = 0; iRow < 8; ++iRow) {
            for (int iCol = 0; iCol < 8; ++iCol) {
                //delete MainGameBoard[iRow][iCol];
                MainGameBoard[iRow][iCol] = 0;
            }
        }
    }

    void Print() {
        //using namespace std;
        const int kiSquareWidth = 4;
        const int kiSquareHeight = 3;
        for (int iRow = 0; iRow < 8*kiSquareHeight; ++iRow) {
            int iSquareRow = iRow/kiSquareHeight;
            // Print side border with numbering
            if (iRow % 3 == 1) {
                cout << '-' << (char)('1' + 7 - iSquareRow) << '-';
            } else {
                cout << "---";
            }
            // Print the chess board
            for (int iCol = 0; iCol < 8*kiSquareWidth; ++iCol) {
                int iSquareCol = iCol/kiSquareWidth;
                if (((iRow % 3) == 1) && ((iCol % 4) == 1 || (iCol % 4) == 2) && MainGameBoard[7-iSquareRow][iSquareCol] != 0) {
                    if ((iCol % 4) == 1) {
                        cout << MainGameBoard[7-iSquareRow][iSquareCol]->GetColor();
                    } else {
                        cout << MainGameBoard[7-iSquareRow][iSquareCol]->GetPiece();
                    }
                } else {
                    if ((iSquareRow + iSquareCol) % 2 == 1) {
                        cout << '*';
                    } else {
                        cout << ' ';
                    }
                }
            }
            cout << endl;
        }
        // Print the bottom border with numbers
        for (int iRow = 0; iRow < kiSquareHeight; ++iRow) {
            if (iRow % 3 == 1) {
                cout << "---";
                for (int iCol = 0; iCol < 8*kiSquareWidth; ++iCol) {
                    int iSquareCol = iCol/kiSquareWidth;
                    if ((iCol % 4) == 1) {
                        cout << (iSquareCol + 1);
                    } else {
                        cout << '-';
                    }
                }
                cout << endl;
            } else {
                for (int iCol = 1; iCol < 9*kiSquareWidth; ++iCol) {
                    cout << '-';
                }
                cout << endl;
            }
        }
    }

    bool IsInCheck(char PieceColor) {
        // Find the king
        int iKingRow;
        int iKingCol;
        for (int iRow = 0; iRow < 8; ++iRow) {
            for (int iCol = 0; iCol < 8; ++iCol) {
                if (MainGameBoard[iRow][iCol] != 0) {
                    if (MainGameBoard[iRow][iCol]->GetColor() == PieceColor) {
                        if (MainGameBoard[iRow][iCol]->GetPiece() == 'K') {
                            iKingRow = iRow;
                            iKingCol = iCol;
                        }
                    }
                }
            }
        }
        // Run through the opponent's pieces and see if any can take the king
        for (int iRow = 0; iRow < 8; ++iRow) {
            for (int iCol = 0; iCol < 8; ++iCol) {
                if (MainGameBoard[iRow][iCol] != 0) {
                    if (MainGameBoard[iRow][iCol]->GetColor() != PieceColor) {
                        if (MainGameBoard[iRow][iCol]->IsLegalMove(iRow, iCol, iKingRow, iKingCol, MainGameBoard)) {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    bool CanMove(char PieceColor) {
        // Run through all pieces
        for (int iRow = 0; iRow < 8; ++iRow) {
            for (int iCol = 0; iCol < 8; ++iCol) {
                if (MainGameBoard[iRow][iCol] != 0) {
                    // If it is a piece of the current player, see if it has a legal move
                    if (MainGameBoard[iRow][iCol]->GetColor() == PieceColor) {
                        for (int iMoveRow = 0; iMoveRow < 8; ++iMoveRow) {
                            for (int iMoveCol = 0; iMoveCol < 8; ++iMoveCol) {
                                if (MainGameBoard[iRow][iCol]->IsLegalMove(iRow, iCol, iMoveRow, iMoveCol, MainGameBoard)) {
                                    // Make move and check whether king is in check
                                    GamePiece* qpTemp                   = MainGameBoard[iMoveRow][iMoveCol];
                                    MainGameBoard[iMoveRow][iMoveCol]   = MainGameBoard[iRow][iCol];
                                    MainGameBoard[iRow][iCol]           = 0;
                                    bool bCanMove = !IsInCheck(PieceColor);
                                    // Undo the move
                                    MainGameBoard[iRow][iCol]           = MainGameBoard[iMoveRow][iMoveCol];
                                    MainGameBoard[iMoveRow][iMoveCol]   = qpTemp;
                                    if (bCanMove) {
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    GamePiece* MainGameBoard[8][8];
};


class ChessBoard
{
public:
    CBoard mqGameBoard;
    char mcPlayerTurn;

    ChessBoard() : mcPlayerTurn('W') {}
    ~ChessBoard() {}

    void Start() {
        do {
            GetNextMove(mqGameBoard.MainGameBoard);
            AlternateTurn();
            
        } while (!IsGameOver());
        mqGameBoard.Print();
    }

    void GetNextMove(GamePiece* GameBoard[8][8]) {
        //using namespace std;
        bool bValidMove     = false;
        do {
            system ("clear");
            cout<<endl<<endl<<"          Welcome to Chess Game Developed by Cppsecrets "<<endl<<endl<<endl;
            cout<<"                      Keys to sysmbols used "<<endl<<endl<<endl;
            cout<<" * = white space"<<endl;
            cout<<" Blank space = black space"<<endl;
            cout<<" WP = White pawn &  BP = Black pawn"<<endl;
            cout<<" WN = White Knight & BN = Black Knight"<<endl;
            cout<<" WB = White Bishop & BB = Black Bishop"<<endl;
            cout<<" WR = White Rook & BR = Black Rook"<<endl;
            cout<<" WQ = White Queen & BQ = Black Queen"<<endl;
            cout<<" WK = White King & BK =Black King"<<endl;
            cout<<"Rule for move is :"<<endl;
            cout<<"Move by selecting row & column to another valid location using row & column"<<endl<<endl<<endl;
            mqGameBoard.Print();

            // Get input and convert to coordinates
            cout << mcPlayerTurn << "'s Move: ";
            int iStartMove;
            cin >> iStartMove;
            if(iStartMove<0){
                break;
            }
            int iStartRow = (iStartMove / 10) - 1;
            int iStartCol = (iStartMove % 10) - 1;

            cout << "To: ";
            int iEndMove;
            cin >> iEndMove;
            if(iEndMove<0){
                break;
            }
            int iEndRow = (iEndMove / 10) - 1;
            int iEndCol = (iEndMove % 10) - 1;

            // Check that the indices are in range
            // and that the source and destination are different
            if ((iStartRow >= 0 && iStartRow <= 7) &&
                (iStartCol >= 0 && iStartCol <= 7) &&
                (iEndRow >= 0 && iEndRow <= 7) &&
                (iEndCol >= 0 && iEndCol <= 7)) {
                // Additional checks in here
                GamePiece* qpCurrPiece = GameBoard[iStartRow][iStartCol];
                // Check that the piece is the correct color
                if ((qpCurrPiece != 0) && (qpCurrPiece->GetColor() == mcPlayerTurn)) {
                    // Check that the destination is a valid destination
                    if (qpCurrPiece->IsLegalMove(iStartRow, iStartCol, iEndRow, iEndCol, GameBoard)) {
                        // Make the move
                        GamePiece* tmp                   = GameBoard[iStartRow][iStartCol];
                        GamePiece* qpTemp                   = GameBoard[iEndRow][iEndCol];
                        GameBoard[iEndRow][iEndCol]     = GameBoard[iStartRow][iStartCol];
                        GameBoard[iStartRow][iStartCol] = 0;
                        // Make sure that the current player is not in check
                        if (!mqGameBoard.IsInCheck(mcPlayerTurn)) {
                            //delete qpTemp;
                            bValidMove = true;

                            string code = (*tmp).GetCode();
                            //mqGameBoard.trackerW[code]={code, iEndCol, iEndRow};
                            mqGameBoard.trackerW[code].piece=code;
                            mqGameBoard.trackerW[code].x=iEndCol;
                            mqGameBoard.trackerW[code].y=iEndRow;

                        } else { // Undo the last move
                            GameBoard[iStartRow][iStartCol] = GameBoard[iEndRow][iEndCol];
                            GameBoard[iEndRow][iEndCol]     = qpTemp;
                        }
                    }
                }
            }
            if (!bValidMove) {
                cout << "Invalid Move!" << endl;
            }
        } while (!bValidMove);
    }

    void AlternateTurn() {
        mcPlayerTurn = (mcPlayerTurn == 'W') ? 'B' : 'W';
    }

    bool IsGameOver() {
        // Check that the current player can move
        // If not, we have a stalemate or checkmate
        bool bCanMove(false);
        bCanMove = mqGameBoard.CanMove(mcPlayerTurn);
        if (!bCanMove) {
            if (mqGameBoard.IsInCheck(mcPlayerTurn)) {
                AlternateTurn();
                std::cout << "Checkmate, " << mcPlayerTurn << " Wins!" << std::endl;
            } else {
                std::cout << "Stalemate!" << std::endl;
            }
        }
        return !bCanMove;
    }
    
};




//======================================================================================================================================================================================



class GameState
{
public:
    CBoard mqGameBoard;
    char mcPlayerTurn;
    char winner = NONE;
    //static std::mt19937 g;
    //static std::random_device rd;
    static std::random_device rd;
    static std::mt19937 g;

    GameState()
    {
        //std::memset(mqGameBoard.MainGameBoard, NONE, sizeof(mqGameBoard.MainGameBoard));
        
    }

    char getPlayer(){
        return mcPlayerTurn;
    }

    void AlternateTurn() {
        mcPlayerTurn = (mcPlayerTurn == 'W') ? 'B' : 'W';
    }

    bool IsGameOver() {
        bool bCanMove(false);
        bCanMove = mqGameBoard.CanMove(mcPlayerTurn);
        if (!bCanMove) {
            if (mqGameBoard.IsInCheck(mcPlayerTurn)) {
                AlternateTurn();
                winner = mcPlayerTurn;
                //std::cout << "Checkmate, " << mcPlayerTurn << " Wins!" << std::endl;
            } else {
                winner = DRAW;
                //std::cout << "Stalemate!" << std::endl;
            }
        }
        return !bCanMove;
    }
    

    void GetNextMove() {
        //using namespace std;
        bool bValidMove     = false;
        do {
            system ("clear");
            cout<<endl<<endl<<"          Welcome to Chess Game Developed by Cppsecrets "<<endl<<endl<<endl;
            cout<<"                      Keys to sysmbols used "<<endl<<endl<<endl;
            cout<<" * = white space"<<endl;
            cout<<" Blank space = black space"<<endl;
            cout<<" WP = White pawn &  BP = Black pawn"<<endl;
            cout<<" WN = White Knight & BN = Black Knight"<<endl;
            cout<<" WB = White Bishop & BB = Black Bishop"<<endl;
            cout<<" WR = White Rook & BR = Black Rook"<<endl;
            cout<<" WQ = White Queen & BQ = Black Queen"<<endl;
            cout<<" WK = White King & BK =Black King"<<endl;
            cout<<"Rule for move is :"<<endl;
            cout<<"Move by selecting row & column to another valid location using row & column"<<endl<<endl<<endl;
            mqGameBoard.Print();

            // Get input and convert to coordinates
            cout << mcPlayerTurn << "'s Move: ";
            int iStartMove;
            cin >> iStartMove;
            if(iStartMove<0){
                break;
            }
            int iStartRow = (iStartMove / 10) - 1;
            int iStartCol = (iStartMove % 10) - 1;

            cout << "To: ";
            int iEndMove;
            cin >> iEndMove;
            if(iEndMove<0){
                break;
            }
            int iEndRow = (iEndMove / 10) - 1;
            int iEndCol = (iEndMove % 10) - 1;

            // Check that the indices are in range
            // and that the source and destination are different
            if ((iStartRow >= 0 && iStartRow <= 7) &&
                (iStartCol >= 0 && iStartCol <= 7) &&
                (iEndRow >= 0 && iEndRow <= 7) &&
                (iEndCol >= 0 && iEndCol <= 7)) {
                // Additional checks in here
                GamePiece* qpCurrPiece = mqGameBoard.MainGameBoard[iStartRow][iStartCol];
                // Check that the piece is the correct color
                if ((qpCurrPiece != 0) && (qpCurrPiece->GetColor() == mcPlayerTurn)) {
                    // Check that the destination is a valid destination
                    if (qpCurrPiece->IsLegalMove(iStartRow, iStartCol, iEndRow, iEndCol, mqGameBoard.MainGameBoard)) {
                        // Make the move
                        GamePiece* tmp                   = mqGameBoard.MainGameBoard[iStartRow][iStartCol];
                        GamePiece* qpTemp                   = mqGameBoard.MainGameBoard[iEndRow][iEndCol];
                        mqGameBoard.MainGameBoard[iEndRow][iEndCol]     = mqGameBoard.MainGameBoard[iStartRow][iStartCol];
                        mqGameBoard.MainGameBoard[iStartRow][iStartCol] = 0;
                        // Make sure that the current player is not in check
                        if (!mqGameBoard.IsInCheck(mcPlayerTurn)) {
                            //delete qpTemp;
                            bValidMove = true;

                            string code = (*tmp).GetCode();
                            //mqGameBoard.trackerW[code]={code, iEndCol, iEndRow};
                            mqGameBoard.trackerW[code].piece=code;
                            mqGameBoard.trackerW[code].x=iEndCol;
                            mqGameBoard.trackerW[code].y=iEndRow;
                        } else { // Undo the last move
                            mqGameBoard.MainGameBoard[iStartRow][iStartCol] = mqGameBoard.MainGameBoard[iEndRow][iEndCol];
                            mqGameBoard.MainGameBoard[iEndRow][iEndCol]     = qpTemp;
                        }
                    }
                }
            }
            if (!bValidMove) {
                cout << "Invalid Move!" << endl;
            }
        } while (!bValidMove);
    }

    vector<Move> getAllLegalMoves(){
        // used for AI
        vector<Move> moves;
        for(auto &it: mqGameBoard.trackerB){
            int x = it.second.x;
            int y = it.second.y;
            int dir1[5] = {0,1,0,-1,0};
            int dir2[5] = {1,1,-1,-1,1};

            if(it.first[2]=='P'){
                Move tmp = {it.first, x, y + (it.first[1]=='W'? 1:-1)};
                moves.push_back(tmp);
            }
            if(it.first[2]=='K'){
                for(int i=0; i<4; i++){
                    int x_i = x+dir1[i];
                    int y_i = y+dir1[i+1];
                    if(y_i>=0 && y_i<8 && x_i>=0 && x_i<8){
                        Move tmp = {it.first, x_i, y_i};
                        moves.push_back(tmp);
                    }
                    x_i = x+dir2[i];
                    y_i = y+dir2[i+1];
                    if(y_i>=0 && y_i<8 && x_i>=0 && x_i<8){
                        Move tmp = {it.first, x_i, y_i};
                        moves.push_back(tmp);
                    }
                }
            }
            if(it.first[2]=='R' || it.first[2]=='Q'){
                for(int i=0; i<4; i++){
                    int x_i = x+dir1[i];
                    int y_i = y+dir1[i+1];
                    while(y_i>=0 && y_i<8 && x_i>=0 && x_i<8 && mqGameBoard.MainGameBoard[y_i][x_i] == 0 || (*(mqGameBoard.MainGameBoard[y_i][x_i])).GetColor()==getPreviousPlayer()){
                        Move tmp = {it.first, x_i, y_i};
                        moves.push_back(tmp);
                        if((*(mqGameBoard.MainGameBoard[y_i][x_i])).GetColor()==getPreviousPlayer()){
                            break;
                        }
                        x_i+=dir1[i];
                        y_i+=dir1[i+1];
                    }
                }
            }
            if(it.first[2]=='B' || it.first[2]=='Q'){
                for(int i=0; i<4; i++){
                    int x_i = x+dir2[i];
                    int y_i = y+dir2[i+1];
                    while(y_i>=0 && y_i<8 && x_i>=0 && x_i<8 && mqGameBoard.MainGameBoard[y_i][x_i] == 0 || (*(mqGameBoard.MainGameBoard[y_i][x_i])).GetColor()==getPreviousPlayer()){
                        Move tmp = {it.first, x_i, y_i};
                        moves.push_back(tmp);
                        if((*(mqGameBoard.MainGameBoard[y_i][x_i])).GetColor()==getPreviousPlayer()){
                            break;
                        }
                        x_i+=dir2[i];
                        y_i+=dir2[i+1];
                    }
                }
            }
            if(it.first[2]=='N'){
                for(int i=0; i<4; i++){
                    int x_i = x+dir1[i];
                    int y_i = y+2*dir1[i+1];
                    if(y_i>=0 && y_i<8 && x_i>=0 && x_i<8 && mqGameBoard.MainGameBoard[y_i][x_i] == 0 || (*(mqGameBoard.MainGameBoard[y_i][x_i])).GetColor()==getPreviousPlayer()){
                        Move tmp = {it.first, x_i, y_i};
                        moves.push_back(tmp);
                    }
                    x_i = x+2*dir1[i];
                    y_i = y+dir1[i+1];
                    if(y_i>=0 && y_i<8 && x_i>=0 && x_i<8 && mqGameBoard.MainGameBoard[y_i][x_i] == 0 || (*(mqGameBoard.MainGameBoard[y_i][x_i])).GetColor()==getPreviousPlayer()){
                        Move tmp = {it.first, x_i, y_i};
                        moves.push_back(tmp);
                    }
                }
            }
        }
        return moves;
    }

    void move(string piece, int x, int y){
        //used for AI moves
        int iStartRow = mqGameBoard.trackerB[piece].y;
        int iStartCol = mqGameBoard.trackerB[piece].x;
        int iEndRow = y;
        int iEndCol = x;

        GamePiece* tmp = mqGameBoard.MainGameBoard[iStartRow][iStartCol];
        GamePiece* qpTemp = mqGameBoard.MainGameBoard[iEndRow][iEndCol];
        mqGameBoard.MainGameBoard[iEndRow][iEndCol] = mqGameBoard.MainGameBoard[iStartRow][iStartCol];
        mqGameBoard.MainGameBoard[iStartRow][iStartCol] = 0;
        string code = (*tmp).GetCode();

        //mqGameBoard.trackerB[code]={code, iEndCol, iEndRow};
        mqGameBoard.trackerW[code].piece=code;
        mqGameBoard.trackerW[code].x=iEndCol;
        mqGameBoard.trackerW[code].y=iEndRow;

        if(IsGameOver()){
            winner = mcPlayerTurn;
        }

        AlternateTurn();


    }

    void move(Move &move){
        this->move(move.piece, move.x, move.y);
    }

    char getWinner(){
        return winner;
    }

    char getPreviousPlayer(){
        return (mcPlayerTurn == 'W') ? 'B' : 'W';
    }

    char playRandomGame(){
        if (getWinner() != NONE)
            return getWinner();

        std::vector<Move> allLegalMoves = getAllLegalMoves();

        //std::shuffle(allLegalMoves.begin(), allLegalMoves.end(), g);
        std::random_shuffle(allLegalMoves.begin(), allLegalMoves.end());

        for (auto& move : allLegalMoves)
        {
            this->move(move);
            if (getWinner() != NONE)
                break;
        }

        return getWinner();
    }
};



//======================================================================================================================================================================================


class Node
{
private:
	GameState state;
	Node* parent = nullptr;
	std::vector<Move> allowedMoves;
	std::vector<std::unique_ptr<Node> > children;
	int played = 0;
	int wins = 0;

public:
	Node(GameState state){
        this->state = state;

	    allowedMoves = this->state.getAllLegalMoves();
    }
	Node(GameState state, Move move, Node* parent){
        this->state = state;
        this->parent = parent;

        this->state.move(move);

        if (this->state.getWinner() == NONE)
            allowedMoves = this->state.getAllLegalMoves();
    }
	bool isFullyExpanded(){
        return children.size() == allowedMoves.size();
    }
	bool isLeaf(){
        return children.empty();
    }
	bool hasParent(){
        return parent != nullptr;
    }
	double getUCT(){
        if (!played)
            return 0.0;

        return this->wins / (double)this->played + 1.42 * sqrt(log(parent->played) / this->played);

    }
	Node* getParent(){
        return parent;
    }
	Node* getBestChildren(){
        auto best = std::max_element(children.begin(), children.end(),
            [](const std::unique_ptr<Node>& left, const std::unique_ptr<Node>& right) {
            return left->getUCT() < right->getUCT();
        });

        return best->get();
    }
	Node* expand(){
        if (children.size() >= allowedMoves.size())
            return this; // It will be executed only if it's the end game node (no children)

        children.push_back(std::make_unique<Node>(state, allowedMoves[children.size()], this));
        return children.back().get();
    }
	char simulate(){
        if (state.getWinner() != NONE)
            return state.getWinner();

        GameState dummyState = state;
        return dummyState.playRandomGame();
    }
	void update(char winner){
        played += 10;

        if (winner == state.getPreviousPlayer())
            wins += 10;

        else if (winner == DRAW)
            wins += 5;
    }
	Move getMostVisitedMove(){
        auto best = std::max_element(children.begin(), children.end(),
            [](const std::unique_ptr<Node>& left, const std::unique_ptr<Node>& right) {
            return left->played < right->played;
        });

        int bestIndex = std::distance(children.begin(), best);

        return allowedMoves[bestIndex];
    }
	void debugChildren(){
        if (!isFullyExpanded())
        {
            std::cout << "This node isn't fully expanded" << std::endl;
            return;
        }

        for (int i = 0; i < children.size(); i++)
            std::cout << "Move: x: " << (int)allowedMoves[i].x << " y: " << (int)allowedMoves[i].y << " wins: " << children[i]->wins << " played: " << children[i]->played << std::endl;

    }
	int getRealPlayed(){
        return played / 10;
    }
	~Node(){

    }
};


class MonteCarloTreeSeach
{
private:
	Node* selection(){
        Node* currentNode = root.get();

        while (currentNode->isFullyExpanded() && !currentNode->isLeaf())
            currentNode = currentNode->getBestChildren();

        return currentNode;
    }
	Node* expansion(Node* node){
        return node->expand();
    }
	char simulation(Node* node){
        return node->simulate();
    }
	void backpropagation(Node* node, char winner){
        Node* currentNode = node;

        while (currentNode != nullptr)
        {
            currentNode->update(winner);
            currentNode = currentNode->getParent();
        }
    }
	std::unique_ptr<Node> root;
public:
	MonteCarloTreeSeach(){

    }
	Move findBestMoveFor(GameState* gameState){
        root = std::make_unique<Node>(*gameState);
        auto GetTickCount = []() -> unsigned long long
        {
            using namespace std::chrono;
            return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
        };

        int start = GetTickCount();

        while (GetTickCount() - start < 3 * 1000)
        {
            Node* bestNode = selection();
            Node* expanded = bestNode->expand();
            char simulationWinnder = expanded->simulate();
            backpropagation(expanded, simulationWinnder);
        }

        root->debugChildren();

        Move bestMove = root->getMostVisitedMove();
        std::cout << "Best move - x: " << (int)bestMove.x << " y: " << (int)bestMove.y << std::endl << std::endl << std::endl;
        std::cout << "Total played: " << root->getRealPlayed() << std::endl;

        return bestMove;
    }
	~MonteCarloTreeSeach(){

    }
};



//======================================================================================================================================================================================


MonteCarloTreeSeach mcts;
GameState startingState;
Move chosenMove;
std::mutex mutexLock;
std::condition_variable cv;
std::atomic<bool> gameInProgress{true};
std::atomic<bool> moveFound{false};
std::atomic<bool> working{false};
bool hasData = false;
//sf::Text endGameText;
//put in class?

void thinkMove()
{
	while (gameInProgress)
	{
		std::unique_lock<std::mutex> ul(mutexLock);
		cv.wait(ul, [] {return hasData || !gameInProgress; });

		if (!gameInProgress)
			break;

		hasData = false;

		working = true;
		chosenMove = mcts.findBestMoveFor(&startingState);
		working = false;

		moveFound = true;
	}
}

void startMove(GameState& gameState)
{
	startingState = gameState;
	hasData = true;
	cv.notify_one();
}

int main(){
    std::thread moveThread(thinkMove);
	GameState gameState;

    while (true)
	{
        // get user input
        /*
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::MouseButtonReleased && !working)
			{
				int x = event.mouseButton.x;
				int y = event.mouseButton.y;

				std::cout << x << " " << y << " " << (x - OFFSET) / SIZE_BTN << " " << (y - OFFSET) / SIZE_BTN << std::endl;

				int logicX = (x - OFFSET) / SIZE_BTN;
				int logicY = (y - OFFSET) / SIZE_BTN;

				if (gameState.canMoveHere(logicX, logicY))
				{
					gameState.move(logicX, logicY);
                    // get Ai's moves
					if (!checkForEndOfGame(gameState))
					{
						startMove(gameState);
					}
				}
			}
		}
        */
       system ("clear");
       startingState.mqGameBoard.Print();

        gameState.GetNextMove();
        if (!gameState.IsGameOver())
        {
            startMove(gameState);
        }

		if (moveFound)
		{
			moveFound = false;
			gameState.move(chosenMove);
			gameState.IsGameOver();

			//lastMoveRect.setPosition(sf::Vector2f(OFFSET + SIZE_BTN * move.x, OFFSET + SIZE_BTN * move.y));
			//drawLastMoveRect = true;
		}

		//system ("clear");
		//startingState.mqGameBoard.Print();


		if (working)

		if (!gameInProgress)
			cout<<"================Game Over================";
            break;

	}

	gameInProgress = false;
	cv.notify_one();
	moveThread.join();
	return 0;
}
/*
int main() {
    ChessBoard qGame;
    qGame.Start();
    return 0;
}
*/