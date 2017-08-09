
struct Simulation
{
	struct SimSetup
	{
		std::string filename;
	};

	std::string filename;
	SimSetup currentSettings;

	FlatGrid m_grid;

	Simulation() :
		m_grid(100,10,2,2.0,2.0)
	{
	}

	bool InitSimulation()
	{
		SimSetup settings[] = 
		{
			{""},
		};

		unsigned numSettings = 120;

		char* env = getenv("SGE_TASK_ID");

		std::stringstream stream;
	
		int seed = time(NULL);
		if(env)
		{
			unsigned taskId = 0;

			stream << env;
			stream >> taskId;

			if(taskId < numSettings)
			{
				// time() resolution is really poor so need to make sure that repeats don't have the same seed.
				seed += taskId;

				currentSettings = settings[taskId];
				
				filename = settings[taskId].filename;
				
				srand(seed);
				return true;
			}
		}

		return false;
	}

	void StepSimulation(int framecount)
	{
		m_grid.Step(framecount);
	}

	void CleanUpSimulation()
	{
		m_grid.CleanUp();
	}
};
