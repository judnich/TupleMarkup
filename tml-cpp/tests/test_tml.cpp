#include "../source/tml.hpp"

#include <iostream>
using namespace std;

int main(void)
{
	cout << endl << "========== SIMPLE TML C++ TEST ==========" << endl;
	cout << "  The TML C++ library is just a wrapper over the C parser implementation." << endl;
	cout << "  This is not a unit test suite, just a simple sanity check to make sure" << endl;
	cout << "  the C++ wrapper works correctly. For more comprehensive unit tests, refer" << endl;
	cout << "  to the C implementation's tests (located in 'tml-c/tests')." << endl << endl;

	string str = "[ position | 0.1 9.8 2.55 ]";
	cout << "Parsing \"" << str << "\"..." << endl;

	TmlData *data = TmlData::parseString(str);

	TmlNode root = data->getRoot();

	TmlNode titleNode = root.getChildAtIndex(0);
	string title = titleNode.toString();

	TmlNode positionNode = root.getChildAtIndex(1);

	TmlNode element = positionNode.getFirstChild();
	double xPos = element.toDouble();
	element = element.getNextSibling();
	double yPos = element.toDouble();
	element = element.getNextSibling();
	double zPos = element.toDouble();

	cout << "The parsed \"" << title << "\" is (x=" << xPos << ", y=" << yPos << ", z=" << zPos << ")." << endl;

	cout << endl;

	delete data;
	return 0;
}
