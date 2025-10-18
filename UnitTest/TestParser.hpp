#include "gtest/gtest.h"
#include <Parser.h>

using namespace CppInterp;

static Parser g_parser;

struct ExpectedNode {
	NodeType::Type type;
	std::string content;
	std::vector<ExpectedNode> children;
};

struct ParserCase {
	std::string input;
	ExpectedNode expectedTree;
};

class ParserSyntaxTest : public ::testing::TestWithParam<ParserCase> {};

static void ExpectAstMatch(AstNode* actual, const ExpectedNode& expected, const std::string& path = "") {
	ASSERT_NE(actual, nullptr) << "Null AST node at path: " << path;
	EXPECT_EQ(actual->m_type, expected.type)
		<< "Type mismatch at path: " << path;
	EXPECT_EQ(actual->m_token.m_content, expected.content)
		<< "Content mismatch at path: " << path;

	ASSERT_EQ(actual->m_children.size(), expected.children.size())
		<< "Child count mismatch at path: " << path;

	for (size_t i = 0; i < expected.children.size(); ++i) {
		std::string childPath = path + "/" + std::to_string(i);
		ExpectAstMatch(actual->m_children[i], expected.children[i], childPath);
		ASSERT_EQ(actual->m_children[i]->m_parent, actual)
			<< "Parent mismatch at path: " << childPath;
	}
}

static void ExpectAstNodeNumsMatch(AstNode* actual, size_t expectedNum) {
	size_t actualNum = 0;
	std::function<void(AstNode*)> countNodes = [&](AstNode* node) {
		if (!node) return;
		actualNum++;
		for (auto child : node->m_children) {
			countNodes(child);
		}
		};
	countNodes(actual);
	EXPECT_EQ(actualNum, expectedNum) << "Total AST node count mismatch";
}

TEST_P(ParserSyntaxTest, ParsesAst) {
	const auto& param = GetParam();
	AstNode* root = g_parser.Parse(param.input);
	//Parser::PrintAstTree(root);
	ASSERT_NE(root, nullptr);
	ExpectAstMatch(root, param.expectedTree);
	ExpectAstNodeNumsMatch(root, g_parser.GetNodes().size());
}

