
#include "SystemIncludes.cpp"

#include "ProgramIncludes.cpp"

#include "../D3D/D3DIncludes.cpp"

Simulation simulation;

int main(int argc, char* argv[])
{
	int framecount = 0;
	const int totalFrames = 1000000;

	HINSTANCE hInst = GetModuleHandle(NULL);
	HWND hwnd = TCreateWindow(hInst);
	Renderer::Init(hwnd, 1280, 720);


	bool init = simulation.InitSimulation();
	
	if(init)
	{
		std::ofstream outputFile;
		outputFile.open("C:/Users/Tim/Desktop/testData.csv");

		while(framecount < totalFrames)
		{
			TWinMain();
			simulation.StepSimulation(framecount);
			Renderer::Draw();			

			if(framecount % 200 == 0)
			{
			}

			//Sleep(10);
			framecount++;
		}

		outputFile.close();
	
		simulation.CleanUpSimulation();
	}
	Renderer::CleanUp();	

	return 0;
}