
struct CryptBox;

struct CryptReference
{
	CryptBox* m_box;
	int m_cryptId;
	bool m_active;

	CryptReference() :
		m_box(NULL),
		m_cryptId(-1),
		m_active(false)
	{
	}
};

struct CryptBox
{
	std::vector<Vector2D>* m_positions;
	std::vector<Vector2D>* m_nextPositions;
	std::vector<std::vector<CryptReference>> m_collisionReferences;
	std::vector<CryptBox*> m_collisionBoxes;
	std::vector<double> m_lengths;
	std::vector<double> m_nextLengths;
	std::vector<int> m_deadCrypts;
	std::vector<int> m_mutated;
	std::vector<double> m_mucosalMutation;
	NormalDistributionRNG& m_growthFactorRNG;

	CryptBox(int expectedNumberOfCells, NormalDistributionRNG& growthFactorRNG) : 
		m_growthFactorRNG(growthFactorRNG)
	{
		m_positions = new std::vector<Vector2D>();
		m_nextPositions = new std::vector<Vector2D>();
		int reserveSize = expectedNumberOfCells * 10;
		m_positions->reserve(reserveSize);
		m_nextPositions->reserve(reserveSize);
		m_collisionReferences.reserve(reserveSize);
		m_lengths.reserve(reserveSize);
		m_nextLengths.reserve(reserveSize);
		m_deadCrypts.reserve(reserveSize);
		m_mutated.reserve(reserveSize);
		m_mucosalMutation.reserve(reserveSize);
	}

	void CleanUp()
	{
		delete m_positions;
		delete m_nextPositions;
	}

	int AddCrypt(Vector2D position, double startLength, double growthFactor, int mutated)
	{
		m_positions->push_back(position);
		m_nextPositions->push_back(position);
		std::vector<CryptReference> refs;
		m_collisionReferences.push_back(refs);
		m_lengths.push_back(startLength);
		m_nextLengths.push_back(growthFactor);
		m_deadCrypts.push_back(0);
		m_mutated.push_back(mutated);
		m_mucosalMutation.push_back(0.0);

		return m_positions->size() - 1;
	}

	void UpdateCrypts(double gridWidth, double gridHeight)
	{
		for(int i = 0; i < m_positions->size(); i++)
		{
			UpdateCrypt(i, gridWidth, gridHeight);
		}
	}

	void Mutate()
	{
		m_mutated[0] = 1;
	}

	void UpdateCrypt(int id, double gridWidth, double gridHeight)
	{
		if(m_mutated[id] == 0)
		{
			if(m_lengths[id] < m_nextLengths[id])
			{
				m_lengths[id] += 0.01;
			}
			else
			{
				m_lengths[id] -= 0.01;
			}
		}
		else
		{
			m_lengths[id] += 0.001;
			m_mucosalMutation[id] += 10.0;
			if(m_mucosalMutation[id] > 100.0)
			{
				m_mucosalMutation[id] = 100.0;
			}
		}

		if(abs(m_lengths[id] - m_nextLengths[id]) < 0.02)
		{
			m_nextLengths[id] = m_growthFactorRNG.Next();
		}

		if(m_lengths[id] > 1.0)
		{
			m_lengths[id] = 0.5;
			AddCrypt((*m_positions)[id], 0.5, m_growthFactorRNG.Next(), m_mutated[id]);
			(*m_positions)[id].x += 0.00001;
			(*m_positions)[id].y += 0.00001;
			m_nextLengths[id] = m_growthFactorRNG.Next();
		}
		else if(m_lengths[id] < 0.0)
		{
			m_deadCrypts[id] = 1;
		}

		if((*m_positions)[id].x < 0.0f)
		{
			(*m_positions)[id].x -= (*m_positions)[id].x * 0.1f;
		}
		if((*m_positions)[id].x > gridWidth)
		{
			(*m_positions)[id].x -= ((*m_positions)[id].x - gridWidth) * 0.1f;
		}
		if((*m_positions)[id].y < 0.0f)
		{
			(*m_positions)[id].y -= (*m_positions)[id].y * 0.1f;
		}
		if((*m_positions)[id].y > gridHeight)
		{
			(*m_positions)[id].y -= ((*m_positions)[id].y - gridHeight) * 0.1f;
		}
	}