std::vector<ParserCase> parserSimpleCases = {
		{
			R"(import math;)",
			{
				NodeType::PROGRAM, "",
				{
					{ NodeType::IMPORT_STMT, "import",
						{
							{ NodeType::IDENTIFIER, "math", {} }
						}
					}
				}
			}
		},
		{
		R"(function int add(int a, int b) { return a + b; })",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::FUNCTION_DECL, "function",
					{
						{ NodeType::BUILTIN_TYPE, "int", {} },
						{ NodeType::IDENTIFIER, "add", {} },
						{ NodeType::PARAMETER_LIST, "",
							{
								{ NodeType::PARAMETER, "",
									{
										{ NodeType::BUILTIN_TYPE, "int", {} },
										{ NodeType::IDENTIFIER, "a", {} }
									}
								},
								{ NodeType::PARAMETER, "",
									{
										{ NodeType::BUILTIN_TYPE, "int", {} },
										{ NodeType::IDENTIFIER, "b", {} }
									}
								}
							}
						},
						{ NodeType::COMPOUND_STMT, "",
							{
								{ NodeType::RETURN_STMT, "return",
									{
										{ NodeType::BINARY_EXPR, "+",
											{
												{ NodeType::IDENTIFIER, "a", {} },
												{ NodeType::IDENTIFIER, "b", {} }
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	},
	{
		R"(let bool flag = true, tag = false; let Member m = {.a = 1, .b = 2};)",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::VAR_DECL, "let",
					{
						{ NodeType::BUILTIN_TYPE, "bool", {} },
						{ NodeType::DECLARATOR_LIST, "", {
							{ NodeType::DECLARATOR, "",
								{
									{ NodeType::IDENTIFIER, "flag", {} },
									{ NodeType::LITERAL, "true", {} }
								}
							},
							{ NodeType::DECLARATOR, "",
								{
									{ NodeType::IDENTIFIER, "tag", {} },
									{ NodeType::LITERAL, "false", {} }
								}
							}
						}}
					}
				},
				{ NodeType::VAR_DECL, "let",
					{
						{ NodeType::IDENTIFIER, "Member", {} },
						{ NodeType::DECLARATOR, "",
							{
								{ NodeType::IDENTIFIER, "m", {} },
								{ NodeType::INITIALIZER, "",
									{
										{ NodeType::DESIGNATED_INITIALIZER, ".",
											{
												{ NodeType::IDENTIFIER, "a", {} },
												{ NodeType::LITERAL, "1", {} }
											}
										},
										{ NodeType::DESIGNATED_INITIALIZER, ".",
											{
												{ NodeType::IDENTIFIER, "b", {} },
												{ NodeType::LITERAL, "2", {} }
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	},
	{
		R"(let int arr[3] = { 1, 2, 3 };)",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::VAR_DECL, "let",
					{
						{ NodeType::BUILTIN_TYPE, "int", {} },
						{ NodeType::DECLARATOR, "",
							{
								{ NodeType::IDENTIFIER, "arr", {} },
								{ NodeType::ARRAY_SIZE, "", {
									{ NodeType::LITERAL, "3", {} },
								}},
								{ NodeType::INITIALIZER, "",
									{
										{ NodeType::LITERAL, "1", {} },
										{ NodeType::LITERAL, "2", {} },
										{ NodeType::LITERAL, "3", {} }
									}
								}
							}
						}
					}
				}
			}
		}
	},
	{
	R"(let int array[3][3] = {{1,2,3},{4,5,6},{7,8,9}}; let empty array = {};)",
	{
		NodeType::PROGRAM, "",
		{
			{ NodeType::VAR_DECL, "let",
				{
					{ NodeType::BUILTIN_TYPE, "int", {} },
					{ NodeType::DECLARATOR, "",
						{
							{ NodeType::IDENTIFIER, "array", {} },
							{ NodeType::ARRAY_SIZE, "",
								{
									{ NodeType::LITERAL, "3", {} },
									{ NodeType::LITERAL, "3", {} }
								}
							},
							{ NodeType::INITIALIZER, "",
								{
									{ NodeType::INITIALIZER, "",
										{
											{ NodeType::LITERAL, "1", {} },
											{ NodeType::LITERAL, "2", {} },
											{ NodeType::LITERAL, "3", {} }
										}
									},
									{ NodeType::INITIALIZER, "",
										{
											{ NodeType::LITERAL, "4", {} },
											{ NodeType::LITERAL, "5", {} },
											{ NodeType::LITERAL, "6", {} }
										}
									},
									{ NodeType::INITIALIZER, "",
										{
											{ NodeType::LITERAL, "7", {} },
											{ NodeType::LITERAL, "8", {} },
											{ NodeType::LITERAL, "9", {} }
										}
									}
								}
							}
						}
					}
				}
			},
			{ NodeType::VAR_DECL, "let",
				{
					{ NodeType::IDENTIFIER, "empty", {} },
					{ NodeType::DECLARATOR, "",
						{
							{ NodeType::IDENTIFIER, "array", {} },
							{ NodeType::INITIALIZER, "", {} }
						}
					}
				}
			}
		}
	}
	},
	{
		R"(struct Point { int x; int y; };)",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::STRUCT_DECL, "struct",
					{
						{ NodeType::IDENTIFIER, "Point", {} },
						{ NodeType::STRUCT_DECLARATOR_LIST, "",
							{
								{ NodeType::STRUCT_MEMBER_DECL, "",
									{
										{ NodeType::BUILTIN_TYPE, "int", {} },
										{ NodeType::IDENTIFIER, "x", {} }
									}
								},
								{ NodeType::STRUCT_MEMBER_DECL, "",
									{
										{ NodeType::BUILTIN_TYPE, "int", {} },
										{ NodeType::IDENTIFIER, "y", {} }
									}
								}
							}
						}
					}
				}
			}
		}
	},
	{
		R"(if (x > 0) { y = 1; } else { y = 2; })",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::IF_STMT, "if",
					{
						{ NodeType::BINARY_EXPR, ">",
							{
								{ NodeType::IDENTIFIER, "x", {} },
								{ NodeType::LITERAL, "0", {} }
							}
						},
						{ NodeType::COMPOUND_STMT, "",
							{
								{ NodeType::ASSIGN_EXPR, "=",
									{
										{ NodeType::IDENTIFIER, "y", {} },
										{ NodeType::LITERAL, "1", {} }
									}
								}
							}
						},
						{ NodeType::COMPOUND_STMT, "",
							{
								{ NodeType::ASSIGN_EXPR, "=",
									{
										{ NodeType::IDENTIFIER, "y", {} },
										{ NodeType::LITERAL, "2", {} }
									}
								}
							}
						}
					}
				}
			}
		}
	},
	{
		R"(while (i < 10) { i = i + 1; continue; })",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::WHILE_STMT, "while",
					{
						{ NodeType::BINARY_EXPR, "<",
							{
								{ NodeType::IDENTIFIER, "i", {} },
								{ NodeType::LITERAL, "10", {} }
							}
						},
						{ NodeType::COMPOUND_STMT, "",
							{
								{ NodeType::ASSIGN_EXPR, "=",
									{
										{ NodeType::IDENTIFIER, "i", {} },
										{ NodeType::BINARY_EXPR, "+",
											{
												{ NodeType::IDENTIFIER, "i", {} },
												{ NodeType::LITERAL, "1", {} }
											}
										}
									}
								},
								{ NodeType::CONTINUE_STMT, "continue", {} }
							}
						}
					}
				}
			}
		}
	},
	{
		R"(switch(x) { case 1: y = 2; break; default: y = 0; })",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::SWITCH_STMT, "switch",
					{
						{ NodeType::IDENTIFIER, "x", {} },
						{ NodeType::CASE_CLAUSE, "case",
							{
								{ NodeType::LITERAL, "1", {} },
								{ NodeType::ASSIGN_EXPR, "=",
									{
										{ NodeType::IDENTIFIER, "y", {} },
										{ NodeType::LITERAL, "2", {} }
									}
								},
								{ NodeType::BREAK_STMT, "break", {} }
							}
						},
						{ NodeType::DEFAULT_CLAUSE, "default",
							{
								{ NodeType::ASSIGN_EXPR, "=",
									{
										{ NodeType::IDENTIFIER, "y", {} },
										{ NodeType::LITERAL, "0", {} }
									}
								}
							}
						}
					}
				}
			}
		}
	},
	{
		R"(for (i = 0; i < 10; i = i + 1) sum = sum + i;)",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::FOR_STMT, "for",
					{
						{ NodeType::ASSIGN_EXPR, "=",
							{
								{ NodeType::IDENTIFIER, "i", {} },
								{ NodeType::LITERAL, "0", {} }
							}
						},
						{ NodeType::BINARY_EXPR, "<",
							{
								{ NodeType::IDENTIFIER, "i", {} },
								{ NodeType::LITERAL, "10", {} }
							}
						},
						{ NodeType::ASSIGN_EXPR, "=",
							{
								{ NodeType::IDENTIFIER, "i", {} },
								{ NodeType::BINARY_EXPR, "+",
									{
										{ NodeType::IDENTIFIER, "i", {} },
										{ NodeType::LITERAL, "1", {} }
									}
								}
							}
						},
						{ NodeType::ASSIGN_EXPR, "=",
							{
								{ NodeType::IDENTIFIER, "sum", {} },
								{ NodeType::BINARY_EXPR, "+",
									{
										{ NodeType::IDENTIFIER, "sum", {} },
										{ NodeType::IDENTIFIER, "i", {} }
									}
								}
							}
						}
					}
				}
			}
		}
	},
	{
		R"(a = x > 0 ? 1 : 2;)",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::ASSIGN_EXPR, "=",
					{
						{ NodeType::IDENTIFIER, "a", {} },
						{ NodeType::CONDITIONAL_EXPR, "?",
							{
								{ NodeType::BINARY_EXPR, ">",
									{
										{ NodeType::IDENTIFIER, "x", {} },
										{ NodeType::LITERAL, "0", {} }
									}
								},
								{ NodeType::LITERAL, "1", {} },
								{ NodeType::LITERAL, "2", {} }
							}
						}
					}
				}
			}
		}
	},
	{
		R"(obj.list[i++].value = -10;)",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::ASSIGN_EXPR, "=",
					{
						{ NodeType::MEMBER_EXPR, ".",
							{
								{ NodeType::INDEX_EXPR, "",
									{
										{ NodeType::MEMBER_EXPR, ".",
											{
												{ NodeType::IDENTIFIER, "obj", {} },
												{ NodeType::IDENTIFIER, "list", {} }
											}
										},
										{ NodeType::POSTFIX_EXPR, "++",
											{
												{ NodeType::IDENTIFIER, "i", {} }
											}
										}
									}
								},
								{ NodeType::IDENTIFIER, "value", {} }
							}
						},
						{ NodeType::UNARY_EXPR, "-",
							{
								{ NodeType::LITERAL, "10", {} }
							}
						}
					}
				}
			}
		}
	},
	{
		R"((a + b) * (c - (d + e));)",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::BINARY_EXPR, "*",
					{
						{ NodeType::GROUP_EXPR, "",
							{
								{ NodeType::BINARY_EXPR, "+",
									{
										{ NodeType::IDENTIFIER, "a", {} },
										{ NodeType::IDENTIFIER, "b", {} }
									}
								}
							}
						},
						{ NodeType::GROUP_EXPR, "",
							{
								{ NodeType::BINARY_EXPR, "-",
									{
										{ NodeType::IDENTIFIER, "c", {} },
										{ NodeType::GROUP_EXPR, "",
											{
												{ NodeType::BINARY_EXPR, "+",
													{
														{ NodeType::IDENTIFIER, "d", {} },
														{ NodeType::IDENTIFIER, "e", {} }
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	},
	{
		R"(foo(1, 2);)",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::CALL_EXPR, "",
					{
						{ NodeType::IDENTIFIER, "foo", {} },
						{ NodeType::ARGUMENT_LIST, "",
							{
								{ NodeType::LITERAL, "1", {} },
								{ NodeType::LITERAL, "2", {} }
							}
						}
					}
				}
			}
		}
	},
	{
		R"(;)",
		{
			NodeType::PROGRAM, "",
			{
				{ NodeType::EMPTY_STMT, "", {} }
			}
		}
	}
};



INSTANTIATE_TEST_SUITE_P(ParserSimpleSyntax, ParserSyntaxTest, ::testing::ValuesIn(parserSimpleCases));




static ExpectedNode MakeNode(NodeType::Type type,
	std::string content = "",
	std::vector<ExpectedNode> children = {}) {
	return { type, std::move(content), std::move(children) };
}

const char* source1 = R"(let (int, int) -> int f = lambda(int x, int y) -> int { return x + y; };)";

auto case1 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::FUNCTION_TYPE, "", {
			MakeNode(NodeType::PARAMETER_TYPE_LIST, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::BUILTIN_TYPE, "int")
			}),
			MakeNode(NodeType::BUILTIN_TYPE, "int")
		}),
		MakeNode(NodeType::DECLARATOR, "", {
			MakeNode(NodeType::IDENTIFIER, "f"),
			MakeNode(NodeType::FUNCTION_LITERAL, "", {
				MakeNode(NodeType::PARAMETER_LIST, "", {
					MakeNode(NodeType::PARAMETER, "", {
						MakeNode(NodeType::BUILTIN_TYPE, "int"),
						MakeNode(NodeType::IDENTIFIER, "x")
					}),
					MakeNode(NodeType::PARAMETER, "", {
						MakeNode(NodeType::BUILTIN_TYPE, "int"),
						MakeNode(NodeType::IDENTIFIER, "y")
					})
				}),
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::COMPOUND_STMT, "", {
					MakeNode(NodeType::RETURN_STMT, "return", {
						MakeNode(NodeType::BINARY_EXPR, "+", {
							MakeNode(NodeType::IDENTIFIER, "x"),
							MakeNode(NodeType::IDENTIFIER, "y")
						})
					})
				})
			})
		})
	})
	});


