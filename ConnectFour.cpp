#include <iostream>
#include <optional>
#include <limits>
#include <vector>
#include <random>



// ANSI escape codes for text color
#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_RESET    "\x1b[0m"
#define ANSI_HIGHLIGHT      "\033[7m"
#define ANSI_UNDO_HIGHLIGHT "\033[0m"

namespace ConnectFour
{
  // this exception stops the main game loop
  class GameOverException : public std::exception
  {
  public:
    GameOverException() : std::exception() {}
    GameOverException(std::exception& Other) : std::exception(Other) {}
    GameOverException(const char* const Message) : std::exception(Message) {}
  };

  class Board
  {
  public:
    enum class SpaceState
    {
      Empty,
      Player1,
      Player2,
    };

    enum class MoveType
    {
      Player1,
      Player2,
    };

    /// <summary>
    /// this data structure holds the most recent move
    /// </summary>
    struct LastMove_t
    {
      unsigned int x;
      unsigned int y;
      bool isInitialized;   // indicates whether this has been set at least once
    } LastMove;

    static const unsigned int Width = 7;
    static const unsigned int Height = 6;

    Board() : LastMove{ 0, 0, false }, board(Height, std::vector<SpaceState>(Width, SpaceState::Empty))
    {
      
    }

    /// <summary>
    /// copy operator
    /// </summary>
    /// <param name="original">the instance of the original board to be copied</param>
    void operator= (const Board& original)
    {
      board = original.board;
    }

    /// <summary>
    /// this function returns an enum indicating the state of the given position
    /// on the board
    /// </summary>
    /// <param name="Row">0-based index of the row</param>
    /// <param name="Column">0-based index of the column</param>
    /// <returns>an enum indicating the state of that position</returns>
    SpaceState GetSpace(unsigned int Row, unsigned int Column) const
    {
      if (Row >= Height)
        throw std::exception{ "Row out of range" };
      if (Column >= Width)
        throw std::exception{ "Column out of range" };

      return board[Row][Column];
    }

    /// <summary>
    /// this function returns a value indicating how many pieces have been
    /// inserted into this column.
    /// </summary>
    /// <param name="Column">0-based index of the column</param>
    /// <returns>absolute number of pieces existing in the given column</returns>
    unsigned int GetColumnHeight(unsigned int Column) const
    {
      if (Column >= Width)
        throw std::exception{ "Column out of range" };

      for (int i = Height - 1; i >= 0; --i) {
        if (board[i][Column] == SpaceState::Empty) {
          return i;
        }
      }
      return Height; // Column is full
    }

    /// <summary>
    /// check to see if it's possible to make a move to place a token in the given column
    /// </summary>
    /// <param name="Column">0-based index of the column</param>
    /// <returns>true if the move is possible, false otherwise</returns>
    bool CanMakeMove(unsigned int Column) const
    {
      return GetColumnHeight(Column) < Height;
    }

    /// <summary>
    /// if possible, put a token in the given column and adjust the board state accordingly
    /// </summary>
    /// <param name="Move"></param>
    /// <param name="Column"></param>
    void MakeMove(MoveType Move, unsigned int Column)
    {
      if (!CanMakeMove(Column))
        throw std::exception{ "Cannot make move" };

      auto CurrentHeight = GetColumnHeight(Column);
      SetSpace(CurrentHeight, Column, ConvertMoveToSpaceState(Move));
    }

    // output the board to the screen
    void PrintBoard() const
    {
      auto GetSpaceStateCharacter = [](SpaceState s)
      {
        switch (s)
        {
        case SpaceState::Empty:
        default:
          return ". "; // Changed to '.' for better visibility
        case SpaceState::Player1:
          // this creates a string like "<set red>" "x " "<reset color>"
          return ANSI_COLOR_RED "X " ANSI_COLOR_RESET;
        case SpaceState::Player2:
          return "O ";
        }
      };

      // let's clear the screen
      printf("\033[2J"); // Clear the entire screen
      printf("\033[H"); // Reset cursor position to the top-left
      fflush(stdout);  // Important: Flush the output buffer

      for (unsigned int i = 0; i < Height ; i++) // Iterate from bottom to top
      {
        for (unsigned int j = 0; j < Width; j++)
        {
          // set the highlight if this location is the most recent move
          if (IsLastMove(i, j))
          {
            std::cout << ANSI_HIGHLIGHT;
          }
          std::cout << GetSpaceStateCharacter(GetSpace(i, j));
          if (IsLastMove(i, j))
          {
            std::cout << ANSI_UNDO_HIGHLIGHT;
          }
        }
        std::cout << std::endl << std::endl;
      }

      for (unsigned int i = 0; i < Width; i++)
      {
        std::cout << i + 1 << " ";
      }

      std::cout << "\n";

      // make a flowerbox to help separate boards
      for (unsigned int i = 0; i < Width; i++)
      {
        std::cout << "**";
      }

      std::cout << "\n";
      
    }

