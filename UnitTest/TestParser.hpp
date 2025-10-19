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

static ExpectedNode MakeNode(NodeType::Type type,
	std::string content = "",
	std::vector<ExpectedNode> children = {}) {
	return { type, std::move(content), std::move(children) };
}

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

const char* source1 = R"(import math;)";

auto case1 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::IMPORT_STMT, "import", {
		MakeNode(NodeType::IDENTIFIER, "math")
	})
	});

const char* source2 = R"(function int add(int a, int b) { return a + b; })";

auto case2 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::IDENTIFIER, "add"),
		MakeNode(NodeType::PARAMETER_LIST, "", {
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "",{
					MakeNode(NodeType::IDENTIFIER, "a"),
					MakeNode(NodeType::ARRAY_SIZE, ""),
					MakeNode(NodeType::INITIALIZER, ""),
				})
			}),
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "",{
					MakeNode(NodeType::IDENTIFIER, "b"),
					MakeNode(NodeType::ARRAY_SIZE, ""),
					MakeNode(NodeType::INITIALIZER, ""),
				})
			})
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::RETURN_STMT, "return", {
				MakeNode(NodeType::BINARY_EXPR, "+", {
					MakeNode(NodeType::IDENTIFIER, "a"),
					MakeNode(NodeType::IDENTIFIER, "b")
				})
			})
		})
	})
	});

const char* source3 = R"(let bool flag = true, tag = false; let Member m = {1,2};)";

auto case3 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::BUILTIN_TYPE, "bool"),
		MakeNode(NodeType::DECLARATOR_LIST, "", {
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "flag"),
				MakeNode(NodeType::ARRAY_SIZE, ""),
				MakeNode(NodeType::LITERAL, "true")
			}),
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "tag"),
				MakeNode(NodeType::ARRAY_SIZE, ""),
				MakeNode(NodeType::LITERAL, "false")

			})
		})
	}),
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::IDENTIFIER, "Member"),
		MakeNode(NodeType::DECLARATOR_LIST, "", {
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "m"),
				MakeNode(NodeType::ARRAY_SIZE, ""),
				MakeNode(NodeType::INITIALIZER, "", {
					MakeNode(NodeType::LITERAL, "1"),
					MakeNode(NodeType::LITERAL, "2")
					})
				})
			})
	})
	});

const char* source4 = R"(let int arr[3] = { "abc", "def", "gij" };)";

auto case4 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::DECLARATOR_LIST, "", {
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "arr"),
				MakeNode(NodeType::ARRAY_SIZE, "", {
					MakeNode(NodeType::LITERAL, "3")
				}),
				MakeNode(NodeType::INITIALIZER,"",{
					MakeNode(NodeType::LITERAL, "\"abc\""),
					MakeNode(NodeType::LITERAL, "\"def\""),
					MakeNode(NodeType::LITERAL, "\"gij\"")
				})
			})
		})
	})
	});

const char* source5 = R"(let int array[3][3] = {{1,2,3},{4,5,6},{7,8,9}}; let empty array = {};)";

auto case5 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::DECLARATOR_LIST, "", {
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "array"),
				MakeNode(NodeType::ARRAY_SIZE, "", {
					MakeNode(NodeType::LITERAL, "3"),
					MakeNode(NodeType::LITERAL, "3")
				}),
				MakeNode(NodeType::INITIALIZER, "", {
					MakeNode(NodeType::INITIALIZER,"",{
						MakeNode(NodeType::LITERAL, "1"),
						MakeNode(NodeType::LITERAL, "2"),
						MakeNode(NodeType::LITERAL, "3")
					}),
					MakeNode(NodeType::INITIALIZER,"",{
						MakeNode(NodeType::LITERAL, "4"),
						MakeNode(NodeType::LITERAL, "5"),
						MakeNode(NodeType::LITERAL, "6")
					}),
					MakeNode(NodeType::INITIALIZER,"",{
						MakeNode(NodeType::LITERAL, "7"),
						MakeNode(NodeType::LITERAL, "8"),
						MakeNode(NodeType::LITERAL, "9")
					}),
				})
			})
		})
	}),
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::IDENTIFIER, "empty"),
		MakeNode(NodeType::DECLARATOR_LIST, "", {
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "array"),
				MakeNode(NodeType::ARRAY_SIZE, ""),
				MakeNode(NodeType::INITIALIZER, "", {})
			})
		})
	})
	});