const char* source2 = R"(function double compute((int) -> double f) { return f(1,2); })";

auto case2 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "double"),
		MakeNode(NodeType::IDENTIFIER, "compute"),
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::FUNCTION_TYPE, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::BUILTIN_TYPE, "double")
				}),
				MakeNode(NodeType::IDENTIFIER, "f"),
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::RETURN_STMT, "return", {
				MakeNode(NodeType::CALL_EXPR, "", {
					MakeNode(NodeType::IDENTIFIER, "f"),
					MakeNode(NodeType::ARGUMENT_LIST, "", {
						MakeNode(NodeType::LITERAL, "1"),
						MakeNode(NodeType::LITERAL, "2")
					})
				})
			})
		})
	})
	});




const char* source3 = R"(
function int inc(int x) {
    return x + 1;
}

function void main() {
    let int sum = 0;
    let int i = 0;
    while (i < 10) {
        if (i % 2 == 0) {
            sum = sum + inc(i);
        } else {
            continue;
        }
        i = i + 1;
    }
    print(sum);
}
)";

auto case3 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::IDENTIFIER, "inc"),
		MakeNode(NodeType::PARAMETER, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::IDENTIFIER, "x")
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::RETURN_STMT, "return", {
				MakeNode(NodeType::BINARY_EXPR, "+", {
					MakeNode(NodeType::IDENTIFIER, "x"),
					MakeNode(NodeType::LITERAL, "1")
				})
			})
		})
	}),
	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "void"),
		MakeNode(NodeType::IDENTIFIER, "main"),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::VAR_DECL, "let", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "sum"),
					MakeNode(NodeType::LITERAL, "0")
				})
			}),
			MakeNode(NodeType::VAR_DECL, "let", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "i"),
					MakeNode(NodeType::LITERAL, "0")
				})
			}),
			MakeNode(NodeType::WHILE_STMT, "while", {
				MakeNode(NodeType::BINARY_EXPR, "<", {
					MakeNode(NodeType::IDENTIFIER, "i"),
					MakeNode(NodeType::LITERAL, "10")
				}),
				MakeNode(NodeType::COMPOUND_STMT, "", {
					MakeNode(NodeType::IF_STMT, "if", {
						MakeNode(NodeType::BINARY_EXPR, "==", {
							MakeNode(NodeType::BINARY_EXPR, "%", {
								MakeNode(NodeType::IDENTIFIER, "i"),
								MakeNode(NodeType::LITERAL, "2")
							}),
							MakeNode(NodeType::LITERAL, "0")
						}),
						MakeNode(NodeType::COMPOUND_STMT, "", {
							MakeNode(NodeType::ASSIGN_EXPR, "=", {
								MakeNode(NodeType::IDENTIFIER, "sum"),
								MakeNode(NodeType::BINARY_EXPR, "+", {
									MakeNode(NodeType::IDENTIFIER, "sum"),
									MakeNode(NodeType::CALL_EXPR, "", {
										MakeNode(NodeType::IDENTIFIER, "inc"),
										MakeNode(NodeType::IDENTIFIER, "i")
									})
								})
							})
						}),
						MakeNode(NodeType::COMPOUND_STMT, "", {
							MakeNode(NodeType::CONTINUE_STMT,"continue")
						})
					}),
					MakeNode(NodeType::ASSIGN_EXPR, "=", {
						MakeNode(NodeType::IDENTIFIER, "i"),
						MakeNode(NodeType::BINARY_EXPR, "+", {
							MakeNode(NodeType::IDENTIFIER, "i"),
							MakeNode(NodeType::LITERAL, "1")
						})
					})
				})
			}),
			MakeNode(NodeType::CALL_EXPR, "", {
				MakeNode(NodeType::IDENTIFIER, "print"),
				MakeNode(NodeType::IDENTIFIER, "sum")
			})
		})
	})
	});


