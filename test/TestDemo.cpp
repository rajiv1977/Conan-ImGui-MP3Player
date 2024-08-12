#include "ImGuiVis.h"

using namespace std;

int main(void)
{
	string musicFile = "Oryza.mp3";
    std::shared_ptr<ImGuiVis> run(new ImGuiVis(musicFile));
    return EXIT_SUCCESS;
}
