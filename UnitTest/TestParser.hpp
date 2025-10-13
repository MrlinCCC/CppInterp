#include "gtest/gtest.h"
#include <Parser.h>

using namespace CppInterp;

static Parser g_parser;

TEST(ParserTokenTest, PrintAstTree) {
	std::string str = R"(
        import "math";

        function int main(double i=1, string s="123") {
            let int x = 5;
            const int y = 10;
            let int z = x + y * 2;

            if (z > 20) {
                print("large");
            } else {
                print("small");
            }

            while (x < y) {
                x = x + 1;
            }

            for (let int i = 0; i < 3; i = i + 1) {
                print(i);
            }

            return z;
        }
    )";
	try
	{
		auto root = g_parser.Parse(str);
		Parser::PrintAstTree(root);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

}
