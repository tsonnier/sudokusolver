#include "GameBoard.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <vector>

GameBoard::GameBoard()
{

	Binit();
}

GameBoard GameBoard::operator= (const GameBoard& gb)
{
	GameBoard A;
	unsigned int i;

	for(i = 0;i < 9;i++)
	{
		A.m_GameRows[i] = gb.m_GameRows[i];
		A.m_Cols[i] = gb.m_Cols[i];
		A.m_Rows[i] = gb.m_Rows[i];
		A.m_Sectors[i] = gb.m_Sectors[i];
	}

	return A;
}

void GameBoard::Copy(GameBoard& gb)
{
	unsigned int i,j,k;

	for(i = 0;i < 9;i++)
	{
	    for(j=0;j<9;j++)
        {
            if(gb.m_GameRows[i].Gettaken(j))
                this->m_GameRows[i].Settaken(j);
            this->m_GameRows[i].m_square[j].SetVal(gb.m_GameRows[i].m_square[j].GetVal());
            this->m_GameRows[i].m_square[j].SetShown(gb.m_GameRows[i].m_square[j].GetShown());
            this->m_GameRows[i].m_square[j].SetCol(gb.m_GameRows[i].m_square[j].GetCol());
            this->m_GameRows[i].m_square[j].SetRow(gb.m_GameRows[i].m_square[j].GetRow());
            this->m_GameRows[i].m_square[j].SetSector(gb.m_GameRows[i].m_square[j].GetSector());
            for(k=1;k<=9;k++)
                if(gb.m_GameRows[i].m_square[j].GetPossibles(k))
                    this->m_GameRows[i].m_square[j].SetPossibles(k);
        }


		this->m_Cols[i] = gb.m_Cols[i];
		this->m_Rows[i] = gb.m_Rows[i];
		this->m_Sectors[i] = gb.m_Sectors[i];
	}

}

GameBoard::~GameBoard()
{
    //dtor
}

void GameBoard::Binit()
{
	for(int row=0;row<9;row++)
    {
		m_GameRows[row].Rinit();

		m_Rows[row].clear();
		m_Sectors[row].clear();
		m_Cols[row].clear();
		m_GameRows[row].SetCol();
        for(int col=0;col<9;col++)
        {
            // todo - set sector for each square
            m_GameRows[row].m_square[col].SetSector((row/3) * 3 + (col/3));
            m_GameRows[row].m_square[col].SetRow(row);
        }
    }
}

bool GameBoard::SetSquare(unsigned int val, int row, int col)
{
	int sec, undoNum;
	bool colFnd, rowFnd, secFnd;
	GameSquare * sq;
	sq = &m_GameRows[row].m_square[col];
	sec = sq->GetSector();

	if(val == 0)
	{
		undoNum = sq->GetVal();
		sq->SetVal(val);
		m_GameRows[row].Unsettaken(undoNum);
		m_Rows[row].erase(undoNum);
		m_Cols[col].erase(undoNum);
		m_Sectors[sec].erase(undoNum);
		return true;
	}


	rowFnd = (m_Rows[row].find(val) == m_Rows[row].end());
	colFnd = (m_Cols[col].find(val) == m_Cols[col].end());
	secFnd = (m_Sectors[sec].find(val) == m_Sectors[sec].end());
	if (rowFnd &&
		colFnd &&
		secFnd &&
		sq->GetVal() == 0 &&
		val > 0 &&
		val < 10)
	{
		sq->SetVal(val);
		m_GameRows[row].Settaken(val);
		m_Rows[row].insert(val);
		m_Cols[col].insert(val);
		m_Sectors[sec].insert(val);
		return true;
	}
	else
	{
		return false;
	}
}

void GameBoard::RemovePossibles(GameSquare * sq)
{
	int row, col, sec;
	row = sq->GetRow();
	col = sq->GetCol();
	sec = sq->GetSector();
	for(int i=1;i<=9;i++)
	{
		sq->SetPossibles(i);
		if(m_Rows[row].find(i) != m_Rows[row].end())
            sq->RemovePossibles(i);
        if(m_Cols[col].find(i) != m_Cols[col].end())
            sq->RemovePossibles(i);
		if((m_Sectors[sec].find(i) != m_Sectors[sec].end()))
			sq->RemovePossibles(i);
	}


}