const char* source6 = R"(struct Point { int x; int y; };)";

auto case6 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::STRUCT_DECL, "struct", {
		MakeNode(NodeType::IDENTIFIER, "Point"),
		MakeNode(NodeType::STRUCT_DECLARATOR_LIST, "", {
			MakeNode(NodeType::STRUCT_MEMBER_DECL, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR_LIST, "", {
					MakeNode(NodeType::DECLARATOR, "",{
						MakeNode(NodeType::IDENTIFIER, "x"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::INITIALIZER, "", {})
					})
				})
			}),
			MakeNode(NodeType::STRUCT_MEMBER_DECL, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR_LIST, "", {
					MakeNode(NodeType::DECLARATOR, "",{
						MakeNode(NodeType::IDENTIFIER, "y"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::INITIALIZER, "", {})
					})
				})
			})
		})
	})
	});

const char* source7 = R"(if (x > 0) { y = 1; } else { y = 2; })";

auto case7 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::IF_STMT, "if", {
		MakeNode(NodeType::BINARY_EXPR, ">", {
			MakeNode(NodeType::IDENTIFIER, "x"),
			MakeNode(NodeType::LITERAL, "0")
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::ASSIGN_EXPR, "=", {
				MakeNode(NodeType::IDENTIFIER, "y"),
				MakeNode(NodeType::LITERAL, "1")
			})
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::ASSIGN_EXPR, "=", {
				MakeNode(NodeType::IDENTIFIER, "y"),
				MakeNode(NodeType::LITERAL, "2")
			})
		})
	})
	});

const char* source8 = R"(while (i < 10) { i = i + 1; continue; })";

auto case8 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::WHILE_STMT, "while", {
		MakeNode(NodeType::BINARY_EXPR, "<", {
			MakeNode(NodeType::IDENTIFIER, "i"),
			MakeNode(NodeType::LITERAL, "10")
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::ASSIGN_EXPR, "=", {
				MakeNode(NodeType::IDENTIFIER, "i"),
				MakeNode(NodeType::BINARY_EXPR, "+", {
					MakeNode(NodeType::IDENTIFIER, "i"),
					MakeNode(NodeType::LITERAL, "1")
				})
			}),
			MakeNode(NodeType::CONTINUE_STMT, "continue")
		})
	})
	});

const char* source9 = R"(switch(x) { case 1: y = 2; break; default: y = 0; })";

auto case9 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::SWITCH_STMT, "switch", {
		MakeNode(NodeType::IDENTIFIER, "x"),
		MakeNode(NodeType::CASE_CLAUSE, "case", {
			MakeNode(NodeType::LITERAL, "1"),
			MakeNode(NodeType::ASSIGN_EXPR, "=", {
				MakeNode(NodeType::IDENTIFIER, "y"),
				MakeNode(NodeType::LITERAL, "2")
			}),
			MakeNode(NodeType::BREAK_STMT, "break")
		}),
		MakeNode(NodeType::DEFAULT_CLAUSE, "default", {
			MakeNode(NodeType::ASSIGN_EXPR, "=", {
				MakeNode(NodeType::IDENTIFIER, "y"),
				MakeNode(NodeType::LITERAL, "0")
			})
		})
	})
	});

const char* source10 = R"(for (i = 0; i < 10; i = i + 1) sum = sum + i;)";