const char* source4 = R"(
function double square(double x) { return x * x; }

function double test(int n) {
    let double res = 0;
    for (let int i = 0; i < n; i++) {
        if (i > 50) break;
        res = res + square(i+0.5);
    }
    return res;
}
)";

auto case4 = MakeNode(NodeType::PROGRAM, "", {
	// function double square(double x) { return x * x; }
	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "double"),
		MakeNode(NodeType::IDENTIFIER, "square"),
		MakeNode(NodeType::PARAMETER, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "double"),
			MakeNode(NodeType::IDENTIFIER, "x")
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::RETURN_STMT, "return", {
				MakeNode(NodeType::BINARY_EXPR, "*", {
					MakeNode(NodeType::IDENTIFIER, "x"),
					MakeNode(NodeType::IDENTIFIER, "x")
				})
			})
		})
	}),
		// function test(int n) { ... }
		MakeNode(NodeType::FUNCTION_DECL, "function", {
			MakeNode(NodeType::BUILTIN_TYPE, "double"),
			MakeNode(NodeType::IDENTIFIER, "test"),
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::IDENTIFIER, "n")
			}),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				// let double res = 0;
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::BUILTIN_TYPE, "double"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "res"),
						MakeNode(NodeType::LITERAL, "0")
					})
				}),
					// for (int i = 0; i < n; i++) { ... }
					MakeNode(NodeType::FOR_STMT, "for", {
						MakeNode(NodeType::VAR_DECL, "let", {
							MakeNode(NodeType::BUILTIN_TYPE, "int"),
							MakeNode(NodeType::DECLARATOR, "", {
								MakeNode(NodeType::IDENTIFIER, "i"),
								MakeNode(NodeType::LITERAL, "0")
							})
						}),
						MakeNode(NodeType::BINARY_EXPR, "<", {
							MakeNode(NodeType::IDENTIFIER, "i"),
							MakeNode(NodeType::IDENTIFIER, "n")
						}),
						MakeNode(NodeType::POSTFIX_EXPR, "++", {
							MakeNode(NodeType::IDENTIFIER, "i"),
						}),
						MakeNode(NodeType::COMPOUND_STMT, "", {
							MakeNode(NodeType::IF_STMT, "if", {
								MakeNode(NodeType::BINARY_EXPR, ">", {
									MakeNode(NodeType::IDENTIFIER, "i"),
									MakeNode(NodeType::LITERAL, "50")
								}),
								MakeNode(NodeType::BREAK_STMT, "break")
							}),
							MakeNode(NodeType::ASSIGN_EXPR, "=", {
								MakeNode(NodeType::IDENTIFIER, "res"),
								MakeNode(NodeType::BINARY_EXPR, "+", {
									MakeNode(NodeType::IDENTIFIER, "res"),
									MakeNode(NodeType::CALL_EXPR, "", {
										MakeNode(NodeType::IDENTIFIER, "square"),
										MakeNode(NodeType::BINARY_EXPR, "+", {
											MakeNode(NodeType::IDENTIFIER, "i"),
											MakeNode(NodeType::LITERAL, "0.5")
										})
									})
								})
							})
						})
					}),
					// return res;
					MakeNode(NodeType::RETURN_STMT, "return", {
						MakeNode(NodeType::IDENTIFIER, "res")
					})
				})
			})
	});



