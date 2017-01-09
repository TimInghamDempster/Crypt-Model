
struct FlatGrid
{
	std::vector<std::vector<CryptBox> > m_columns;
	const int m_numColumns;
	const int m_numRows;
	const double m_height;
	const double m_width;
	const double m_boxHeight;
	const double m_boxWidth;
	NormalDistributionRNG m_growthFactorRNG;
	
	FlatGrid(int numRows, int numColumns, int rootExpectedNumberOfCryptsInBox, double boxHeight, double boxWidth)
		:
		m_numColumns(numColumns),
		m_numRows(numRows),
		m_height(boxHeight * numColumns),
		m_width(boxWidth * numRows),
		m_boxHeight(boxHeight),
		m_boxWidth(boxWidth),
		m_growthFactorRNG(-0.0, 0.001)
	{
		float stepWidth = boxWidth / rootExpectedNumberOfCryptsInBox;
		float stepHeight = boxHeight / rootExpectedNumberOfCryptsInBox;

		for(int col = 0; col < m_numColumns; col++)
		{
			std::vector<CryptBox> column;
			m_columns.push_back(column);
			for(int row = 0; row < m_numRows; row++)
			{
				CryptBox box(rootExpectedNumberOfCryptsInBox * rootExpectedNumberOfCryptsInBox, m_growthFactorRNG);
				m_columns[col].push_back(box);
				for(int y = 0; y < rootExpectedNumberOfCryptsInBox; y++)
				{
					for(int x = 0; x < rootExpectedNumberOfCryptsInBox; x++)
					{
						Vector2D pos;
						pos.x = row * boxWidth + x * stepWidth;
						pos.y = col * boxHeight + y * stepHeight;
						m_columns[col][row].AddCrypt(pos, 0.5, m_growthFactorRNG.Next(), 0);
					}
				}
			}
		}
		for(int colId = 0; colId < m_numColumns; colId++)
		{
			for(int rowId = 0; rowId < m_numRows; rowId++)
			{
				CryptBox& box = m_columns[colId][rowId];

				if(rowId < m_numRows - 1)
				{
					box.m_collisionBoxes.push_back(&m_columns[colId][rowId + 1]);
					if(colId < m_numColumns - 1)
					{
						box.m_collisionBoxes.push_back(&m_columns[colId + 1][rowId + 1]);
					}
				}
				if(colId < m_numColumns - 1)
				{
					box.m_collisionBoxes.push_back(&m_columns[colId + 1][rowId]);
				}
				if(rowId > 0 && colId < m_numColumns - 1)
				{
					box.m_collisionBoxes.push_back(&m_columns[colId + 1][rowId - 1]);
				}
			}
		}
	}

	void CleanUp()
	{
		for(int col = 0; col < m_numColumns; col++)
		{
			for(int row = 0; row < m_numRows; row++)
			{
				m_columns[col][row].CleanUp();
			}
		}
	}

	CryptBox* FindBox(Vector2D position)
	{
		int row = (int)(position.x / m_boxWidth);
		int column = (int)(position.y / m_boxHeight);

		row = row < m_numRows ? row : m_numRows - 1;
		row = row >= 0 ? row : 0;

		column = column < m_numColumns ? column : m_numColumns - 1;
		column = column >= 0 ? column : 0;

		return &m_columns[column][row]; // Garunteed safe because the column vector (and grid vector) never change.
	}

	void AssignCryptsToBoxes()
	{
		for(int colId = 0; colId < m_numColumns; colId++)
		{
			for(int rowId = 0; rowId < m_numRows; rowId++)
			{
				CryptBox& box = m_columns[colId][rowId];
				for(int cryptId = 0; cryptId < box.m_positions->size(); cryptId++)
				{
					CryptBox* newBox = FindBox((*box.m_positions)[cryptId]);
					if(newBox != &box)
					{
						newBox->AddCrypt((*box.m_positions)[cryptId], box.m_lengths[cryptId], box.m_growthFactors[cryptId], box.m_mutated[cryptId]);
						box.RemoveCrypt(cryptId);
					}
				}
			}
		}
	}

	void Step(int framecount)
	{
		if(framecount % 100 == 0)
		{
			AssignCryptsToBoxes();

			for(int colId = 0; colId < m_numColumns; colId++)
			{
				for(int rowId = 0; rowId < m_numRows; rowId++)
				{
					m_columns[colId][rowId].RemoveDeadCrypts();
					m_columns[colId][rowId].UpdateCollisions();
				}
			}
		}
		if(framecount == 1000)
		{
			m_columns[50][50].Mutate();
		}
		for(int colId = 0; colId < m_numColumns; colId++)
		{
			for(int rowId = 0; rowId < m_numRows; rowId++)
			{
				CryptBox& box = m_columns[colId][rowId];
				box.UpdateCrypts();
			}
		}
		for(int colId = 0; colId < m_numColumns; colId++)
		{
			for(int rowId = 0; rowId < m_numRows; rowId++)
			{
				CryptBox& box = m_columns[colId][rowId];
				box.ClonePositions();
			}
		}
		for(int colId = 0; colId < m_numColumns; colId++)
		{
			for(int rowId = 0; rowId < m_numRows; rowId++)
			{
				CryptBox& box = m_columns[colId][rowId];

				box.UpdatePositions();
				box.SwapBuffers();
			}
		}
	}
};