auto case10 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::FOR_STMT, "for", {
		MakeNode(NodeType::ASSIGN_EXPR, "=", {
			MakeNode(NodeType::IDENTIFIER, "i"),
			MakeNode(NodeType::LITERAL, "0")
		}),
		MakeNode(NodeType::BINARY_EXPR, "<", {
			MakeNode(NodeType::IDENTIFIER, "i"),
			MakeNode(NodeType::LITERAL, "10")
		}),
		MakeNode(NodeType::ASSIGN_EXPR, "=", {
			MakeNode(NodeType::IDENTIFIER, "i"),
			MakeNode(NodeType::BINARY_EXPR, "+", {
				MakeNode(NodeType::IDENTIFIER, "i"),
				MakeNode(NodeType::LITERAL, "1")
			})
		}),
		MakeNode(NodeType::ASSIGN_EXPR, "=", {
			MakeNode(NodeType::IDENTIFIER, "sum"),
			MakeNode(NodeType::BINARY_EXPR, "+", {
				MakeNode(NodeType::IDENTIFIER, "sum"),
				MakeNode(NodeType::IDENTIFIER, "i")
			})
		})
	})
	});

const char* source11 = R"(a = x > 0 ? 1 : 2;)";

auto case11 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::ASSIGN_EXPR, "=", {
		MakeNode(NodeType::IDENTIFIER, "a"),
		MakeNode(NodeType::CONDITIONAL_EXPR, "?", {
			MakeNode(NodeType::BINARY_EXPR, ">", {
				MakeNode(NodeType::IDENTIFIER, "x"),
				MakeNode(NodeType::LITERAL, "0")
			}),
			MakeNode(NodeType::LITERAL, "1"),
			MakeNode(NodeType::LITERAL, "2")
		})
	})
	});

const char* source12 = R"(obj.list[i++].value = -10;)";

auto case12 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::ASSIGN_EXPR, "=", {
		MakeNode(NodeType::MEMBER_EXPR, ".", {
			MakeNode(NodeType::INDEX_EXPR, "", {
				MakeNode(NodeType::MEMBER_EXPR, ".", {
					MakeNode(NodeType::IDENTIFIER, "obj"),
					MakeNode(NodeType::IDENTIFIER, "list")
				}),
				MakeNode(NodeType::POSTFIX_EXPR, "++", {
					MakeNode(NodeType::IDENTIFIER, "i")
				})
			}),
			MakeNode(NodeType::IDENTIFIER, "value")
		}),
		MakeNode(NodeType::UNARY_EXPR, "-", {
			MakeNode(NodeType::LITERAL, "10")
		})
	})
	});

const char* source13 = R"((a + b) * (c - (d + e));)";

auto case13 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::BINARY_EXPR, "*", {
		MakeNode(NodeType::GROUP_EXPR, "", {
			MakeNode(NodeType::BINARY_EXPR, "+", {
				MakeNode(NodeType::IDENTIFIER, "a"),
				MakeNode(NodeType::IDENTIFIER, "b")
			})
		}),
		MakeNode(NodeType::GROUP_EXPR, "", {
			MakeNode(NodeType::BINARY_EXPR, "-", {
				MakeNode(NodeType::IDENTIFIER, "c"),
				MakeNode(NodeType::GROUP_EXPR, "", {
					MakeNode(NodeType::BINARY_EXPR, "+", {
						MakeNode(NodeType::IDENTIFIER, "d"),
						MakeNode(NodeType::IDENTIFIER, "e")
					})
				})
			})
		})
	})
	});

const char* source14 = R"(foo(1, 2);)";

auto case14 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::CALL_EXPR, "", {
		MakeNode(NodeType::IDENTIFIER, "foo"),
		MakeNode(NodeType::ARGUMENT_LIST, "", {
			MakeNode(NodeType::LITERAL, "1"),
			MakeNode(NodeType::LITERAL, "2")
		})
	})
	});

const char* source15 = R"(;)";

auto case15 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::EMPTY_STMT, "", {})
	});




const char* source16 = R"(let (int, int) -> int f = lambda(int x, int y) -> int { return x + y; };)";

