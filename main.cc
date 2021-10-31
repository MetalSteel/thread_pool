#include "thread_pool.h"

int Calc(int x, int y, int n)
{
	std::cout << "thread id = " << std::this_thread::get_id() << std::endl;

	std::this_thread::sleep_for(std::chrono::seconds(n));
	return x + y;
}

int main(int argc, char* argv[])
{
	ThreadPool pool;
	pool.Start();

	auto r1 = pool.Submit(Calc, 10, 20, 3);
	auto r2 = pool.Submit(Calc, 10, 30, 2);
	auto r3 = pool.Submit(Calc, 10, 40, 4);
	auto r4 = pool.Submit(Calc, 10, 50, 5);
	auto r5 = pool.Submit(Calc, 10, 60, 6);
	auto r6 = pool.Submit(Calc, 10, 70, 7);
	auto r7 = pool.Submit(Calc, 10, 80, 8);
	auto r8 = pool.Submit(Calc, 10, 90, 9);

	std::cout << r1.get() << std::endl;
	std::cout << r2.get() << std::endl;
	std::cout << r3.get() << std::endl;
	std::cout << r4.get() << std::endl;
	std::cout << r5.get() << std::endl;
	std::cout << r6.get() << std::endl;
	std::cout << r7.get() << std::endl;
	std::cout << r8.get() << std::endl;

	pool.Shutdown();

	return EXIT_SUCCESS;
}