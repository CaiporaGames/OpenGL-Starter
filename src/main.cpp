#include "../include/app/App.hpp"
#include <cstdio>

int main() 
{
	App app;

	if (!app.init(800, 600, "Pong Game")) return 1;

	app.run();

	return 0;
}