const char* source5 = R"(
function int max(int a=5,int b=12) {
    if (a > b) return a;
    else return b;
}

function int clamp(int x,int low,int high) {
    if (x < low)
        return low;
    else if (x > high)
        return high;
    else
        return x;
}

function void main() {
    let int a = max();
    a = max(10, 20);
    let int b = clamp(a, 0, 50);
    print(b);
}
)";

auto case5 = MakeNode(NodeType::PROGRAM, "", {
	// --- function int max(int a=5, b=12) ---
	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::IDENTIFIER, "max"),
		MakeNode(NodeType::PARAMETER_LIST, "", {
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "a"),
					MakeNode(NodeType::LITERAL, "5")
				})
			}),
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "b"),
					MakeNode(NodeType::LITERAL, "12")
				})
			})
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::IF_STMT, "if", {
				MakeNode(NodeType::BINARY_EXPR, ">", {
					MakeNode(NodeType::IDENTIFIER, "a"),
					MakeNode(NodeType::IDENTIFIER, "b")
				}),
				MakeNode(NodeType::RETURN_STMT, "return", {
					MakeNode(NodeType::IDENTIFIER, "a")
				}),
				MakeNode(NodeType::RETURN_STMT, "return", {
					MakeNode(NodeType::IDENTIFIER, "b")
				})
			})
		})
	}),

		// --- function int clamp(int x, int low, int high) ---
		MakeNode(NodeType::FUNCTION_DECL, "function", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::IDENTIFIER, "clamp"),
			MakeNode(NodeType::PARAMETER_LIST, "", {
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::IDENTIFIER, "x")
				}),
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::IDENTIFIER, "low")
				}),
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::IDENTIFIER, "high")
				})
			}),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				MakeNode(NodeType::IF_STMT, "if", {
					MakeNode(NodeType::BINARY_EXPR, "<", {
						MakeNode(NodeType::IDENTIFIER, "x"),
						MakeNode(NodeType::IDENTIFIER, "low")
					}),
					MakeNode(NodeType::RETURN_STMT, "return", {
						MakeNode(NodeType::IDENTIFIER, "low")
					}),
					MakeNode(NodeType::IF_STMT, "if", {
						MakeNode(NodeType::BINARY_EXPR, ">", {
							MakeNode(NodeType::IDENTIFIER, "x"),
							MakeNode(NodeType::IDENTIFIER, "high")
						}),
						MakeNode(NodeType::RETURN_STMT, "return", {
							MakeNode(NodeType::IDENTIFIER, "high")
						}),
						MakeNode(NodeType::RETURN_STMT, "return", {
							MakeNode(NodeType::IDENTIFIER, "x")
						})
					})
				})
			})
		}),

		// --- function void main() ---
		MakeNode(NodeType::FUNCTION_DECL, "function", {
			MakeNode(NodeType::BUILTIN_TYPE, "void"),
			MakeNode(NodeType::IDENTIFIER, "main"),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				// let int a = max();
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "a"),
						MakeNode(NodeType::CALL_EXPR, "", {
							MakeNode(NodeType::IDENTIFIER, "max")
						})
					})
				}),

					// a = max(10, 20);
					MakeNode(NodeType::ASSIGN_EXPR, "=", {
						MakeNode(NodeType::IDENTIFIER, "a"),
						MakeNode(NodeType::CALL_EXPR, "", {
							MakeNode(NodeType::IDENTIFIER, "max"),
							MakeNode(NodeType::ARGUMENT_LIST, "", {
								MakeNode(NodeType::LITERAL, "10"),
								MakeNode(NodeType::LITERAL, "20")
							})
						})
					}),

					// let int b = clamp(a, 0, 50);
					MakeNode(NodeType::VAR_DECL, "let", {
						MakeNode(NodeType::BUILTIN_TYPE, "int"),
						MakeNode(NodeType::DECLARATOR, "", {
							MakeNode(NodeType::IDENTIFIER, "b"),
							MakeNode(NodeType::CALL_EXPR, "", {
								MakeNode(NodeType::IDENTIFIER, "clamp"),
								MakeNode(NodeType::ARGUMENT_LIST, "", {
									MakeNode(NodeType::IDENTIFIER, "a"),
									MakeNode(NodeType::LITERAL, "0"),
									MakeNode(NodeType::LITERAL, "50")
								})
							})
						})
					}),

					// print(b);
					MakeNode(NodeType::CALL_EXPR, "", {
						MakeNode(NodeType::IDENTIFIER, "print"),
						MakeNode(NodeType::IDENTIFIER, "b")
					})
				})
			})
	});