auto case16 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::FUNCTION_TYPE, "", {
			MakeNode(NodeType::PARAMETER_TYPE_LIST, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::BUILTIN_TYPE, "int")
			}),
			MakeNode(NodeType::BUILTIN_TYPE, "int")
		}),
		MakeNode(NodeType::DECLARATOR_LIST, "", {
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "f"),
				MakeNode(NodeType::ARRAY_SIZE,""),
				MakeNode(NodeType::FUNCTION_LITERAL, "", {
					MakeNode(NodeType::PARAMETER_LIST, "", {
						MakeNode(NodeType::PARAMETER, "", {
							MakeNode(NodeType::BUILTIN_TYPE, "int"),
							MakeNode(NodeType::DECLARATOR, "",{
								MakeNode(NodeType::IDENTIFIER, "x"),
								MakeNode(NodeType::ARRAY_SIZE, ""),
								MakeNode(NodeType::INITIALIZER, "", {})
							}),

						}),
						MakeNode(NodeType::PARAMETER, "", {
							MakeNode(NodeType::BUILTIN_TYPE, "int"),
							MakeNode(NodeType::DECLARATOR, "",{
								MakeNode(NodeType::IDENTIFIER, "y"),
								MakeNode(NodeType::ARRAY_SIZE, ""),
								MakeNode(NodeType::INITIALIZER, "", {})
							})
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
	})
	});


const char* source17 = R"(function double compute((int) -> double f) { return f(1,2); })";

auto case17 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "double"),
		MakeNode(NodeType::IDENTIFIER, "compute"),
			MakeNode(NodeType::PARAMETER_LIST, "", {
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::FUNCTION_TYPE, "", {
						MakeNode(NodeType::PARAMETER_TYPE_LIST, "", {
							MakeNode(NodeType::BUILTIN_TYPE, "int"),
						}),
						MakeNode(NodeType::BUILTIN_TYPE, "double"),
					}),
						MakeNode(NodeType::DECLARATOR, "",{
							MakeNode(NodeType::IDENTIFIER, "f"),
							MakeNode(NodeType::ARRAY_SIZE, ""),
							MakeNode(NodeType::INITIALIZER, "", {})
						})
				})
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


const char* source18 = R"(
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

auto case18 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::IDENTIFIER, "inc"),
		MakeNode(NodeType::PARAMETER_LIST, "", {
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "",{
					MakeNode(NodeType::IDENTIFIER, "x"),
					MakeNode(NodeType::ARRAY_SIZE, ""),
					MakeNode(NodeType::INITIALIZER, "", {})
				})
			}),
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
		MakeNode(NodeType::PARAMETER_LIST, ""),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::VAR_DECL, "let", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR_LIST, "", {
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "sum"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::LITERAL, "0")
					})
				})
			}),
			MakeNode(NodeType::VAR_DECL, "let", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR_LIST, "", {
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "i"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::LITERAL, "0")
					})
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
										MakeNode(NodeType::ARGUMENT_LIST, "", {
											MakeNode(NodeType::IDENTIFIER, "i")
										})
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
				MakeNode(NodeType::ARGUMENT_LIST, "", {
					MakeNode(NodeType::IDENTIFIER, "sum")
				})
			})
		})
	})
	});


const char* source19 = R"(
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