	void RemoveDeadCrypts()
	{
		for(int i = 0; i < m_positions->size(); i++)
		{
			if(m_deadCrypts[i] == 1)
			{
				RemoveCrypt(i);
			}
		}
	}

	void UpdateCollisions()
	{
		for(int i = 0; i < m_positions->size(); i++)
		{
			m_collisionReferences[i].clear();
		}

		for(int otherBoxId = 0; otherBoxId < m_collisionBoxes.size(); otherBoxId++)
		{
			CryptBox* otherBox = m_collisionBoxes[otherBoxId];

			for(int i = 0; i < m_positions->size(); i++)
			{
				for(int j = 0; j < otherBox->m_positions->size(); j++)
				{
					if(m_deadCrypts[i] == 0 && otherBox->m_deadCrypts[j] == 0)
					{
						Vector2D delta = (*m_positions)[i] - (*otherBox->m_positions)[j];
						if(delta.Length() <= 2.0)
						{
							CryptReference ref;
							ref.m_box = otherBox;
							ref.m_cryptId = j;
							m_collisionReferences[i].push_back(ref);
						}
					}
				}
			}
		}
		for(int i = 0; i < m_positions->size(); i++)
		{
			for(int j = i + 1; j < m_positions->size(); j++)
			{
				Vector2D delta = (*m_positions)[i] - (*m_positions)[j];
				if(delta.Length() <= 2.0)
				{
					CryptReference ref;
					ref.m_box = this;
					ref.m_cryptId = j;
					m_collisionReferences[i].push_back(ref);
				}
			}
		}
	}

	void UpdatePositions()
	{
		for(int crypt = 0; crypt < m_collisionReferences.size(); crypt++)
		{
			for(int otherCryptId = 0; otherCryptId < m_collisionReferences[crypt].size(); otherCryptId++)
			{
				CryptBox* otherBox = m_collisionReferences[crypt][otherCryptId].m_box;
				int otherCrypt = m_collisionReferences[crypt][otherCryptId].m_cryptId;

				Vector2D delta = (*m_positions)[crypt] - (*otherBox->m_positions)[otherCrypt];
				if(delta.Length() < 1.0)
				{
					Vector2D normalised = delta / delta.Length();
					
					float error = 1.0 - delta.Length();
					normalised *= error;
					normalised /= 5.0f;

					(*m_nextPositions)[crypt] += normalised;
					(*otherBox->m_nextPositions)[otherCrypt] -= normalised;
				}

				float invasionDelta = m_mucosalMutation[crypt] - otherBox->m_mucosalMutation[otherCrypt];
				invasionDelta *= 0.2;

				if((invasionDelta > 0.0 && m_mucosalMutation[crypt] > 90.0) || (invasionDelta < 0.0 && otherBox->m_mucosalMutation[otherCrypt] > 90.0))
				{
					m_mucosalMutation[crypt] -= invasionDelta;
					otherBox->m_mucosalMutation[otherCrypt] += invasionDelta;
				}
			}
		}
	}

	void ClonePositions()
	{
		for(int i = 0; i < m_positions->size(); i++)
		{
			(*m_nextPositions)[i] = (*m_positions)[i];
		}
	}
	
	void SwapBuffers()
	{
		std::vector<Vector2D>* temp = m_positions;
		m_positions = m_nextPositions;
		m_nextPositions = temp;
	}

	void RemoveCrypt(int cryptId)
	{
		int last = m_positions->size() - 1;

		(*m_positions)[cryptId] = (*m_positions)[last];
		m_positions->pop_back();
		(*m_nextPositions)[cryptId] = (*m_nextPositions)[last];
		m_nextPositions->pop_back();
		m_collisionReferences[cryptId] = m_collisionReferences[last];
		m_collisionReferences.pop_back();
		m_lengths[cryptId] = m_lengths[last];
		m_lengths.pop_back();
		m_nextLengths[cryptId] = m_nextLengths[last];
		m_nextLengths.pop_back();
		m_deadCrypts[cryptId] = m_deadCrypts[last];
		m_deadCrypts.pop_back();
		m_mutated[cryptId] = m_mutated[last];
		m_mutated.pop_back();
		m_mucosalMutation[cryptId] = m_mucosalMutation[last];
		m_mucosalMutation.pop_back();
	}
};
