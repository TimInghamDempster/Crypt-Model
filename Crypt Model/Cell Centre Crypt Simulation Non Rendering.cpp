
#include "SystemIncludes.cpp"

#include "ProgramIncludes.cpp"

Simulation simulation;

int main(int argc, char* argv[])
{
	int framecount = 0;
	const int totalFrames = 1000000;

	HINSTANCE hInst = GetModuleHandle(NULL);
	HWND hwnd = TCreateWindow(hInst);


	bool init = simulation.InitSimulation();
	
	if(init)
	{
		std::ofstream outputFile;
		outputFile.open("C:/Users/Tim/Desktop/testData.csv");

		while(framecount < totalFrames)
		{
			TWinMain();
			simulation.StepSimulation();	

			if(framecount % 200 == 0)
			{
			}

			//Sleep(10);
		}

		outputFile.close();
	
		simulation.CleanUpSimulation();
	}

	return 0;
}