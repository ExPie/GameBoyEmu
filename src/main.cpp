#include "main.hpp"
#include "cpu.hpp"
#include "render.hpp"
#include "memory.hpp"
#include "board.hpp"
#include <iostream>

#include <chrono>
#include <thread>

void printHello() {
	std::cout << "Hello, world." << std::endl;
}

int Adder::add(int x, int y) {
	return x + y;
}

int main(int argc, char* args[]) {
	printHello();
	std::cout << Adder::add(1, 2) << std::endl;
	std::cout << sizeof(char) << std::endl;


	//for(long i = 0; i < 10000000; i++)
	//	std::this_thread::sleep_for(std::chrono::nanoseconds(1));
	std::chrono::nanoseconds a = std::chrono::nanoseconds(1200000000);
	std::chrono::system_clock::time_point b = std::chrono::system_clock::now();
	std::this_thread::sleep_until(b + a);
	std::cout << sizeof(char) << std::endl;

	//Memory mem("..\\..\\ROM\\Super Mario Land 2 - 6 Golden Coins (UE) (V1.2) [!].gb");
	
	
	Render *render;
	if(argc > 1) render = new Render(std::string(args[1]));
	else render = new Render();
	delete render;

	std::cin.ignore();
	
	return 0;
}