auto case19 = MakeNode(NodeType::PROGRAM, "", {
	// function double square(double x) { return x * x; }
	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "double"),
		MakeNode(NodeType::IDENTIFIER, "square"),
		MakeNode(NodeType::PARAMETER_LIST, "", {
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "double"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "x"),
					MakeNode(NodeType::ARRAY_SIZE, ""),
					MakeNode(NodeType::INITIALIZER, "")
				})
			})
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
			MakeNode(NodeType::PARAMETER_LIST, "", {
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "n"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::INITIALIZER, "")
					})
				})
			}),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				// let double res = 0;
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::BUILTIN_TYPE, "double"),
					MakeNode(NodeType::DECLARATOR_LIST, "", {
						MakeNode(NodeType::DECLARATOR, "", {
							MakeNode(NodeType::IDENTIFIER, "res"),
							MakeNode(NodeType::ARRAY_SIZE, ""),
							MakeNode(NodeType::LITERAL, "0"),
						})
					})
				}),
					// for (int i = 0; i < n; i++) { ... }
					MakeNode(NodeType::FOR_STMT, "for", {
						MakeNode(NodeType::VAR_DECL, "let", {
							MakeNode(NodeType::BUILTIN_TYPE, "int"),
							MakeNode(NodeType::DECLARATOR_LIST, "", {
								MakeNode(NodeType::DECLARATOR, "", {
									MakeNode(NodeType::IDENTIFIER, "i"),
									MakeNode(NodeType::ARRAY_SIZE, ""),
									MakeNode(NodeType::LITERAL, "0")
								})
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
								MakeNode(NodeType::BREAK_STMT, "break"),
								MakeNode(NodeType::EMPTY_STMT, "")
							}),
							MakeNode(NodeType::ASSIGN_EXPR, "=", {
								MakeNode(NodeType::IDENTIFIER, "res"),
								MakeNode(NodeType::BINARY_EXPR, "+", {
									MakeNode(NodeType::IDENTIFIER, "res"),
									MakeNode(NodeType::CALL_EXPR, "", {
										MakeNode(NodeType::IDENTIFIER, "square"),
										MakeNode(NodeType::ARGUMENT_LIST, "", {
											MakeNode(NodeType::BINARY_EXPR, "+", {
												MakeNode(NodeType::IDENTIFIER, "i"),
												MakeNode(NodeType::LITERAL, "0.5")
											})
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



const char* source20 = R"(
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

auto case20 = MakeNode(NodeType::PROGRAM, "", {
	// --- function int max(int a=5,int b=12) ---
	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::IDENTIFIER, "max"),
		MakeNode(NodeType::PARAMETER_LIST, "", {
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "a"),
					MakeNode(NodeType::ARRAY_SIZE, ""),
					MakeNode(NodeType::LITERAL, "5")
				})
			}),
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "b"),
					MakeNode(NodeType::ARRAY_SIZE, ""),
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
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "x"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::INITIALIZER, "")
					})
				}),
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "low"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::INITIALIZER, "")
					})
				}),
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "high"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::INITIALIZER, "")
					})
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
			MakeNode(NodeType::PARAMETER_LIST, ""),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				// let int a = max();
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR_LIST, "", {
						MakeNode(NodeType::DECLARATOR, "", {
							MakeNode(NodeType::IDENTIFIER, "a"),
							MakeNode(NodeType::ARRAY_SIZE, ""),
							MakeNode(NodeType::CALL_EXPR, "", {
								MakeNode(NodeType::IDENTIFIER, "max"),
								MakeNode(NodeType::ARGUMENT_LIST, "", {})
							})
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
						MakeNode(NodeType::DECLARATOR_LIST, "", {
							MakeNode(NodeType::DECLARATOR, "", {
								MakeNode(NodeType::IDENTIFIER, "b"),
								MakeNode(NodeType::ARRAY_SIZE, ""),
								MakeNode(NodeType::CALL_EXPR, "", {
									MakeNode(NodeType::IDENTIFIER, "clamp"),
									MakeNode(NodeType::ARGUMENT_LIST, "", {
										MakeNode(NodeType::IDENTIFIER, "a"),
										MakeNode(NodeType::LITERAL, "0"),
										MakeNode(NodeType::LITERAL, "50")
									})
								})
							})
						})
					}),

					// print(b);
					MakeNode(NodeType::CALL_EXPR, "", {
						MakeNode(NodeType::IDENTIFIER, "print"),
						MakeNode(NodeType::ARGUMENT_LIST, "", {
							MakeNode(NodeType::IDENTIFIER, "b")
						})
					})
				})
			})
	});


const char* source21 = R"(
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