bool GameBoard::GenBoard(int row, int col)
{
	unsigned int k, num, random;
	std::vector<unsigned int> selectList (9);
	wxString lst, error;
	bool setSucc, nextSucc;

	srand(time(NULL));

	if(col == 9)
	{
		col = 0;
		row++;
	}
	if(row == 9)
		return true;



	RemovePossibles(&(m_GameRows[row].m_square[col]));  // Possible problem
	selectList.clear();
	for(k=1;k<=9;k++)
		if(m_GameRows[row].m_square[col].GetPossibles(k))
			selectList.push_back(k);
	random_shuffle(selectList.begin(), selectList.end());

	// Shuffle things around more
	if(selectList.size() > 2)
		random = rand() % selectList.size();

	for(k=0;k<random && selectList.size() > 2;k++)
	{
		num = selectList.back();
		selectList.pop_back();
		selectList.insert(selectList.begin(), num);
	}

	lst.clear();
	for(k=0;k<selectList.size();k++)
	{
		lst << selectList[k] << _(" ");
	}
	lst << selectList.size();
	//wxMessageBox(lst);

	if(selectList.size() == 0)
		return false;
	else
	{
		setSucc = false;
		nextSucc = false;
		while (!nextSucc)
		{
			while (!setSucc)
			{
				if(selectList.size() == 0)
				{
					return false;
				}

				num = selectList.back();
				selectList.pop_back();
				setSucc = SetSquare(num, row, col);
			}
			nextSucc = GenBoard(row, col + 1);
			if(!nextSucc)
			{
				SetSquare(0, row, col);
				setSucc = false;
			}
		}
		return true;
	}


}

bool GameBoard::Solvable()
{
	// Determines whether the puzzle is currently solvable

    bool solved;
    GameBoard * trying = new GameBoard();

    trying->Binit();
    trying->Copy(*this);
    solved = trying->Solve();
    delete trying;


	return solved;
}

void GameBoard::RemoveSquares()
{

	// Removes squares from the visible list until the minimum number of squares remains
	//   for the puzzle to remain solvable - in theory

	bool squareRemoved = true;

    squareRemoved = true;
    ResetCols();
    ResetRows();
    ResetSectors();
    while(squareRemoved)
    {
        squareRemoved = RemoveLayerEasy();
    }
}

bool GameBoard::RemoveLayerEasy()
{
    // Checks all squares and removes those that leave the board in a solvable state
    bool squareRemoved = false;
	unsigned int row, col, tempVal;
	GameSquare * sq;


    for(col=0;col<9;col++)
        for(row=0;row<9;row++)
        {
            sq = &m_GameRows[row].m_square[col];
            if (sq->GetVal() != 0)
            {
               	tempVal = sq->GetVal();
                SetSquare(0, row, col);
                if(Solvable())
                {
                    squareRemoved = true;
                    sq->SetShown(false);
                }
                else
                {
                    SetSquare(tempVal, row, col);
                }
            }
        }
    return squareRemoved;
}

bool GameBoard::Solve()
{
    bool solSq, unknSq;
    unsigned int row, col;
    unsigned int numPos;
    GameSquare * sq;

    solSq = true;
    unknSq = true;

    while(solSq && unknSq)  // We solved a square and there are still square to solve
    {
        solSq = false;
        unknSq = false;

        for(row=0;row<9;row++)
            for(col=0;col<9;col++)   // Check all squares
            {
                sq = &m_GameRows[row].m_square[col];
                if(sq->GetVal() == 0)  //Square is unknown
                {
                    unknSq = true;
                    RemovePossibles(sq);  // Figure out what is possible
                    numPos = sq->GetNumPossibles();
                    if(numPos == 1)   // Only one possible value
                    {
                        sq->SetVal(sq->GetOnlyPossible());// Set value to only possibility
                        solSq = true;
                    }
                    else   // More than one possible value
                        unknSq = true;
                }
            }
    }
    if(unknSq)
        return false;
    else
        return true;
}

void GameBoard::ResetRows()
{
    unsigned int row, col;
    GameSquare * sq;

    // Add all elements back
    for(row=0;row<9;row++)
        for(col=0;col<9;col++)
        {
            m_Rows[row].insert(col+1);
        }

    // Remove the values that are in each of the rows
    for(row=0;row<9;row++)
        for(col=0;col<9;col++)
        {
            sq = &m_GameRows[row].m_square[col];
            if(sq->GetVal() > 0)
                m_Rows[row].erase(sq->GetVal());
        }
}

void GameBoard::ResetCols()
{
    unsigned int row, col;
    GameSquare * sq;

    // Add all elements back
    for(row=0;row<9;row++)
        for(col=0;col<9;col++)
        {
            m_Cols[col].insert(row+1);
        }

    // Remove the values that are in each of the rows
    for(row=0;row<9;row++)
        for(col=0;col<9;col++)
        {
            sq = &m_GameRows[row].m_square[col];
            if(sq->GetVal() > 0)
                m_Cols[col].erase(sq->GetVal());
        }
}

void GameBoard::ResetSectors()
{
    unsigned int row, col, sec;
    GameSquare * sq;

    // Add all elements back
    for(row=0;row<9;row++)
        for(col=0;col<9;col++)
        {
            m_Sectors[row].insert(col+1);
        }

    // Remove the values that are in each of the rows
    for(row=0;row<9;row++)
        for(col=0;col<9;col++)
        {
            sq = &m_GameRows[row].m_square[col];
            sec = sq->GetSector();
            if(sq->GetVal() > 0)
                m_Sectors[sec].erase(sq->GetVal());
        }
}
