#include "primario4.h"

using namespace std;

int main(int argc, char** argv)
{
	int task_init;
	task_init=Init_All();
	if(task_init<=0)
	{
		printf("Error Starting the System");
	}
	// YOU NEVER REACH HERE, because this program runs directly or on a
	// microcontroller and is not called by any Operating System, as in the 
	// case of a PC program.
	return 0;
}
					