auto case21 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::DECLARATOR_LIST, "", {
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "globalVal"),
				MakeNode(NodeType::ARRAY_SIZE, ""),
				MakeNode(NodeType::LITERAL, "42"),
			})
		})
	}),

	MakeNode(NodeType::VAR_DECL, "const", {
		MakeNode(NodeType::BUILTIN_TYPE, "string"),
		MakeNode(NodeType::DECLARATOR_LIST, "", {
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "message"),
				MakeNode(NodeType::ARRAY_SIZE, ""),
				MakeNode(NodeType::LITERAL, "\"Hello\""),
			})
		})
	}),

	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "string"),
		MakeNode(NodeType::IDENTIFIER, "printMessage"),
		MakeNode(NodeType::PARAMETER_LIST, ""),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::CALL_EXPR, "", {
				MakeNode(NodeType::IDENTIFIER, "print"),
				MakeNode(NodeType::ARGUMENT_LIST, "", {
					MakeNode(NodeType::IDENTIFIER, "message")
				})
			}),
		}),
	}),

	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "void"),
		MakeNode(NodeType::IDENTIFIER, "main"),
		MakeNode(NodeType::PARAMETER_LIST, ""),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::CALL_EXPR, "", {
				MakeNode(NodeType::IDENTIFIER, "print"),
				MakeNode(NodeType::ARGUMENT_LIST, "", {
					MakeNode(NodeType::IDENTIFIER, "globalVal"),
				})
			}),
			MakeNode(NodeType::CALL_EXPR, "",{
				MakeNode(NodeType::IDENTIFIER, "printMessage"),
				MakeNode(NodeType::ARGUMENT_LIST, ""),
			}),
		}),
	}),
	});



const char* source22 = R"(
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

auto case22 = MakeNode(NodeType::PROGRAM, "", {
	// --- struct def ---
	MakeNode(NodeType::STRUCT_DECL, "struct", {
		MakeNode(NodeType::IDENTIFIER, "Point"),
		MakeNode(NodeType::STRUCT_DECLARATOR_LIST, "", {
			MakeNode(NodeType::STRUCT_MEMBER_DECL, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR_LIST, "",{
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "x"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::INITIALIZER, ""),
					})
				})
			}),
			MakeNode(NodeType::STRUCT_MEMBER_DECL, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR_LIST, "",{
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "y"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::INITIALIZER, ""),
					})
				})
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
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "arr"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::INITIALIZER, ""),
					})
				}),
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "n"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::INITIALIZER, ""),
					})
				})
			}),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR_LIST, "", {
						MakeNode(NodeType::DECLARATOR, "", {
							MakeNode(NodeType::IDENTIFIER, "total"),
							MakeNode(NodeType::ARRAY_SIZE, ""),
							MakeNode(NodeType::LITERAL, "0"),
						})
					})
				}),
				MakeNode(NodeType::FOR_STMT, "for", {
					MakeNode(NodeType::VAR_DECL, "let", {
						MakeNode(NodeType::BUILTIN_TYPE, "int"),
						MakeNode(NodeType::DECLARATOR_LIST, "", {
							MakeNode(NodeType::DECLARATOR, "", {
								MakeNode(NodeType::IDENTIFIER, "i"),
								MakeNode(NodeType::ARRAY_SIZE, ""),
								MakeNode(NodeType::LITERAL, "0"),
							})
						})
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
							MakeNode(NodeType::EMPTY_STMT, "", {})
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
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::LITERAL, "0"),
					})
				}),
				MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "b"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::LITERAL, "1"),
					})
				}),
			}),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::IDENTIFIER, "Point"),
					MakeNode(NodeType::DECLARATOR_LIST, "", {
						MakeNode(NodeType::DECLARATOR, "", {
							MakeNode(NodeType::IDENTIFIER, "p"),
							MakeNode(NodeType::ARRAY_SIZE, ""),
							MakeNode(NodeType::CALL_EXPR, "",{
								MakeNode(NodeType::IDENTIFIER, "Point"),
								MakeNode(NodeType::ARGUMENT_LIST, ""),
							}),
						})
					})
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
			MakeNode(NodeType::PARAMETER_LIST, ""),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				MakeNode(NodeType::VAR_DECL, "const", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR_LIST, "", {
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
					})
				}),
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::IDENTIFIER, "Point"),
					MakeNode(NodeType::DECLARATOR_LIST, "", {
						MakeNode(NodeType::DECLARATOR, "", {
							MakeNode(NodeType::IDENTIFIER, "pt"),
							MakeNode(NodeType::ARRAY_SIZE, ""),
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
						})
					})
				}),
				MakeNode(NodeType::CALL_EXPR, "", {
					MakeNode(NodeType::IDENTIFIER, "print"),
					MakeNode(NodeType::ARGUMENT_LIST, "",{
						MakeNode(NodeType::BINARY_EXPR, "+", {
						MakeNode(NodeType::MEMBER_EXPR, ".", {
							MakeNode(NodeType::IDENTIFIER, "pt"),
							MakeNode(NodeType::IDENTIFIER, "x"),
						}),
						MakeNode(NodeType::MEMBER_EXPR, ".", {
							MakeNode(NodeType::IDENTIFIER, "pt"),
							MakeNode(NodeType::IDENTIFIER, "y"),
						})
					})
					}),
				}),
			}),
		}),
	});

