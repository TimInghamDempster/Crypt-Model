
struct FlatGrid
{
	std::vector<std::vector<CryptBox> > m_columns;
	const int m_numColumns;
	const int m_numRows;
	const double m_height;
	const double m_width;
	const double m_boxHeight;
	const double m_boxWidth;
	
	FlatGrid(int numRows, int numColumns, int rootExpectedNumberOfCryptsInBox, double boxHeight, double boxWidth)
		:
		m_numColumns(numColumns),
		m_numRows(numRows),
		m_height(boxHeight * numColumns),
		m_width(boxWidth * numRows),
		m_boxHeight(boxHeight),
		m_boxWidth(boxWidth)
	{
		float stepWidth = boxWidth / rootExpectedNumberOfCryptsInBox;
		float stepHeight = boxHeight / rootExpectedNumberOfCryptsInBox;

		for(int col = 0; col < m_numColumns; col++)
		{
			std::vector<CryptBox> column;
			m_columns.push_back(column);
			for(int row = 0; row < m_numRows; row++)
			{
				CryptBox box(rootExpectedNumberOfCryptsInBox * rootExpectedNumberOfCryptsInBox);
				m_columns[col].push_back(box);
				for(int y = 0; y < rootExpectedNumberOfCryptsInBox; y++)
				{
					for(int x = 0; x < rootExpectedNumberOfCryptsInBox; x++)
					{
						Vector2D pos;
						pos.x = row * boxWidth + x * stepWidth;
						pos.y = col * boxHeight + y * stepHeight;
						m_columns[col][row].AddCrypt(pos);
					}
				}
			}
		}

		/*for(int col = 0; col < m_numColumns; col++)
		{
			for(int row = 0; row < m_numRows; row++)
			{
				m_columns[col][row].m_potentialCollisionBoxes.push_back(&m_columns[col][row]);

				if(row == 0)
				{
					for(int innerCol = col + 1; innerCol < m_numColumns; innerCol++)
					{
						m_columns[col][row].m_potentialCollisionBoxes.push_back(&m_columns[innerCol][row]);
					}
				}

				if(row < m_numRows - 1)
				{
					m_columns[col][row].m_potentialCollisionBoxes.push_back(&m_columns[col][row + 1]);
				}

				
				if(col < m_numColumns - 1)
				{
						
					
					if(row < m_numRows - 1)
					{
						m_columns[col][row].m_potentialCollisionBoxes.push_back(&m_columns[col + 1][row + 1]);
					}

					if(row > 0)
					{
						m_columns[col][row].m_potentialCollisionBoxes.push_back(&m_columns[col + 1][row]);
						m_columns[col][row].m_potentialCollisionBoxes.push_back(&m_columns[col + 1][row - 1]);
					} 
				}
				else
				{
					
					if(row < m_numRows - 1)
					{
						m_columns[col][row].m_potentialCollisionBoxes.push_back(&m_columns[0][row + 1]);
					}

					if(row > 0)
					{
						m_columns[col][row].m_potentialCollisionBoxes.push_back(&m_columns[0][row]);
						m_columns[col][row].m_potentialCollisionBoxes.push_back(&m_columns[0][row - 1]);
					} 
				}
			}
			
		}*/
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

		return &m_columns[column][row]; // Garunteed safe because the column vector (and grid vector) never change.*/
	}