const char* source6 = R"(
let int globalVal = 42;
const string message = "Hello";

function string printMessage() {
    print(message);
}

function void main() {
    print(globalVal);
    printMessage();
}
)";

auto case6 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::DECLARATOR, "", {
			MakeNode(NodeType::IDENTIFIER, "globalVal"),
			MakeNode(NodeType::LITERAL, "42"),
		}),
	}),

	MakeNode(NodeType::VAR_DECL, "const", {
		MakeNode(NodeType::BUILTIN_TYPE, "string"),
		MakeNode(NodeType::DECLARATOR, "", {
			MakeNode(NodeType::IDENTIFIER, "message"),
			MakeNode(NodeType::LITERAL, "\"Hello\""),
		}),
	}),

	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "string"),
		MakeNode(NodeType::IDENTIFIER, "printMessage"),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::CALL_EXPR, "", {
				MakeNode(NodeType::IDENTIFIER, "print"),
				MakeNode(NodeType::IDENTIFIER, "message"),
			}),
		}),
	}),

	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "void"),
		MakeNode(NodeType::IDENTIFIER, "main"),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::CALL_EXPR, "", {
				MakeNode(NodeType::IDENTIFIER, "print"),
				MakeNode(NodeType::IDENTIFIER, "globalVal"),
			}),
			MakeNode(NodeType::CALL_EXPR, "",{
				MakeNode(NodeType::IDENTIFIER, "printMessage"),
			}),
		}),
	}),
	});