    /// <summary>
    /// check to see if there is a winning state
    /// </summary>
    /// <param name="player">a SpaceState enum indicating player</param>
    /// <returns>true if this is a winning condition, false otherwise</returns>
    bool CheckWin(SpaceState player) const {
      // Check horizontal
      for (int r = 0; r < Height; r++) {
        for (int c = 0; c <= Width - 4; c++) {
          if (board[r][c] == player && 
            board[r][c + 1] == player && 
            board[r][c + 2] == player && 
            board[r][c + 3] == player) {
            return true;
          }
        }
      }

      // Check vertical
      for (int c = 0; c < Width; c++) {
        for (int r = 0; r <= Height - 4; r++) {
          if (board[r][c] == player && 
            board[r + 1][c] == player &&
            board[r + 2][c] == player &&
            board[r + 3][c] == player) {
            return true;
          }
        }
      }

      // Check diagonals (top-left to bottom-right)
      for (int r = 0; r <= Height - 4; r++) {
        for (int c = 0; c <= Width - 4; c++) {
          if (board[r][c] == player &&
            board[r + 1][c + 1] == player &&
            board[r + 2][c + 2] == player &&
            board[r + 3][c + 3] == player) {
            return true;
          }
        }
      }

      // Check diagonals (bottom-left to top-right)
      for (int r = 3; r < Height; r++) {
        for (int c = 0; c <= Width - 4; c++) {
          if (board[r][c] == player &&
            board[r - 1][c + 1] == player &&
            board[r - 2][c + 2] == player &&
            board[r - 3][c + 3] == player) {
            return true;
          }
        }
      }

      return false;
    }

    static SpaceState ConvertMoveToSpaceState(MoveType Move)
    {
      if (Move == MoveType::Player1)
        return SpaceState::Player1;
      else
        return SpaceState::Player2;
    }

  private:
    // put a token in this place on the board
    void SetSpace(unsigned int Row, unsigned int Column, SpaceState NewState)
    {
      if (Row >= Height)
        throw std::exception{ "Row out of range" };
      if (Column >= Width)
        throw std::exception{ "Column out of range" };

      LastMove.x = Column;
      LastMove.y = Row;
      LastMove.isInitialized = true;

      board[Row][Column] = NewState;
    }

    /// <summary>
    /// this checks to see if the given row and column for the board
    /// are the coordinates of the last move played
    /// </summary>
    /// <param name="Row"></param>
    /// <param name="Column"></param>
    /// <returns></returns>
    bool IsLastMove(unsigned int Row, unsigned int Column) const
    {
      return (LastMove.isInitialized && LastMove.x == Column &&
        LastMove.y == Row);
    }
    

    std::vector<std::vector<SpaceState>> board;
  };
}

/// <summary>
/// this static method handles input of a column for the player to move
/// </summary>
/// <returns>an optional type with the column</returns>
static std::optional<unsigned int> GetRequestedColumn()
{
  unsigned int RequestedColumn = 0;

  std::cout << std::endl << "Enter a column between 1 and " << ConnectFour::Board::Width << ".  ";

  std::cin >> RequestedColumn;

  if (std::cin.eof() || std::cin.bad())
  {
    return std::nullopt;
  }
  else if (std::cin.fail() || RequestedColumn < 1 || RequestedColumn > ConnectFour::Board::Width) // Check for valid input range
  {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Invalid input. Please enter a column number between 1 and " << ConnectFour::Board::Width << ".\n"; // Inform the user
    return std::nullopt;
  }

  return RequestedColumn - 1; // Adjust for 0-based indexing
}

