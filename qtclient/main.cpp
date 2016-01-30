#include "signetapplication.h"

int main(int argc, char **argv)
{
	SignetApplication a(argc, argv);
	if (a.sendMessage("wake up"))
		return 0;
	a.init();
	a.exec();
	return 0;
}