const char* source7 = R"(
struct Point {
    int x;
    int y;
};

function int sumArray(int arr,int n) {
    let int total = 0;
    for (let int i = 0; i < n; i = i + 1) {
        total = total + arr[i];
        if (arr[i] % 2 == 0) {
            total = total + 1;
        }
    }
    return total;
}

function Point makePoint(int a=0, int b=1) {
    let Point p = Point();
    p.x = a;
    p.y = b;
    return p;
}

function void main() {
    const int data[4] = {1, 2, 3, 4};
    let Point pt = makePoint(sumArray(data, 4), 10);
    print(pt.x + pt.y);
}
)";

auto case7 = MakeNode(NodeType::PROGRAM, "", {
	// --- struct def ---
	MakeNode(NodeType::STRUCT_DECL, "struct", {
		MakeNode(NodeType::IDENTIFIER, "Point"),
		MakeNode(NodeType::STRUCT_DECLARATOR_LIST, "", {
			MakeNode(NodeType::STRUCT_MEMBER_DECL, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::IDENTIFIER, "x"),
			}),
			MakeNode(NodeType::STRUCT_MEMBER_DECL, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::IDENTIFIER, "y"),
			})
		}),
	}),

		// --- sumArray ---
		MakeNode(NodeType::FUNCTION_DECL, "function", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::IDENTIFIER, "sumArray"),
			MakeNode(NodeType::PARAMETER_LIST, "", {
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::IDENTIFIER, "arr"),
				}),
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::IDENTIFIER, "n"),
				})
			}),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "total"),
						MakeNode(NodeType::LITERAL, "0"),
					}),
				}),
				MakeNode(NodeType::FOR_STMT, "for", {
					MakeNode(NodeType::VAR_DECL, "let", {
						MakeNode(NodeType::BUILTIN_TYPE, "int"),
						MakeNode(NodeType::DECLARATOR, "", {
							MakeNode(NodeType::IDENTIFIER, "i"),
							MakeNode(NodeType::LITERAL, "0"),
						}),
					}),
					MakeNode(NodeType::BINARY_EXPR, "<", {
						MakeNode(NodeType::IDENTIFIER, "i"),
						MakeNode(NodeType::IDENTIFIER, "n"),
					}),
					MakeNode(NodeType::ASSIGN_EXPR, "=", {
						MakeNode(NodeType::IDENTIFIER, "i"),
						MakeNode(NodeType::BINARY_EXPR, "+", {
							MakeNode(NodeType::IDENTIFIER, "i"),
							MakeNode(NodeType::LITERAL, "1"),
						}),
					}),
					MakeNode(NodeType::COMPOUND_STMT, "", {
						MakeNode(NodeType::ASSIGN_EXPR, "=", {
							MakeNode(NodeType::IDENTIFIER, "total"),
							MakeNode(NodeType::BINARY_EXPR, "+", {
								MakeNode(NodeType::IDENTIFIER, "total"),
								MakeNode(NodeType::INDEX_EXPR, "", {
									MakeNode(NodeType::IDENTIFIER, "arr"),
									MakeNode(NodeType::IDENTIFIER, "i"),
								}),
							}),
						}),
						MakeNode(NodeType::IF_STMT, "if", {
							MakeNode(NodeType::BINARY_EXPR, "==", {
								MakeNode(NodeType::BINARY_EXPR, "%", {
									MakeNode(NodeType::INDEX_EXPR, "", {
										MakeNode(NodeType::IDENTIFIER, "arr"),
										MakeNode(NodeType::IDENTIFIER, "i"),
									}),
									MakeNode(NodeType::LITERAL, "2"),
								}),
								MakeNode(NodeType::LITERAL, "0"),
							}),
							MakeNode(NodeType::COMPOUND_STMT, "", {
								MakeNode(NodeType::ASSIGN_EXPR, "=", {
									MakeNode(NodeType::IDENTIFIER, "total"),
									MakeNode(NodeType::BINARY_EXPR, "+", {
										MakeNode(NodeType::IDENTIFIER, "total"),
										MakeNode(NodeType::LITERAL, "1"),
									}),
								}),
							}),
						}),
					}),
				}),
				MakeNode(NodeType::RETURN_STMT, "return", {
					MakeNode(NodeType::IDENTIFIER, "total"),
				}),
			}),
		}),

		// --- makePoint ---
		MakeNode(NodeType::FUNCTION_DECL, "function", {
			MakeNode(NodeType::IDENTIFIER, "Point"),
			MakeNode(NodeType::IDENTIFIER, "makePoint"),
			MakeNode(NodeType::PARAMETER_LIST, "", {
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "a"),
						MakeNode(NodeType::LITERAL, "0"),
					})
				}),
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "b"),
						MakeNode(NodeType::LITERAL, "1"),
					})
				}),
			}),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::IDENTIFIER, "Point"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "p"),
						MakeNode(NodeType::CALL_EXPR, "",{
							MakeNode(NodeType::IDENTIFIER, "Point"),
						}),
					}),
				}),
				MakeNode(NodeType::ASSIGN_EXPR, "=", {
					MakeNode(NodeType::MEMBER_EXPR, ".", {
						MakeNode(NodeType::IDENTIFIER, "p"),
						MakeNode(NodeType::IDENTIFIER, "x"),
					}),
					MakeNode(NodeType::IDENTIFIER, "a"),
				}),
				MakeNode(NodeType::ASSIGN_EXPR, "=", {
					MakeNode(NodeType::MEMBER_EXPR, ".", {
						MakeNode(NodeType::IDENTIFIER, "p"),
						MakeNode(NodeType::IDENTIFIER, "y"),
					}),
					MakeNode(NodeType::IDENTIFIER, "b"),
				}),
				MakeNode(NodeType::RETURN_STMT, "return", {
					MakeNode(NodeType::IDENTIFIER, "p"),
				}),
			}),
		}),

		// --- main ---
		MakeNode(NodeType::FUNCTION_DECL, "function", {
			MakeNode(NodeType::BUILTIN_TYPE, "void"),
			MakeNode(NodeType::IDENTIFIER, "main"),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				MakeNode(NodeType::VAR_DECL, "const", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "data"),
						MakeNode(NodeType::ARRAY_SIZE,"",{
							MakeNode(NodeType::LITERAL, "4"),
						}),
						MakeNode(NodeType::INITIALIZER, "", {
							MakeNode(NodeType::LITERAL, "1"),
							MakeNode(NodeType::LITERAL, "2"),
							MakeNode(NodeType::LITERAL, "3"),
							MakeNode(NodeType::LITERAL, "4"),
						}),
					}),
				}),
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::IDENTIFIER, "Point"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "pt"),
						MakeNode(NodeType::CALL_EXPR, "", {
							MakeNode(NodeType::IDENTIFIER, "makePoint"),
							MakeNode(NodeType::ARGUMENT_LIST, "", {
								MakeNode(NodeType::CALL_EXPR, "", {
									MakeNode(NodeType::IDENTIFIER, "sumArray"),
									MakeNode(NodeType::ARGUMENT_LIST, "", {
										MakeNode(NodeType::IDENTIFIER, "data"),
										MakeNode(NodeType::LITERAL, "4"),
									})
								}),
								MakeNode(NodeType::LITERAL, "10")
							})
						})
					}),
				}),
				MakeNode(NodeType::CALL_EXPR, "", {
					MakeNode(NodeType::IDENTIFIER, "print"),
					MakeNode(NodeType::BINARY_EXPR, "+", {
						MakeNode(NodeType::MEMBER_EXPR, ".", {
							MakeNode(NodeType::IDENTIFIER, "pt"),
							MakeNode(NodeType::IDENTIFIER, "x"),
						}),
						MakeNode(NodeType::MEMBER_EXPR, ".", {
							MakeNode(NodeType::IDENTIFIER, "pt"),
							MakeNode(NodeType::IDENTIFIER, "y"),
						}),
					}),
				}),
			}),
		}),
	});





const std::vector<ParserCase> parserComplexCases = {
	{source1, case1},
	{source2, case2},
	{source3, case3},
	{source4, case4},
	{source5, case5},
	{source6, case6},
	{source7, case7},
};

INSTANTIATE_TEST_SUITE_P(ParserComplexSyntax, ParserSyntaxTest, ::testing::ValuesIn(parserComplexCases));

