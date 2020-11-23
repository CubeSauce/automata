#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>


enum cell_type
{
    CT_BLACK = 0,
    CT_WHITE = 1,
};

struct cell
{
    cell_type type;
    struct pawn* current_pawn = nullptr;
};

enum pawn_type
{
    PT_ROOK     = 'R',
    PT_KNIGHT   = 'N',
    PT_BISHOP   = 'B',
    PT_QUEEN    = 'Q',
    PT_KING     = 'K',
    PT_PAWN     = 'P',
};

struct pawn
{
    pawn_type type;
    bool is_black = false;

    bool can_move(int fx, int fy, int tx, int ty)
    {
    }
};

enum game_type
{
    GT_CLASSIC_CHESS,
};

namespace classic_chess
{
    static const char pieces_setup[8][8] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {'/', '/', '/', '/', '/', '/', '/', '/'},
        {'/', '/', '/', '/', '/', '/', '/', '/'},
        {'/', '/', '/', '/', '/', '/', '/', '/'},
        {'/', '/', '/', '/', '/', '/', '/', '/'},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},
    };
    
    static const char board_setup[8][8] = {
        {'B', 'W', 'B', 'W', 'B', 'W', 'B', 'W'},
        {'W', 'B', 'W', 'B', 'W', 'B', 'W', 'B'},
        {'B', 'W', 'B', 'W', 'B', 'W', 'B', 'W'},
        {'W', 'B', 'W', 'B', 'W', 'B', 'W', 'B'},
        {'B', 'W', 'B', 'W', 'B', 'W', 'B', 'W'},
        {'W', 'B', 'W', 'B', 'W', 'B', 'W', 'B'},
        {'B', 'W', 'B', 'W', 'B', 'W', 'B', 'W'},
        {'W', 'B', 'W', 'B', 'W', 'B', 'W', 'B'},
    };
};

struct field
{
    game_type type;
    int size[2] = { 0, 0 };
    cell** cells = nullptr;

    int next_in_grave = 0;
    pawn* graveyard[64] = {nullptr};

    bool last_move_was_black = true;

    field(game_type gt)
        :type(gt)
    {
        if(type == GT_CLASSIC_CHESS)
        {
            size[0] = 8;
            size[1] = 8;

            cells = new cell*[size[1]];
            for(auto cy = 0; cy < size[1]; cy++)
            {
                cells[cy] = new cell[size[0]];
                for(auto cx = 0; cx < size[0]; cx++)
                {
                    const char ct = classic_chess::board_setup[cy][cx];
                    cells[cy][cx].type = (ct == 'W') ? CT_WHITE : CT_BLACK;
                    
                    char pt = classic_chess::pieces_setup[size[1] - cy - 1][cx];
                    if(pt != '/')
                    {
                        bool is_black = false;
                        if(pt > 'a' && pt < 'z')
                        {
                            //black
                            is_black = true;
                            int dt = 'a' - 'A';
                            pt -= dt;
                        }

                        cells[cy][cx].current_pawn = new pawn();
                        cells[cy][cx].current_pawn->type = (pawn_type) pt;
                        cells[cy][cx].current_pawn->is_black = is_black;
                    }
                }
            }
        }
    }

    bool move(const char* cmd)
    {
        char from_a = -1, from_b = -1, to_a = -1, to_b = -1;
        int cmds = strlen(cmd);
        
        
        if(cmds == 7)
        {
            //should be castles oops
            return castles();
        }
        else if(cmds == 5)
        {
            // "e4-d5"
            from_a = tolower(cmd[0]);
            from_b = cmd[1] - '0';
            
            to_a = tolower(cmd[3]);
            to_b = cmd[4] - '0';
        }
        else if(cmds == 4)
        {
            // "e4-d5"
            from_a = tolower(cmd[0]);
            from_b = cmd[1] - '0';
            
            to_a = tolower(cmd[2]);
            to_b = cmd[3] - '0';
        }
        else
        {
            return false;
        }
        
        return move(from_a, from_b, to_a, to_b);
    }

    bool castles()
    {
        bool can_move = false;

        if(last_move_was_black)
        {//white
            last_move_was_black ^= 1;
            pawn* rook  = cells[0][7].current_pawn; //Pawn To Move
            pawn* bishop = cells[0][5].current_pawn; 
            pawn* knight = cells[0][6].current_pawn; 
            pawn* king  = cells[0][4].current_pawn; //Pawn To Attack

            if(knight == nullptr && bishop == nullptr && king && king->type == PT_KING && rook && rook->type == PT_ROOK)
            {
                cells[0][4].current_pawn = nullptr;
                cells[0][5].current_pawn = rook;
                cells[0][6].current_pawn = king;
                cells[0][7].current_pawn = nullptr;
                printf("castles");
                return true;
            }
        }
        else
        {
            last_move_was_black ^= 1;
            pawn* king = cells[7][4].current_pawn; //Pawn To Move
            pawn* bishop = cells[7][5].current_pawn; 
            pawn* knight = cells[7][6].current_pawn; 
            pawn* rook = cells[7][7].current_pawn; //Pawn To Attack

            if(knight == nullptr && bishop == nullptr && king && king->type == PT_KING && rook && rook->type == PT_ROOK)
            {
                cells[7][4].current_pawn = nullptr;
                cells[7][5].current_pawn = rook;
                cells[7][6].current_pawn = king;
                cells[7][7].current_pawn = nullptr;
                printf("castles");
                return true;
            }
        }

        return false;
    }

