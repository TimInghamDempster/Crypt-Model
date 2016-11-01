
#include "SystemIncludes.cpp"

#include "ProgramIncludes.cpp"

Simulation simulation;

int main(int argc, char* argv[])
{
	int framecount = 0;
	const int totalFrames = 1000000;

	bool init = simulation.InitSimulation();
	
	if(init)
	{
		std::ofstream outputFile;
		outputFile.open("C:/Users/Tim/Desktop/testData.csv");

		while(framecount < totalFrames)
		{
			simulation.StepSimulation();	

			if(framecount % 200 == 0)
			{
			}

			//Sleep(10);
			framecount++;
		}

		outputFile.close();
	
		simulation.CleanUpSimulation();
	}

	return 0;
}