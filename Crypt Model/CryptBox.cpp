
struct CryptBox;

struct MutationData
{
	bool mutateQuiecence;
	bool mutateAttachment;
	bool mutateCellForces;
};

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

	CryptBox(int expectedNumberOfCells)
	{
		m_positions = new std::vector<Vector2D>();
		m_nextPositions = new std::vector<Vector2D>();
		int reserveSize = expectedNumberOfCells * 10;
		m_positions->reserve(reserveSize);
		m_nextPositions->reserve(reserveSize);
		m_collisionReferences.reserve(reserveSize);
	}

	void CleanUp()
	{
		delete m_positions;
		delete m_nextPositions;
	}

	int AddCrypt(Vector2D position)
	{
		m_positions->push_back(position);
		m_nextPositions->push_back(position);
		std::vector<CryptReference> refs;
		m_collisionReferences.push_back(refs);
		
		return m_positions->size() - 1;
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
	}
};