    bool move(char from_a, int from_b, char to_a, int to_b)
    {
        int a_offset1 = 'A';
        int a_offset2 = 'A';
        if(from_a < 'A' || to_a < 'A' || from_b < 0 || from_b > size[1] || to_b < 0 || to_b > size[1])
        {
            return false;
        }

        if(from_a >= 'a' || from_a <= 'z')
        {
            a_offset1 = 'a';
        }
        else
        {
            return false;
        }

        if(to_a >= 'a' || to_a <= 'z')
        {
            a_offset2 = 'a';
        }
        else
        {
            return false;
        }
        
        int fx = from_a - a_offset1;
        if(fx < 0 || fx > size[0])
        {
            return false;
        }
        int fy = from_b - 1;
        
        int tx = to_a - a_offset2;
        if(tx < 0 || tx > size[0])
        {
            return false;
        }
        int ty = to_b - 1;

        //printf("%c%d-%c%d\n", from_a, from_b, to_a, to_b);
        pawn* ptm = cells[fy][fx].current_pawn; //Pawn To Move
        pawn* pta = cells[ty][tx].current_pawn; //Pawn To Attack

        if(ptm == nullptr && (ptm->is_black && !last_move_was_black))
        {
            return false;
        }

        last_move_was_black ^= 1;

        int dx = tx - fx;
        int dy = ty - fy;
        int adx = abs(dx);
        int ady = abs(dy);

        bool can_move = false;
        switch(ptm->type)
        {
            case PT_KING:
            {
                can_move = adx == 1 && ady == 1;
                break;                
            }
            case PT_QUEEN:
            {
                can_move = (dx == 0 || dy == 0 || dx == dy);
                break;
            }
            case PT_BISHOP:
            {
                can_move = adx == ady;
                break;
            }
            case PT_KNIGHT:
            {
                can_move = (adx == 2 && ady == 1) || (adx == 1 && ady == 2);
                break;
            }
            case PT_ROOK:
            {
                can_move = dx == 0 || dy == 0;
                break;
            }
            case PT_PAWN:
            {
                if(pta == nullptr)
                {
                    can_move = dx == 0 && ady > 0 && ady < 3;
                }
                else
                {
                    if(ptm->is_black)
                    {
                        can_move = dy < 0 && adx == ady && adx == 1;
                    }
                    else
                    {
                        can_move = dy > 0 && adx == ady && adx == 1;                        
                    }
                }
                break;
            }
        }

        if(can_move)
        {
            printf("%c%d-%c%d\n", from_a, from_b, to_a, to_b);

            if(pta)
            {
                graveyard[next_in_grave++] = pta;
            }

            cells[fy][fx].current_pawn = nullptr;
            cells[ty][tx].current_pawn = ptm;   //Stomping over pta, if it exists we stored it in the graveyard

            return true;
        }

        return false;
    }

    void print()
    {   
        int dt = 'a' - 'A';
        
        printf("\n   ");
        for(int cx = 0; cx < size[0]; ++cx)
        {
           printf(" %c ", 'a' + cx);
        }

        printf("\n");
        for(int cy = size[1] - 1; cy >= 0; --cy)
        {
            printf(" %d ", cy + 1);
            for(int cx = 0; cx < size[0]; ++cx)
            {
                if(pawn* p = cells[cy][cx].current_pawn)
                {
                    if(p->is_black)
                    {
                        printf("[%c]", p->type + dt);
                    }
                    else
                    {
                        printf("[%c]", p->type);
                    }                    
                }
                else
                {
                    printf("[%c]", cells[cy][cx].type ? 219 : 177);
                }
            }

            printf(" %d \n", cy + 1);
        }

        printf("   ");
        for(int cx = 0; cx < size[0]; ++cx)
        {
           printf(" %c ", 'a' + cx);
        }
    }
};

int main(int argc, char* argv[])
{
    srand(0);
    field f(GT_CLASSIC_CHESS);
    f.print();

    printf("%d\n", argc);
    return 0;

    const char* moves[] = {
        "c2c4", "g8f6",
        "d2d4", "e7e6",
        "g1f3", "b7b6",
        "a2a3", "c8b7",
        "c1f4", "f8e7",
        "B1c3", "castles",
        "A1c1", "B8a6",
        "H2h3", "D7d5",
        "E2e3", "C7c5",
        "C3b5", "C5d4",
        "E3d4", "D5c4",
        "F1c4", "F6d5",
        "F4e5", "A6c7",
        "B5c7", "D5c7",
        "castles", "A8c8",
        "D1e2", "H7h6",
        "C4d3", "E7f6",
        "F1e1", "F6e5",
        "E2e5", "C7d5",
        "C1c8"
    };

    for(int i = 0; i < 39; ++i)
    {
        if(f.move(moves[i]))
        {
            f.print();
        }
        else
        {
            break;
        }
    }

    return 0;
}