auto source23 = R"(
function void main() {
	let int a = 5;
	let int b = 10;
	if (a < b) {
		if (a > 0) {
			print(a);
		} else {
			print(b);
		}
	} else {
		print(0);
	}
}
)";

auto case23 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::FUNCTION_DECL, "function", {
		MakeNode(NodeType::BUILTIN_TYPE, "void"),
		MakeNode(NodeType::IDENTIFIER, "main"),
		MakeNode(NodeType::PARAMETER_LIST, ""),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			// let int a = 5;
			MakeNode(NodeType::VAR_DECL, "let", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR_LIST, "", {
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "a"),
						MakeNode(NodeType::ARRAY_SIZE, ""),
						MakeNode(NodeType::LITERAL, "5"),
					})
				})
			}),
				// let int b = 10;
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR_LIST, "", {
						MakeNode(NodeType::DECLARATOR, "", {
							MakeNode(NodeType::IDENTIFIER, "b"),
							MakeNode(NodeType::ARRAY_SIZE, ""),
							MakeNode(NodeType::LITERAL, "10"),
						})
					})
				}),
				// if (a < b)
				MakeNode(NodeType::IF_STMT, "if", {
				// condition: a < b
				MakeNode(NodeType::BINARY_EXPR, "<", {
					MakeNode(NodeType::IDENTIFIER, "a"),
					MakeNode(NodeType::IDENTIFIER, "b"),
				}),
				// then block
				MakeNode(NodeType::COMPOUND_STMT, "", {
					// inner if (a > 0)
					MakeNode(NodeType::IF_STMT, "if", {
						MakeNode(NodeType::BINARY_EXPR, ">", {
							MakeNode(NodeType::IDENTIFIER, "a"),
							MakeNode(NodeType::LITERAL, "0"),
						}),
						MakeNode(NodeType::COMPOUND_STMT, "", {
							MakeNode(NodeType::CALL_EXPR, "", {
								MakeNode(NodeType::IDENTIFIER, "print"),
								MakeNode(NodeType::ARGUMENT_LIST, "", {
									MakeNode(NodeType::IDENTIFIER, "a"),
								}),
							}),
						}),
						MakeNode(NodeType::COMPOUND_STMT, "", {
							MakeNode(NodeType::CALL_EXPR, "", {
								MakeNode(NodeType::IDENTIFIER, "print"),
								MakeNode(NodeType::ARGUMENT_LIST, "", {
									MakeNode(NodeType::IDENTIFIER, "b"),
								}),
							}),
						}),
					}),
				}),
				// else block
				MakeNode(NodeType::COMPOUND_STMT, "", {
					MakeNode(NodeType::CALL_EXPR, "", {
						MakeNode(NodeType::IDENTIFIER, "print"),
						MakeNode(NodeType::ARGUMENT_LIST, "", {
							MakeNode(NodeType::LITERAL, "0"),
						}),
					}),
				}),
			}),
		}),
	}),
	});



const std::vector<ParserCase> parserCases = {
	{source1, case1},
	{source2, case2},
	{source3, case3},
	{source4, case4},
	{source5, case5},
	{source6, case6},
	{source7, case7},
	{source8, case8},
	{source9, case9},
	{source10, case10},
	{source11, case11},
	{source12, case12},
	{source13, case13},
	{source14, case14},
	{source15, case15},
	{source16, case16},
	{source17, case17},
	{source18, case18},
	{source19, case19},
	{source20, case20},
	{source21, case21},
	{source22, case22},
	{source23, case23},
};

INSTANTIATE_TEST_SUITE_P(ParserSyntax, ParserSyntaxTest, ::testing::ValuesIn(parserCases));