	void Step()
	{/*
		for(int colId = 0; colId < m_numColumns; colId++)
		{
			for(int rowId = 0; rowId < m_numRows; rowId++)
			{
				CellBox& box = m_columns[colId][rowId];

				for(int cellId = 0; cellId < (int)box.m_positions.size(); cellId++)
				{
					for(int collisionBoxId = 0; collisionBoxId < (int)box.m_potentialCollisionBoxes.size(); collisionBoxId++)
					{
						CellBox* collisionBox = box.m_potentialCollisionBoxes[collisionBoxId];

						int innnerCellStart = collisionBox != &box ? 0 : cellId + 1; // avoid double update when testing in same box

						for(int innerCellId = innnerCellStart; innerCellId < (int)collisionBox->m_positions.size(); innerCellId++)
						//for(int innerCellId = (int)collisionBox->m_positions.size() - 1; innerCellId >= innnerCellStart; innerCellId--)
						{*/


							/*int cryptId1 = (int)m_cells.CryptIds[i];
							int cryptId2 = (int)m_cells.CryptIds[j];

							Vector3D cryptPos1 = m_crypts.m_cryptPositions[cryptId1];
							Vector3D cryptPos2 = m_crypts.m_cryptPositions[cryptId2];*/

							// Use this version for spherical cells
							/*Vector3D outerPos = box.m_positions[cellId];
							Vector3D innerPos = collisionBox->m_positions[innerCellId];*/
							/*
							Vector3D innerPos;
							Vector3D outerPos;

							if(box.m_positions[cellId].y < 0.0f && collisionBox->m_positions[innerCellId].y < 100.0f)
							{
								outerPos = box.m_onMembranePositions[cellId];
								innerPos = collisionBox->m_onMembranePositions[innerCellId];
							}
							else
							{
								outerPos = box.m_positions[cellId];
								innerPos = collisionBox->m_positions[innerCellId];
							}

							//Vector3d dummy;

							//GetClosestPointOnMembrane(m_cells.Positions[i] - cryptPos1, out outerPos, out dummy);
							//GetClosestPointOnMembrane(m_cells.Positions[j] - cryptPos2, out innerPos, out dummy);

							Vector3D delta = outerPos - innerPos;
							double separation = delta.Length();

							if(outerPos.x < -1500.0f || outerPos.x > 1500.0f)
							{
								int a = 0;
							}

							double targetSeparation = box.m_radii[cellId] + collisionBox->m_radii[innerCellId];

							CellReference& otherCell  = box.m_otherSubCellIndex[cellId];
							if (otherCell.m_active
								&& otherCell.m_box == collisionBox
								&& otherCell.m_cellId == innerCellId)
							{
								double growthFactor;
								
								if(box.m_cycleStages[cellId] == CellCycleStages::M)
								{
									growthFactor = box.m_currentStageNumTimesteps[cellId] / m_MStageRequiredTimesteps;
								}
								else
								{
									growthFactor = collisionBox->m_currentStageNumTimesteps[innerCellId] / m_MStageRequiredTimesteps;
								}
								
								targetSeparation *= growthFactor;
							}*/
							/*else
							{
							if (m_cells.CycleStages[i] == CellCycleStage.Child)
							{
							int parentId = m_cells.ChildPointIndices[i];
							double growthFactor = m_cells.GrowthStageCurrentTimesteps[parentId] / m_cells.GrowthStageRequiredTimesteps[parentId];
							targetSeparation = m_cells.Radii[j] + m_cells.Radii[i] * growthFactor;
							}
							if (m_cells.CycleStages[j] == CellCycleStage.Child)
							{
							int parentId = m_cells.ChildPointIndices[j];
							double growthFactor = m_cells.GrowthStageCurrentTimesteps[parentId] / m_cells.GrowthStageRequiredTimesteps[parentId];
							targetSeparation = m_cells.Radii[i] + m_cells.Radii[j] * growthFactor;
							}
							}*/
							/*
							if (separation < targetSeparation)
							{
								double restitution = targetSeparation - separation;
								restitution *= m_cellStiffness;
								if (separation < 0.1f)
								{
									separation = 0.1f;
									delta.x = (double)std::rand() / RAND_MAX - 0.5f;
									delta.y = (double)std::rand() / RAND_MAX - 0.5f;
									delta.z = (double)std::rand() / RAND_MAX - 0.5f;
								}
								
								Vector3D force = delta / separation * restitution;*/
								/*Vector3D cryptForce = force;
								cryptForce.Y = 0.0f;

								m_crypts.m_cellularity[cryptId1]++;
								m_crypts.m_cellularity[cryptId2]++;

								if ((cryptPos1 - outerPos).Length() < m_cryptRadius + m_flutingRadius)
								{
								//m_crypts.m_forces[cryptId1] += cryptForce;
								}

								if ((cryptPos2 - innerPos).Length() < m_cryptRadius + m_flutingRadius)
								{
								//m_crypts.m_forces[cryptId2] -= cryptForce;
								}*/
							/*
								Vector3D outerForce = force;
								Vector3D innerForce = force;

								if(box.m_mutations[cellId].mutateCellForces)
								{
									outerForce /= 2.0;
								}

								if(collisionBox->m_mutations[innerCellId].mutateCellForces)
								{
									innerForce /= 2.0;
								}

								box.m_positions[cellId] += outerForce;
								collisionBox->m_positions[innerCellId] -= innerForce;
							}
						}
					}
				}
			}
		}*/
	}
};