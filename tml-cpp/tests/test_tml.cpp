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

	string str = "[ [color|red] [position | 0.1 9.8 2.55] ]";
	cout << "Parsing \"" << str << "\"..." << endl;

	TmlDoc *doc = TmlDoc::parseString(str);
	TmlNode root = doc->getRoot();

	TmlNode positionNode = root.findFirstChild("[position | \\? \\? \\?]"); // returns [position | 0.1 9.8 2.55]
	string nodeName = positionNode.getChildAtIndex(0).toString(); // returns "position"

	TmlNode posData = positionNode.getChildAtIndex(1); // returns [0.1 9.8 2.55]

	float vec[3];
	posData.toFloatArray(vec, 3);

	cout << "The parsed \"" << nodeName << "\" is (x=" << vec[0] << ", y=" << vec[1] << ", z=" << vec[2] << ")." << endl;

	cout << endl;

	delete doc;
	return 0;
}