int main()
{
  ConnectFour::Board b;

  srand((unsigned int)std::time(0));

  // there may be nested places where a game over condition occurs, so throw an exception
  // to stop the loop
  while (true) {
    
    ConnectFour::Board::MoveType currentPlayer = ConnectFour::Board::MoveType::Player1;

    try
    {
      // this is the main game loop

      b.PrintBoard();

      while (true) { // Loop until valid input
        auto Column = GetRequestedColumn();
        if (Column.has_value()) {
          try {
            b.MakeMove(currentPlayer, *Column);
            break; // Exit input loop if valid move is made
          }
          catch (std::exception& e) {
            std::cout << e.what() << std::endl;

          }
        }
      }

      if (b.CheckWin(ConnectFour::Board::ConvertMoveToSpaceState(currentPlayer))) {
        b.PrintBoard();
        std::cout << (currentPlayer == ConnectFour::Board::MoveType::Player1 ? "Player 1" : "Player 2") << " wins!\n";
        throw ConnectFour::GameOverException();
      }
      else {
        // Switch players
        currentPlayer = (currentPlayer == ConnectFour::Board::MoveType::Player1) ?
          ConnectFour::Board::MoveType::Player2 : ConnectFour::Board::MoveType::Player1;

        // Check for a draw
        bool isDraw = true;
        for (int c = 0; c < b.Width; ++c) {
          if (b.CanMakeMove(c)) {
            isDraw = false;
            break;
          }
        }
        if (isDraw) {
          b.PrintBoard();
          std::cout << "It's a draw!" << std::endl;
          throw ConnectFour::GameOverException();
        }
        // if it's the computer's turn, then do some extra logic
        else if (currentPlayer == ConnectFour::Board::MoveType::Player2)
        {
          int autoMove = 1 + (rand() % b.Width);

          b.PrintBoard();

          // iterate through the possible board plays, looking for either a winning play,
          // or a blocking play, or a random play if neither of those are available.  To
          // make the game more difficult, this could look to see if a move isn't likely
          // to cause a win..

          // look for winning play
          for (int i = 0; i < b.Width; i++)
          {
            // create a copy of the board
            ConnectFour::Board boardWin;
            boardWin = b;

            // make a move, if possible, and then check for a win
            if (boardWin.CanMakeMove(i))
            {
              boardWin.MakeMove(ConnectFour::Board::MoveType::Player2, i);

              // if it's a win, do the win logic
              if (boardWin.CheckWin(ConnectFour::Board::SpaceState::Player2))
              {
                b.MakeMove(ConnectFour::Board::MoveType::Player2, i);
                b.PrintBoard();
                std::cout << "Player 2 wins!" << std::endl;
                throw ConnectFour::GameOverException();
              }
            }
          } // end for loop

          // look for a blocking play
          bool bFoundBlock = false;
          for (int i = 0; i < b.Width; i++)
          {
            // create a copy of the board
            ConnectFour::Board boardWin;
            boardWin = b;

            // make a move, if possible, and then check for a win by player 1
            if (boardWin.CanMakeMove(i))
            {
              boardWin.MakeMove(ConnectFour::Board::MoveType::Player1, i);
              if (boardWin.CheckWin(ConnectFour::Board::SpaceState::Player1))
              {
                // let's block player 1
                b.MakeMove(ConnectFour::Board::MoveType::Player2, i);
                bFoundBlock = true;

                // if this move will win the game, do the win
                if (b.CheckWin(ConnectFour::Board::SpaceState::Player2))
                {
                  b.PrintBoard();
                  std::cout << "Player 2 wins!" << std::endl;
                  throw ConnectFour::GameOverException();
                }
                break;
              }
            }
          } // end for loop

          // if a blocking move was not found, then make a random move
          if (!bFoundBlock)
          {
            // skip columns till one is available.  since it's not a draw, there must
            // be an available column
            while (!b.CanMakeMove(autoMove - 1))
            {
              autoMove++;
              if (autoMove > b.Width)
              {
                autoMove = 1;   // autoMove is 1-indexed
              }
            }

            b.MakeMove(ConnectFour::Board::MoveType::Player2, autoMove - 1);
          }

          if (b.CheckWin(ConnectFour::Board::ConvertMoveToSpaceState(ConnectFour::Board::MoveType::Player2))) {
            b.PrintBoard();
            std::cout << "Player 2 wins!" << std::endl;
            throw ConnectFour::GameOverException();
          }

          // switch player back to player 1
          currentPlayer = ConnectFour::Board::MoveType::Player1;
        }

      } // end player 2 branch
    } // end while loop

    catch (ConnectFour::GameOverException exc)
    {
      char temp;

      std::cin >> temp;

      // reset the board
      ConnectFour::Board newBoard;
      b = newBoard;

      // all done so beep 4 times for losers
      if (b.CheckWin(ConnectFour::Board::ConvertMoveToSpaceState(ConnectFour::Board::MoveType::Player2))) {
        printf("\a\a\a\a"); // The '\a' character is the "alert" or beep character.
        fflush(stdout);   // don't delay the beep
      }
    }
    catch (std::exception exs)
    {
      printf(exs.what());
    }
  }
  // intentionally allowing other exceptions to end game

  

  return 0;
}