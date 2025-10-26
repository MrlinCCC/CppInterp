#include "gtest/gtest.h"
#include <Parser.h>

using namespace CppInterp;

struct ExpectedNode {
	NodeType::Type type;
	std::string content;
	std::vector<ExpectedNode> children;
};

class AstCompareVisitor : public AstVisitor {
public:
	const ExpectedNode* currentExpected;
	std::string path;

	AstCompareVisitor(const ExpectedNode* expected, std::string path = "")
		: currentExpected(expected), path(std::move(path)) {
	}

	void CheckNodeType(const AstNode* node) {
		EXPECT_EQ(node->m_nodeType, currentExpected->type)
			<< "Type mismatch at path: " << path;
	}

	void Visit(ProgramNode& node) override {
		CheckNodeType(&node);
		for (size_t i = 0; i < node.m_declarations.size(); ++i) {
			AstCompareVisitor v(&currentExpected->children[i], path + "/stmt" + std::to_string(i));
			node.m_declarations[i]->Accept(v);
			EXPECT_EQ(node.m_declarations[i]->m_parent, &node);
		}
		EXPECT_EQ(node.m_declarations.size(), currentExpected->children.size())
			<< "Child count mismatch at path: " << path;
	}

	void Visit(ImportNode& node) override {
		CheckNodeType(&node);
		EXPECT_EQ(node.m_moduleName, currentExpected->content)
			<< "Module mismatch at path: " << path;
		bool isStringLiteral = currentExpected->type == NodeType::LITERAL ? true : false;
		EXPECT_EQ(node.m_isStringLiteral, isStringLiteral)
			<< "Import module type not match at path:" << path;
		EXPECT_TRUE(currentExpected->children.empty())
			<< "Child is not empty at path: " << path;
	}

	void Visit(FunctionDeclNode& node) override {
		CheckNodeType(&node);
		int cur = 0;
		if (node.m_returnType) {
			AstCompareVisitor v1(&currentExpected->children[0], path + "/returnType");
			node.m_returnType->Accept(v1);
			EXPECT_EQ(node.m_name->m_parent, &node);
			cur++;
		}
		if (node.m_name) {
			AstCompareVisitor v2(&currentExpected->children[1], path + "/name");
			node.m_name->Accept(v2);
			EXPECT_EQ(node.m_name->m_parent, &node);
			cur++;
		}
		for (size_t i = 0; i < node.m_params.size(); ++i) {
			AstCompareVisitor v(&currentExpected->children[i + cur], path + "/param" + std::to_string(i));
			node.m_params[i]->Accept(v);
			EXPECT_EQ(node.m_params[i]->m_parent, &node);
		}
		cur += node.m_params.size();
		if (node.m_body) {
			AstCompareVisitor v(&currentExpected->children.back(), path + "/body");
			node.m_body->Accept(v);
			EXPECT_EQ(node.m_body->m_parent, &node);
			cur++;
		}
		EXPECT_EQ(cur, currentExpected->children.size())
			<< "Child count mismatch at path: " << path;
	}

	void Visit(CompoundStmtNode& node) override {
		CheckNodeType(&node);
		for (size_t i = 0; i < node.m_statements.size(); ++i) {
			AstCompareVisitor v(&currentExpected->children[i], path + "/stmt" + std::to_string(i));
			node.m_statements[i]->Accept(v);
			EXPECT_EQ(node.m_statements[i]->m_parent, &node);
		}
		EXPECT_EQ(node.m_statements.size(), currentExpected->children.size())
			<< "Child count mismatch at path: " << path;
	}

	void Visit(ExpressionStmtNode& node) override {
		CheckNodeType(&node);
		if (node.m_expression) {
			AstCompareVisitor v(&currentExpected->children[0], path + "/expr");
			node.m_expression->Accept(v);
			EXPECT_EQ(node.m_expression->m_parent, &node);
			EXPECT_EQ(currentExpected->children.size(), 1)
				<< "Child count mismatch at path: " << path;
		}
	}

	void Visit(VariableDeclNode& node) override {
		CheckNodeType(&node);
		bool isConst = currentExpected->content == "const" ? true : false;
		EXPECT_EQ(node.m_isConst, isConst) << "VariableDecl type mismatch at path:" << path;
		AstCompareVisitor v(&currentExpected->children[0], path + "/type");
		node.m_type->Accept(v);
		EXPECT_EQ(node.m_type->m_parent, &node);
		for (size_t i = 0; i < node.m_declarators.size(); ++i) {
			AstCompareVisitor v(&currentExpected->children[i + 1], path + "/init" + std::to_string(i));
			node.m_declarators[i]->Accept(v);
			EXPECT_EQ(node.m_declarators[i]->m_parent, &node);
		}
		EXPECT_EQ(node.m_declarators.size(), currentExpected->children.size() - 1)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(StructDeclNode& node) override {
		CheckNodeType(&node);
		EXPECT_TRUE(node.m_name);
		AstCompareVisitor v2(&currentExpected->children[0], path + "/name");
		node.m_name->Accept(v2);
		EXPECT_EQ(node.m_name->m_parent, &node);
		for (size_t i = 0; i < node.m_members.size(); ++i) {
			AstCompareVisitor v(&currentExpected->children[i + 1], path + "/member" + std::to_string(i));
			node.m_members[i]->Accept(v);
			EXPECT_EQ(node.m_members[i]->m_parent, &node);
		}
		EXPECT_EQ(node.m_members.size(), currentExpected->children.size() - 1)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(IfStmtNode& node) override {
		CheckNodeType(&node);
		EXPECT_TRUE(node.m_condition);
		AstCompareVisitor v(&currentExpected->children[0], path + "/cond");
		node.m_condition->Accept(v);
		EXPECT_EQ(node.m_condition->m_parent, &node);
		EXPECT_TRUE(node.m_thenStmt);
		AstCompareVisitor v1(&currentExpected->children[1], path + "/then");
		node.m_thenStmt->Accept(v1);
		EXPECT_EQ(node.m_thenStmt->m_parent, &node);
		if (node.m_elseStmt && currentExpected->children.size() == 3) {
			AstCompareVisitor v2(&currentExpected->children[2], path + "/else");
			node.m_elseStmt->Accept(v2);
		}
		EXPECT_TRUE((currentExpected->children.size() == 2 && !node.m_elseStmt)
			|| (currentExpected->children.size() == 3 && node.m_elseStmt))
			<< "Child count mismatch at path: " << path;
	}

	void Visit(SwitchStmtNode& node) override {
		CheckNodeType(&node);
		int cur = 0;
		EXPECT_TRUE(node.m_condition);
		AstCompareVisitor v(&currentExpected->children[0], path + "/cond");
		node.m_condition->Accept(v);
		EXPECT_EQ(node.m_condition->m_parent, &node);
		cur++;
		for (size_t i = 0; i < node.m_cases.size(); ++i) {
			AstCompareVisitor v1(&currentExpected->children[i + 1], path + "/case" + std::to_string(i));
			node.m_cases[i]->Accept(v1);
			EXPECT_EQ(node.m_cases[i]->m_parent, &node);
		}
		cur += node.m_cases.size();
		if (node.m_default) {
			AstCompareVisitor v2(&currentExpected->children.back(), path + "/default");
			node.m_default->Accept(v2);
			cur++;
		}
		EXPECT_EQ(cur, currentExpected->children.size())
			<< "Child count mismatch at path: " << path;
	}

	void Visit(CaseNode& node) override {
		CheckNodeType(&node);
		EXPECT_TRUE(node.m_literal);
		AstCompareVisitor v1(&currentExpected->children[0], path + "/literal");
		node.m_literal->Accept(v1);
		EXPECT_EQ(node.m_literal->m_parent, &node);
		for (size_t i = 0; i < node.m_statements.size(); ++i) {
			AstCompareVisitor v2(&currentExpected->children[i + 1], path + "/statements" + std::to_string(i));
			node.m_statements[i]->Accept(v2);
			EXPECT_EQ(node.m_statements[i]->m_parent, &node);
		}
		EXPECT_EQ(node.m_statements.size(), currentExpected->children.size() - 1)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(DefaultNode& node) override {
		CheckNodeType(&node);
		for (size_t i = 0; i < node.m_statements.size(); ++i) {
			AstCompareVisitor v1(&currentExpected->children[i], path + "/statements" + std::to_string(i));
			node.m_statements[i]->Accept(v1);
			EXPECT_EQ(node.m_statements[i]->m_parent, &node);
		}
		EXPECT_EQ(node.m_statements.size(), currentExpected->children.size())
			<< "Child count mismatch at path: " << path;
	}

	void Visit(WhileStmtNode& node) override {
		CheckNodeType(&node);
		EXPECT_TRUE(node.m_condition);
		AstCompareVisitor v1(&currentExpected->children[0], path + "/cond");
		node.m_condition->Accept(v1);
		EXPECT_EQ(node.m_condition->m_parent, &node);
		EXPECT_TRUE(node.m_body);
		AstCompareVisitor v2(&currentExpected->children[1], path + "/body");
		node.m_body->Accept(v2);
		EXPECT_EQ(node.m_body->m_parent, &node);
		EXPECT_EQ(currentExpected->children.size(), 2)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(ForStmtNode& node) override {
		CheckNodeType(&node);
		int cur = 0;
		if (node.m_init) {
			AstCompareVisitor v1(&currentExpected->children[0], path + "/init");
			node.m_init->Accept(v1);
			EXPECT_EQ(node.m_init->m_parent, &node);
			cur++;
		}
		if (node.m_condition) {
			AstCompareVisitor v2(&currentExpected->children[1], path + "/cond");
			node.m_condition->Accept(v2);
			EXPECT_EQ(node.m_condition->m_parent, &node);
			cur++;
		}
		if (node.m_increment) {
			AstCompareVisitor v3(&currentExpected->children[2], path + "/inc");
			node.m_increment->Accept(v3);
			EXPECT_EQ(node.m_increment->m_parent, &node);
			cur++;
		}
		EXPECT_TRUE(node.m_body);
		AstCompareVisitor v4(&currentExpected->children[3], path + "/body");
		node.m_body->Accept(v4);
		EXPECT_EQ(node.m_body->m_parent, &node);
		cur++;
		EXPECT_EQ(cur, currentExpected->children.size())
			<< "Child count mismatch at path: " << path;
	}

	void Visit(ReturnStmtNode& node) override {
		CheckNodeType(&node);
		if (node.m_expression) {
			AstCompareVisitor v1(&currentExpected->children[0], path + "/expr");
			node.m_expression->Accept(v1);
			EXPECT_EQ(node.m_expression->m_parent, &node);
			EXPECT_EQ(currentExpected->children.size(), 1)
				<< "Child count mismatch at path: " << path;
		}
		else
			EXPECT_TRUE(currentExpected->children.empty())
			<< "Child is not empty at path: " << path;
	}

	void Visit(BreakStmtNode& node) override { CheckNodeType(&node); EXPECT_TRUE(currentExpected->children.empty()); }
	void Visit(ContinueStmtNode& node) override { CheckNodeType(&node); EXPECT_TRUE(currentExpected->children.empty()); }

	void Visit(CommaExprNode& node) override {
		CheckNodeType(&node);
		for (size_t i = 0; i < node.m_expressions.size(); ++i) {
			AstCompareVisitor v1(&currentExpected->children[i], path + "/expr" + std::to_string(i));
			node.m_expressions[i]->Accept(v1);
			EXPECT_EQ(node.m_expressions[i]->m_parent, &node);
		}
		EXPECT_EQ(node.m_expressions.size(), currentExpected->children.size())
			<< "Child count mismatch at path: " << path;
	}

	void Visit(AssignmentExprNode& node) override {
		CheckNodeType(&node);
		EXPECT_EQ(node.m_op, currentExpected->content);
		EXPECT_TRUE(node.m_left);
		AstCompareVisitor v1(&currentExpected->children[0], path + "/left");
		node.m_left->Accept(v1);
		EXPECT_EQ(node.m_left->m_parent, &node);
		EXPECT_TRUE(node.m_right);
		AstCompareVisitor v2(&currentExpected->children[1], path + "/right");
		node.m_right->Accept(v2);
		EXPECT_EQ(node.m_right->m_parent, &node);
		EXPECT_EQ(currentExpected->children.size(), 2)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(ConditionalExprNode& node) override {
		CheckNodeType(&node);
		EXPECT_TRUE(node.m_condition);
		AstCompareVisitor v1(&currentExpected->children[0], path + "/cond");
		node.m_condition->Accept(v1);
		EXPECT_EQ(node.m_condition->m_parent, &node);
		EXPECT_TRUE(node.m_trueExpr);
		AstCompareVisitor v2(&currentExpected->children[1], path + "/true");
		node.m_trueExpr->Accept(v2);
		EXPECT_EQ(node.m_trueExpr->m_parent, &node);
		EXPECT_TRUE(node.m_falseExpr);
		AstCompareVisitor v3(&currentExpected->children[2], path + "/false");
		node.m_falseExpr->Accept(v3);
		EXPECT_EQ(node.m_falseExpr->m_parent, &node);
		EXPECT_EQ(currentExpected->children.size(), 3)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(BinaryExprNode& node) override {
		CheckNodeType(&node);
		EXPECT_TRUE(node.m_left);
		AstCompareVisitor v1(&currentExpected->children[0], path + "/left");
		node.m_left->Accept(v1);
		EXPECT_EQ(node.m_left->m_parent, &node);
		EXPECT_TRUE(node.m_right);
		AstCompareVisitor v2(&currentExpected->children[1], path + "/right");
		node.m_right->Accept(v2);
		EXPECT_EQ(node.m_right->m_parent, &node);
		EXPECT_EQ(currentExpected->children.size(), 2)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(UnaryExprNode& node) override {
		CheckNodeType(&node);
		EXPECT_EQ(node.m_op, currentExpected->content);
		EXPECT_TRUE(node.m_operand);
		AstCompareVisitor v(&currentExpected->children[0], path + "/operand");
		node.m_operand->Accept(v);
		EXPECT_EQ(node.m_operand->m_parent, &node);
		EXPECT_EQ(currentExpected->children.size(), 1)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(PostfixExprNode& node) override {
		CheckNodeType(&node);
		EXPECT_EQ(node.m_op, currentExpected->content);
		EXPECT_TRUE(node.m_primary);
		AstCompareVisitor v(&currentExpected->children[0], path + "/primary");
		node.m_primary->Accept(v);
		EXPECT_EQ(node.m_primary->m_parent, &node);
		EXPECT_EQ(currentExpected->children.size(), 1)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(FunctionCallNode& node) override {
		CheckNodeType(&node);
		EXPECT_TRUE(node.m_callee);
		AstCompareVisitor v(&currentExpected->children[0], path + "/callee");
		node.m_callee->Accept(v);
		EXPECT_EQ(node.m_callee->m_parent, &node);
		for (size_t i = 0; i < node.m_arguments.size(); ++i) {
			AstCompareVisitor v1(&currentExpected->children[i + 1], path + "/arg" + std::to_string(i));
			node.m_arguments[i]->Accept(v1);
			EXPECT_EQ(node.m_arguments[i]->m_parent, &node);
		}
		EXPECT_EQ(node.m_arguments.size(), currentExpected->children.size() - 1)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(ArrayIndexNode& node) override {
		CheckNodeType(&node);
		EXPECT_TRUE(node.m_array);
		AstCompareVisitor v1(&currentExpected->children[0], path + "/array");
		node.m_array->Accept(v1);
		EXPECT_EQ(node.m_array->m_parent, &node);
		EXPECT_TRUE(node.m_index);
		AstCompareVisitor v2(&currentExpected->children[1], path + "/index");
		node.m_index->Accept(v2);
		EXPECT_EQ(node.m_index->m_parent, &node);
		EXPECT_EQ(currentExpected->children.size(), 2)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(MemberAccessNode& node) override {
		CheckNodeType(&node);
		EXPECT_TRUE(node.m_object);
		AstCompareVisitor v1(&currentExpected->children[0], path + "/object");
		node.m_object->Accept(v1);
		EXPECT_EQ(node.m_object->m_parent, &node);
		EXPECT_TRUE(node.m_memberName);
		AstCompareVisitor v2(&currentExpected->children[1], path + "/memberName");
		node.m_memberName->Accept(v2);
		EXPECT_EQ(node.m_memberName->m_parent, &node);
		EXPECT_EQ(currentExpected->children.size(), 2)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(FunctionLiteralNode& node) override {
		CheckNodeType(&node);
		int cur = 0;
		for (size_t i = 0; i < node.m_params.size(); ++i) {
			AstCompareVisitor v1(&currentExpected->children[i], path + "/param" + std::to_string(i));
			node.m_params[i]->Accept(v1);
			EXPECT_EQ(node.m_params[i]->m_parent, &node);
		}
		cur += node.m_params.size();
		EXPECT_TRUE(node.m_returnType);
		AstCompareVisitor v2(&currentExpected->children[cur], path + "/returnType");
		node.m_returnType->Accept(v2);
		EXPECT_EQ(node.m_returnType->m_parent, &node);
		cur++;
		EXPECT_TRUE(node.m_body);
		AstCompareVisitor v3(&currentExpected->children[cur], path + "/body");
		node.m_body->Accept(v3);
		EXPECT_EQ(node.m_body->m_parent, &node);
		cur++;
		EXPECT_EQ(cur, currentExpected->children.size())
			<< "Child count mismatch at path: " << path;
	}

	void Visit(IdentifierNode& node) override {
		CheckNodeType(&node);
		EXPECT_EQ(node.m_name, currentExpected->content);
		EXPECT_TRUE(currentExpected->children.empty())
			<< "Child is not empty at path: " << path;
	}

	void Visit(LiteralNode& node) override {
		CheckNodeType(&node);
		EXPECT_EQ(node.m_value, currentExpected->content);
		EXPECT_TRUE(currentExpected->children.empty())
			<< "Child is not empty at path: " << path;
	}

	void Visit(ParameterNode& node) override {
		CheckNodeType(&node);
		EXPECT_TRUE(node.m_type);
		AstCompareVisitor v1(&currentExpected->children[0], path + "/type");
		node.m_type->Accept(v1);
		EXPECT_EQ(node.m_type->m_parent, &node);
		EXPECT_TRUE(node.m_declarator);
		AstCompareVisitor v2(&currentExpected->children[1], path + "/declarator");
		node.m_declarator->Accept(v2);
		EXPECT_EQ(node.m_declarator->m_parent, &node);
		EXPECT_EQ(currentExpected->children.size(), 2)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(DeclaratorNode& node) override {
		CheckNodeType(&node);
		int cur = 0;
		EXPECT_TRUE(node.m_name);
		AstCompareVisitor v(&currentExpected->children[0], path + "/name");
		node.m_name->Accept(v);
		EXPECT_EQ(node.m_name->m_parent, &node);
		cur++;
		for (int i = 0; i < node.m_arraySizes.size(); i++) {
			AstCompareVisitor v1(&currentExpected->children[i + cur], path + "/arraySizes");
			node.m_arraySizes[i]->Accept(v1);
			EXPECT_EQ(node.m_arraySizes[i]->m_parent, &node);
		}
		cur += node.m_arraySizes.size();
		if (node.m_initializer) {
			AstCompareVisitor v2(&currentExpected->children[cur], path + "/initializer");
			node.m_initializer->Accept(v2);
			EXPECT_EQ(node.m_initializer->m_parent, &node);
			cur++;
		}
		EXPECT_EQ(cur, currentExpected->children.size())
			<< "Child count mismatch at path: " << path;
	}

	void Visit(StructMemberNode& node) override {
		CheckNodeType(&node);
		EXPECT_TRUE(node.m_type);
		AstCompareVisitor v(&currentExpected->children[0], path + "/type");
		node.m_type->Accept(v);
		for (int i = 0; i < node.m_declarators.size(); i++) {
			AstCompareVisitor v1(&currentExpected->children[i + 1], path + "/declarators");
			node.m_declarators[i]->Accept(v1);
			EXPECT_EQ(node.m_declarators[i]->m_parent, &node);
		}
		EXPECT_EQ(node.m_declarators.size(), currentExpected->children.size() - 1)
			<< "Child count mismatch at path: " << path;
	}

	void Visit(InitializerNode& node) override {
		CheckNodeType(&node);
		for (size_t i = 0; i < node.m_values.size(); ++i) {
			AstCompareVisitor v1(&currentExpected->children[i], path + "/initializer" + std::to_string(i));
			node.m_values[i]->Accept(v1);
			EXPECT_EQ(node.m_values[i]->m_parent, &node);
		}
		EXPECT_EQ(node.m_values.size(), currentExpected->children.size())
			<< "Child count mismatch at path: " << path;
	}

	void Visit(BuiltinTypeNode& node) override {
		CheckNodeType(&node);
		EXPECT_EQ(node.m_name, currentExpected->content);
		EXPECT_TRUE(currentExpected->children.empty())
			<< "Child is not empty at path: " << path;
	}

	void Visit(NamedTypeNode& node) override {
		CheckNodeType(&node);
		EXPECT_EQ(node.m_name, currentExpected->content);
		EXPECT_TRUE(currentExpected->children.empty())
			<< "Child is not empty at path: " << path;
	}

	void Visit(FunctionTypeNode& node) override {
		CheckNodeType(&node);
		for (size_t i = 0; i < node.m_paramTypes.size(); ++i) {
			AstCompareVisitor v1(&currentExpected->children[i], path + "/param" + std::to_string(i));
			node.m_paramTypes[i]->Accept(v1);
			EXPECT_EQ(node.m_paramTypes[i]->m_parent, &node);
		}
		AstCompareVisitor v2(&currentExpected->children.back(), path + "/return");
		node.m_returnType->Accept(v2);
		EXPECT_EQ(node.m_returnType->m_parent, &node);
		EXPECT_EQ(node.m_paramTypes.size() + 1, currentExpected->children.size())
			<< "Child count mismatch at path: " << path;
	}
};


class AstNodeCountVisitor : public AstVisitor {
public:
	size_t count = 0;

	void Count(AstNode* node) {
		if (node) node->Accept(*this);
	}

	void Visit(ProgramNode& node) override {
		count++;
		for (auto child : node.m_declarations) Count(child);
	}

	void Visit(ImportNode& node) override { count++; }
	void Visit(FunctionDeclNode& node) override {
		count++;
		Count(node.m_returnType);
		Count(node.m_name);
		Count(node.m_body);
		for (auto p : node.m_params) Count(p);
	}

	void Visit(CompoundStmtNode& node) override {
		count++;
		for (auto stmt : node.m_statements) Count(stmt);
	}

	void Visit(ExpressionStmtNode& node) override {
		count++;
		Count(node.m_expression);
	}

	void Visit(VariableDeclNode& node) override {
		count++;
		Count(node.m_type);
		for (auto p : node.m_declarators)
			Count(p);
	}

	void Visit(StructDeclNode& node) override {
		count++;
		Count(node.m_name);
		for (auto m : node.m_members) Count(m);
	}

	void Visit(IfStmtNode& node) override {
		count++;
		Count(node.m_condition);
		Count(node.m_thenStmt);
		Count(node.m_elseStmt);
	}

	void Visit(SwitchStmtNode& node) override {
		count++;
		Count(node.m_condition);
		for (auto c : node.m_cases) Count(c);
		Count(node.m_default);
	}

	void Visit(CaseNode& node) override {
		count++;
		Count(node.m_literal);
		for (auto s : node.m_statements) Count(s);
	}

	void Visit(DefaultNode& node) override {
		count++;
		for (auto s : node.m_statements) Count(s);
	}

	void Visit(WhileStmtNode& node) override {
		count++;
		Count(node.m_condition);
		Count(node.m_body);
	}

	void Visit(ForStmtNode& node) override {
		count++;
		Count(node.m_init);
		Count(node.m_condition);
		Count(node.m_increment);
		Count(node.m_body);
	}

	void Visit(ReturnStmtNode& node) override {
		count++;
		Count(node.m_expression);
	}

	void Visit(BreakStmtNode& node) override { count++; }
	void Visit(ContinueStmtNode& node) override { count++; }

	void Visit(CommaExprNode& node) override {
		count++;
		for (auto e : node.m_expressions) Count(e);
	}

	void Visit(AssignmentExprNode& node) override {
		count++;
		Count(node.m_left);
		Count(node.m_right);
	}

	void Visit(ConditionalExprNode& node) override {
		count++;
		Count(node.m_condition);
		Count(node.m_trueExpr);
		Count(node.m_falseExpr);
	}

	void Visit(BinaryExprNode& node) override {
		count++;
		Count(node.m_left);
		Count(node.m_right);
	}

	void Visit(UnaryExprNode& node) override {
		count++;
		Count(node.m_operand);
	}

	void Visit(PostfixExprNode& node) override {
		count++;
		Count(node.m_primary);
	}

	void Visit(FunctionCallNode& node) override {
		count++;
		Count(node.m_callee);
		for (auto a : node.m_arguments) Count(a);
	}

	void Visit(ArrayIndexNode& node) override {
		count++;
		Count(node.m_array);
		Count(node.m_index);
	}

	void Visit(MemberAccessNode& node) override {
		count++;
		Count(node.m_object);
		Count(node.m_memberName);
	}

	void Visit(FunctionLiteralNode& node) override {
		count++;
		Count(node.m_body);
		Count(node.m_returnType);
		for (auto p : node.m_params) Count(p);
	}

	void Visit(IdentifierNode& node) override { count++; }
	void Visit(LiteralNode& node) override { count++; }

	void Visit(ParameterNode& node) override {
		count++;
		Count(node.m_type);
		Count(node.m_declarator);
	}

	void Visit(DeclaratorNode& node) override {
		count++;
		Count(node.m_name);
		for (auto s : node.m_arraySizes) Count(s);
		if (node.m_initializer) Count(node.m_initializer);
	}

	void Visit(StructMemberNode& node) override {
		count++;
		Count(node.m_type);
		for (auto d : node.m_declarators) Count(d);
	}

	void Visit(InitializerNode& node) override {
		count++;
		for (auto e : node.m_values) Count(e);
	}

	void Visit(BuiltinTypeNode& node) override { count++; }
	void Visit(NamedTypeNode& node) override { count++; }
	void Visit(FunctionTypeNode& node) override {
		count++;
		Count(node.m_returnType);
		for (auto p : node.m_paramTypes) Count(p);
	}
};

struct ParserCase {
	std::string input;
	ExpectedNode expectedTree;
};

class ParserSyntaxTest : public ::testing::TestWithParam<ParserCase> {};

static void ExpectAstMatch(AstNode* actual, const ExpectedNode& expected) {
	ASSERT_NE(actual, nullptr);
	AstCompareVisitor visitor(&expected);
	actual->Accept(visitor);
}

static void ExpectAstNodeNumsMatch(AstNode* root, size_t expected) {
	AstNodeCountVisitor counter;
	counter.Count(root);
	EXPECT_EQ(counter.count, expected);
}

TEST_P(ParserSyntaxTest, ParsesAst) {
	static Parser g_parser;
	const auto& param = GetParam();
	AstNode* root = g_parser.Parse(param.input);
	AstPrinter::PrintAstTree(root);
	ASSERT_NE(root, nullptr);
	ExpectAstMatch(root, param.expectedTree);
	ExpectAstNodeNumsMatch(root, g_parser.GetNodes().size());
}

static ExpectedNode MakeNode(NodeType::Type type,
	std::string content = "",
	std::vector<ExpectedNode> children = {}) {
	return { type, std::move(content), std::move(children) };
}

const char* source1 = R"(import math;)";

auto case1 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::IMPORT_STMT, "math")
	});

const char* source2 = R"(function int add(int a, int b) { return a + b; })";

auto case2 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::FUNCTION_DECL, "", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::IDENTIFIER, "add"),
		MakeNode(NodeType::PARAMETER, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::DECLARATOR, "",{
				MakeNode(NodeType::IDENTIFIER, "a"),
			})
		}),
		MakeNode(NodeType::PARAMETER, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::DECLARATOR, "",{
				MakeNode(NodeType::IDENTIFIER, "b")
			})
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::RETURN_STMT, "", {
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
		MakeNode(NodeType::DECLARATOR, "", {
			MakeNode(NodeType::IDENTIFIER, "flag"),
			MakeNode(NodeType::LITERAL, "true")
		}),
		MakeNode(NodeType::DECLARATOR, "", {
			MakeNode(NodeType::IDENTIFIER, "tag"),
			MakeNode(NodeType::LITERAL, "false")

		})
	}),
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::NAMED_TYPE, "Member"),
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "m"),
				MakeNode(NodeType::INITIALIZER, "", {
					MakeNode(NodeType::LITERAL, "1"),
					MakeNode(NodeType::LITERAL, "2")
					})
				})
		})
	});

const char* source4 = R"(let int arr[3] = { "abc", "def", "gij" };)";

auto case4 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::VAR_DECL, "let", {
	MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::DECLARATOR, "", {
			MakeNode(NodeType::IDENTIFIER, "arr"),
			MakeNode(NodeType::LITERAL, "3"),
			MakeNode(NodeType::INITIALIZER,"",{
				MakeNode(NodeType::LITERAL, "\"abc\""),
				MakeNode(NodeType::LITERAL, "\"def\""),
				MakeNode(NodeType::LITERAL, "\"gij\"")
			})
		})
	})
	});

const char* source5 = R"(let int array[3][3] = {{1,2,3},{4,5,6},{7,8,9}}; let empty array = {};)";

auto case5 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::DECLARATOR, "", {
			MakeNode(NodeType::IDENTIFIER, "array"),
			MakeNode(NodeType::LITERAL, "3"),
			MakeNode(NodeType::LITERAL, "3"),
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
	}),
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::NAMED_TYPE, "empty"),
		MakeNode(NodeType::DECLARATOR, "", {
			MakeNode(NodeType::IDENTIFIER, "array"),
			MakeNode(NodeType::INITIALIZER, "", {})
		})
	})
	});

const char* source6 = R"(struct Point { int x; int y; };)";

auto case6 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::STRUCT_DECL, "", {
		MakeNode(NodeType::IDENTIFIER, "Point"),
		MakeNode(NodeType::STRUCT_MEMBER_DECL, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::DECLARATOR, "",{
				MakeNode(NodeType::IDENTIFIER, "x"),
			})
		}),
		MakeNode(NodeType::STRUCT_MEMBER_DECL, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::DECLARATOR, "",{
				MakeNode(NodeType::IDENTIFIER, "y"),
			})
		})
	})
	});

const char* source7 = R"(if (x > 0) { y = 1; } else { y = 2; })";

auto case7 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::IF_STMT, "", {
		MakeNode(NodeType::BINARY_EXPR, ">", {
			MakeNode(NodeType::IDENTIFIER, "x"),
			MakeNode(NodeType::LITERAL, "0")
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::EXPRESSION_STMT, "", {
				MakeNode(NodeType::ASSIGN_EXPR, "=", {
					MakeNode(NodeType::IDENTIFIER, "y"),
					MakeNode(NodeType::LITERAL, "1")
				})
			})
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::EXPRESSION_STMT, "", {
				MakeNode(NodeType::ASSIGN_EXPR, "=", {
					MakeNode(NodeType::IDENTIFIER, "y"),
					MakeNode(NodeType::LITERAL, "2")
				})
			})
		})
	})
	});

const char* source8 = R"(while (i < 10) { i = i + 1; continue; })";

auto case8 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::WHILE_STMT, "", {
		MakeNode(NodeType::BINARY_EXPR, "<", {
			MakeNode(NodeType::IDENTIFIER, "i"),
			MakeNode(NodeType::LITERAL, "10")
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::EXPRESSION_STMT, "", {
				MakeNode(NodeType::ASSIGN_EXPR, "=", {
					MakeNode(NodeType::IDENTIFIER, "i"),
					MakeNode(NodeType::BINARY_EXPR, "+", {
						MakeNode(NodeType::IDENTIFIER, "i"),
						MakeNode(NodeType::LITERAL, "1")
					})
				})
			}),
			MakeNode(NodeType::CONTINUE_STMT, "")
		})
	})
	});

const char* source9 = R"(switch(x) { case 1: y = 2; break; default: y = 0; })";

auto case9 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::SWITCH_STMT, "", {
		MakeNode(NodeType::IDENTIFIER, "x"),
		MakeNode(NodeType::CASE_STMT, "", {
			MakeNode(NodeType::LITERAL, "1"),
			MakeNode(NodeType::EXPRESSION_STMT, "", {
				MakeNode(NodeType::ASSIGN_EXPR, "=", {
					MakeNode(NodeType::IDENTIFIER, "y"),
					MakeNode(NodeType::LITERAL, "2")
				})
			}),
			MakeNode(NodeType::BREAK_STMT, "")
		}),
		MakeNode(NodeType::DEFAULT_STMT, "", {
			MakeNode(NodeType::EXPRESSION_STMT, "", {
				MakeNode(NodeType::ASSIGN_EXPR, "=", {
					MakeNode(NodeType::IDENTIFIER, "y"),
					MakeNode(NodeType::LITERAL, "0")
				})
			})
		})
	})
	});

const char* source10 = R"(for (i = 0; i < 10; i = i + 1) sum = sum + i;)";

auto case10 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::FOR_STMT, "", {
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
		MakeNode(NodeType::EXPRESSION_STMT, "", {
			MakeNode(NodeType::ASSIGN_EXPR, "=", {
				MakeNode(NodeType::IDENTIFIER, "sum"),
				MakeNode(NodeType::BINARY_EXPR, "+", {
					MakeNode(NodeType::IDENTIFIER, "sum"),
					MakeNode(NodeType::IDENTIFIER, "i")
				})
			})
		})
	})
	});

const char* source11 = R"(a = x > 0 ? 1 : 2;)";

auto case11 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::EXPRESSION_STMT, "", {
		MakeNode(NodeType::ASSIGN_EXPR, "=", {
			MakeNode(NodeType::IDENTIFIER, "a"),
			MakeNode(NodeType::COND_EXPR, "?", {
				MakeNode(NodeType::BINARY_EXPR, ">", {
					MakeNode(NodeType::IDENTIFIER, "x"),
					MakeNode(NodeType::LITERAL, "0")
				}),
				MakeNode(NodeType::LITERAL, "1"),
				MakeNode(NodeType::LITERAL, "2")
			})
		})
	})
	});

const char* source12 = R"(obj.list[i++].value = -10;)";

auto case12 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::EXPRESSION_STMT, "", {
		MakeNode(NodeType::ASSIGN_EXPR, "=", {
			MakeNode(NodeType::MEMBER_ACCESS, ".", {
				MakeNode(NodeType::ARRAY_INDEX, "", {
					MakeNode(NodeType::MEMBER_ACCESS, ".", {
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
	})
	});

const char* source13 = R"((a + b) * (c - (d + e));)";

auto case13 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::EXPRESSION_STMT, "", {
		MakeNode(NodeType::BINARY_EXPR, "*", {
			MakeNode(NodeType::BINARY_EXPR, "+", {
				MakeNode(NodeType::IDENTIFIER, "a"),
				MakeNode(NodeType::IDENTIFIER, "b")
			}),
			MakeNode(NodeType::BINARY_EXPR, "-", {
				MakeNode(NodeType::IDENTIFIER, "c"),
				MakeNode(NodeType::BINARY_EXPR, "+", {
					MakeNode(NodeType::IDENTIFIER, "d"),
					MakeNode(NodeType::IDENTIFIER, "e")
				})
			})
		})
	})
	});

const char* source14 = R"(foo(1, 2);)";

auto case14 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::EXPRESSION_STMT, "", {
		MakeNode(NodeType::FUNCTION_CALL, "", {
			MakeNode(NodeType::IDENTIFIER, "foo"),
			MakeNode(NodeType::LITERAL, "1"),
			MakeNode(NodeType::LITERAL, "2")
		})
	})
	});

const char* source15 = R"(;)";

auto case15 = MakeNode(NodeType::PROGRAM, "", {
		MakeNode(NodeType::EXPRESSION_STMT, "", {})
	});




const char* source16 = R"(let (int, int) -> int f = lambda(int x, int y) -> int { return x + y; };)";

auto case16 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::VAR_DECL, "let", {
		MakeNode(NodeType::FUNCTION_TYPE, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::BUILTIN_TYPE, "int")
		}),
		MakeNode(NodeType::DECLARATOR, "", {
			MakeNode(NodeType::IDENTIFIER, "f"),
			MakeNode(NodeType::FUNCTION_LITERAL, "", {
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "",{
						MakeNode(NodeType::IDENTIFIER, "x"),
					}),

				}),
				MakeNode(NodeType::PARAMETER, "", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "",{
						MakeNode(NodeType::IDENTIFIER, "y"),
					})
				}),
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::COMPOUND_STMT, "", {
					MakeNode(NodeType::RETURN_STMT, "", {
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


const char* source17 = R"(function double compute((int) -> double f) { return f(1,2); })";

auto case17 = MakeNode(NodeType::PROGRAM, "", {
	MakeNode(NodeType::FUNCTION_DECL, "", {
		MakeNode(NodeType::BUILTIN_TYPE, "double"),
		MakeNode(NodeType::IDENTIFIER, "compute"),
		MakeNode(NodeType::PARAMETER, "", {
			MakeNode(NodeType::FUNCTION_TYPE, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::BUILTIN_TYPE, "double"),
			}),
				MakeNode(NodeType::DECLARATOR, "",{
					MakeNode(NodeType::IDENTIFIER, "f"),
				})
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::RETURN_STMT, "", {
				MakeNode(NodeType::FUNCTION_CALL, "", {
					MakeNode(NodeType::IDENTIFIER, "f"),
					MakeNode(NodeType::LITERAL, "1"),
					MakeNode(NodeType::LITERAL, "2")
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
	MakeNode(NodeType::FUNCTION_DECL, "", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::IDENTIFIER, "inc"),
		MakeNode(NodeType::PARAMETER, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::DECLARATOR, "",{
				MakeNode(NodeType::IDENTIFIER, "x"),
			})
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::RETURN_STMT, "", {
				MakeNode(NodeType::BINARY_EXPR, "+", {
					MakeNode(NodeType::IDENTIFIER, "x"),
					MakeNode(NodeType::LITERAL, "1")
				})
			})
		})
	}),
	MakeNode(NodeType::FUNCTION_DECL, "", {
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
			MakeNode(NodeType::WHILE_STMT, "", {
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
							MakeNode(NodeType::EXPRESSION_STMT, "", {
								MakeNode(NodeType::ASSIGN_EXPR, "=", {
									MakeNode(NodeType::IDENTIFIER, "sum"),
									MakeNode(NodeType::BINARY_EXPR, "+", {
										MakeNode(NodeType::IDENTIFIER, "sum"),
										MakeNode(NodeType::FUNCTION_CALL, "", {
											MakeNode(NodeType::IDENTIFIER, "inc"),
											MakeNode(NodeType::IDENTIFIER, "i"),
											})
										})
									})
								})
							}),
						MakeNode(NodeType::COMPOUND_STMT, "", {
							MakeNode(NodeType::CONTINUE_STMT,"")
							})
						}),
						MakeNode(NodeType::EXPRESSION_STMT, "", {
						MakeNode(NodeType::ASSIGN_EXPR, "=", {
							MakeNode(NodeType::IDENTIFIER, "i"),
							MakeNode(NodeType::BINARY_EXPR, "+", {
								MakeNode(NodeType::IDENTIFIER, "i"),
								MakeNode(NodeType::LITERAL, "1")
							})
						})
					})
					}),
				}),
			MakeNode(NodeType::EXPRESSION_STMT, "", {
				MakeNode(NodeType::FUNCTION_CALL, "", {
					MakeNode(NodeType::IDENTIFIER, "print"),
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
	MakeNode(NodeType::FUNCTION_DECL, "", {
		MakeNode(NodeType::BUILTIN_TYPE, "double"),
		MakeNode(NodeType::IDENTIFIER, "square"),
		MakeNode(NodeType::PARAMETER, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "double"),
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "x")
			})
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::RETURN_STMT, "", {
				MakeNode(NodeType::BINARY_EXPR, "*", {
					MakeNode(NodeType::IDENTIFIER, "x"),
					MakeNode(NodeType::IDENTIFIER, "x")
				})
			})
		})
	}),
		// function test(int n) { ... }
		MakeNode(NodeType::FUNCTION_DECL, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "double"),
			MakeNode(NodeType::IDENTIFIER, "test"),
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "n"),
				})
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
								MakeNode(NodeType::BREAK_STMT, "")
							}),
							MakeNode(NodeType::EXPRESSION_STMT, "", {
								MakeNode(NodeType::ASSIGN_EXPR, "=", {
									MakeNode(NodeType::IDENTIFIER, "res"),
									MakeNode(NodeType::BINARY_EXPR, "+", {
										MakeNode(NodeType::IDENTIFIER, "res"),
										MakeNode(NodeType::FUNCTION_CALL, "", {
											MakeNode(NodeType::IDENTIFIER, "square"),
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
					MakeNode(NodeType::RETURN_STMT, "", {
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
	MakeNode(NodeType::FUNCTION_DECL, "", {
		MakeNode(NodeType::BUILTIN_TYPE, "int"),
		MakeNode(NodeType::IDENTIFIER, "max"),
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
		}),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::IF_STMT, "if", {
				MakeNode(NodeType::BINARY_EXPR, ">", {
					MakeNode(NodeType::IDENTIFIER, "a"),
					MakeNode(NodeType::IDENTIFIER, "b")
				}),
				MakeNode(NodeType::RETURN_STMT, "", {
					MakeNode(NodeType::IDENTIFIER, "a")
				}),
				MakeNode(NodeType::RETURN_STMT, "", {
					MakeNode(NodeType::IDENTIFIER, "b")
				})
			})
		})
	}),

		// --- function int clamp(int x, int low, int high) ---
		MakeNode(NodeType::FUNCTION_DECL, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::IDENTIFIER, "clamp"),
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "x")
				})
			}),
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "low")
				})
			}),
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "high")
				})
			}),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				MakeNode(NodeType::IF_STMT, "if", {
					MakeNode(NodeType::BINARY_EXPR, "<", {
						MakeNode(NodeType::IDENTIFIER, "x"),
						MakeNode(NodeType::IDENTIFIER, "low")
					}),
					MakeNode(NodeType::RETURN_STMT, "", {
						MakeNode(NodeType::IDENTIFIER, "low")
					}),
					MakeNode(NodeType::IF_STMT, "if", {
						MakeNode(NodeType::BINARY_EXPR, ">", {
							MakeNode(NodeType::IDENTIFIER, "x"),
							MakeNode(NodeType::IDENTIFIER, "high")
						}),
						MakeNode(NodeType::RETURN_STMT, "", {
							MakeNode(NodeType::IDENTIFIER, "high")
						}),
						MakeNode(NodeType::RETURN_STMT, "", {
							MakeNode(NodeType::IDENTIFIER, "x")
						})
					})
				})
			})
		}),

		// --- function void main() ---
		MakeNode(NodeType::FUNCTION_DECL, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "void"),
			MakeNode(NodeType::IDENTIFIER, "main"),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				// let int a = max();
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "a"),
						MakeNode(NodeType::FUNCTION_CALL, "", {
							MakeNode(NodeType::IDENTIFIER, "max")
						})
					})
				}),

					// a = max(10, 20);
					MakeNode(NodeType::EXPRESSION_STMT, "", {
						MakeNode(NodeType::ASSIGN_EXPR, "=", {
							MakeNode(NodeType::IDENTIFIER, "a"),
							MakeNode(NodeType::FUNCTION_CALL, "", {
								MakeNode(NodeType::IDENTIFIER, "max"),
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
							MakeNode(NodeType::FUNCTION_CALL, "", {
								MakeNode(NodeType::IDENTIFIER, "clamp"),
								MakeNode(NodeType::IDENTIFIER, "a"),
								MakeNode(NodeType::LITERAL, "0"),
								MakeNode(NodeType::LITERAL, "50")
							})
						})
					}),

					// print(b);
					MakeNode(NodeType::EXPRESSION_STMT, "", {
						MakeNode(NodeType::FUNCTION_CALL, "", {
							MakeNode(NodeType::IDENTIFIER, "print"),
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
		MakeNode(NodeType::DECLARATOR, "", {
			MakeNode(NodeType::IDENTIFIER, "globalVal"),
			MakeNode(NodeType::LITERAL, "42"),
		})
	}),

	MakeNode(NodeType::VAR_DECL, "const", {
		MakeNode(NodeType::BUILTIN_TYPE, "string"),
		MakeNode(NodeType::DECLARATOR, "", {
			MakeNode(NodeType::IDENTIFIER, "message"),
			MakeNode(NodeType::LITERAL, "\"Hello\""),
		})
	}),

	MakeNode(NodeType::FUNCTION_DECL, "", {
		MakeNode(NodeType::BUILTIN_TYPE, "string"),
		MakeNode(NodeType::IDENTIFIER, "printMessage"),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::EXPRESSION_STMT, "", {
				MakeNode(NodeType::FUNCTION_CALL, "", {
					MakeNode(NodeType::IDENTIFIER, "print"),
					MakeNode(NodeType::IDENTIFIER, "message")
				})
			})
		})
	}),

	MakeNode(NodeType::FUNCTION_DECL, "", {
		MakeNode(NodeType::BUILTIN_TYPE, "void"),
		MakeNode(NodeType::IDENTIFIER, "main"),
		MakeNode(NodeType::COMPOUND_STMT, "", {
			MakeNode(NodeType::EXPRESSION_STMT, "", {
				MakeNode(NodeType::FUNCTION_CALL, "", {
					MakeNode(NodeType::IDENTIFIER, "print"),
					MakeNode(NodeType::IDENTIFIER, "globalVal"),
				})
			}),
			MakeNode(NodeType::EXPRESSION_STMT, "", {
				MakeNode(NodeType::FUNCTION_CALL, "",{
					MakeNode(NodeType::IDENTIFIER, "printMessage")
				})
			})
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
	MakeNode(NodeType::STRUCT_DECL, "", {
		MakeNode(NodeType::IDENTIFIER, "Point"),
		MakeNode(NodeType::STRUCT_MEMBER_DECL, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "x")
			})
		}),
		MakeNode(NodeType::STRUCT_MEMBER_DECL, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::DECLARATOR, "", {
				MakeNode(NodeType::IDENTIFIER, "y")
			})
		})
	}),

		// --- sumArray ---
		MakeNode(NodeType::FUNCTION_DECL, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "int"),
			MakeNode(NodeType::IDENTIFIER, "sumArray"),
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "arr")
				})
			}),
			MakeNode(NodeType::PARAMETER, "", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "n")
				})
			}),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "total"),
						MakeNode(NodeType::LITERAL, "0")
					})
				}),
				MakeNode(NodeType::FOR_STMT, "", {
					MakeNode(NodeType::VAR_DECL, "let", {
						MakeNode(NodeType::BUILTIN_TYPE, "int"),
						MakeNode(NodeType::DECLARATOR, "", {
							MakeNode(NodeType::IDENTIFIER, "i"),
							MakeNode(NodeType::LITERAL, "0"),
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
						MakeNode(NodeType::EXPRESSION_STMT, "", {
							MakeNode(NodeType::ASSIGN_EXPR, "=", {
								MakeNode(NodeType::IDENTIFIER, "total"),
								MakeNode(NodeType::BINARY_EXPR, "+", {
									MakeNode(NodeType::IDENTIFIER, "total"),
									MakeNode(NodeType::ARRAY_INDEX, "", {
										MakeNode(NodeType::IDENTIFIER, "arr"),
										MakeNode(NodeType::IDENTIFIER, "i"),
									}),
								})
							})
						}),
						MakeNode(NodeType::IF_STMT, "", {
							MakeNode(NodeType::BINARY_EXPR, "==", {
								MakeNode(NodeType::BINARY_EXPR, "%", {
									MakeNode(NodeType::ARRAY_INDEX, "", {
										MakeNode(NodeType::IDENTIFIER, "arr"),
										MakeNode(NodeType::IDENTIFIER, "i"),
									}),
									MakeNode(NodeType::LITERAL, "2"),
								}),
								MakeNode(NodeType::LITERAL, "0"),
							}),
							MakeNode(NodeType::COMPOUND_STMT, "", {
								MakeNode(NodeType::EXPRESSION_STMT, "", {
									MakeNode(NodeType::ASSIGN_EXPR, "=", {
										MakeNode(NodeType::IDENTIFIER, "total"),
										MakeNode(NodeType::BINARY_EXPR, "+", {
											MakeNode(NodeType::IDENTIFIER, "total"),
											MakeNode(NodeType::LITERAL, "1"),
										}),
									})
								})
							})
						}),
					}),
				}),
				MakeNode(NodeType::RETURN_STMT, "return", {
					MakeNode(NodeType::IDENTIFIER, "total"),
				}),
			}),
		}),

		// --- makePoint ---
		MakeNode(NodeType::FUNCTION_DECL, "", {
			MakeNode(NodeType::NAMED_TYPE, "Point"),
			MakeNode(NodeType::IDENTIFIER, "makePoint"),
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
			MakeNode(NodeType::COMPOUND_STMT, "", {
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::NAMED_TYPE, "Point"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "p"),
						MakeNode(NodeType::FUNCTION_CALL, "",{
							MakeNode(NodeType::IDENTIFIER, "Point")
						}),
					})
				}),
				MakeNode(NodeType::EXPRESSION_STMT, "", {
					MakeNode(NodeType::ASSIGN_EXPR, "=", {
						MakeNode(NodeType::MEMBER_ACCESS, ".", {
							MakeNode(NodeType::IDENTIFIER, "p"),
							MakeNode(NodeType::IDENTIFIER, "x"),
						}),
						MakeNode(NodeType::IDENTIFIER, "a"),
					})
				}),
				MakeNode(NodeType::EXPRESSION_STMT, "", {
					MakeNode(NodeType::ASSIGN_EXPR, "=", {
						MakeNode(NodeType::MEMBER_ACCESS, ".", {
							MakeNode(NodeType::IDENTIFIER, "p"),
							MakeNode(NodeType::IDENTIFIER, "y"),
						}),
						MakeNode(NodeType::IDENTIFIER, "b"),
					})
				}),
				MakeNode(NodeType::RETURN_STMT, "", {
					MakeNode(NodeType::IDENTIFIER, "p"),
				}),
			}),
		}),

		// --- main ---
		MakeNode(NodeType::FUNCTION_DECL, "", {
			MakeNode(NodeType::BUILTIN_TYPE, "void"),
			MakeNode(NodeType::IDENTIFIER, "main"),
			MakeNode(NodeType::COMPOUND_STMT, "", {
				MakeNode(NodeType::VAR_DECL, "const", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "data"),
						MakeNode(NodeType::LITERAL, "4"),
						MakeNode(NodeType::INITIALIZER, "", {
							MakeNode(NodeType::LITERAL, "1"),
							MakeNode(NodeType::LITERAL, "2"),
							MakeNode(NodeType::LITERAL, "3"),
							MakeNode(NodeType::LITERAL, "4"),
						})
					}),
				}),
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::NAMED_TYPE, "Point"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "pt"),
						MakeNode(NodeType::FUNCTION_CALL, "", {
							MakeNode(NodeType::IDENTIFIER, "makePoint"),
								MakeNode(NodeType::FUNCTION_CALL, "", {
								MakeNode(NodeType::IDENTIFIER, "sumArray"),
								MakeNode(NodeType::IDENTIFIER, "data"),
								MakeNode(NodeType::LITERAL, "4")
							}),
							MakeNode(NodeType::LITERAL, "10")
						})
					})
				}),
				MakeNode(NodeType::EXPRESSION_STMT, "", {
					MakeNode(NodeType::FUNCTION_CALL, "", {
						MakeNode(NodeType::IDENTIFIER, "print"),
						MakeNode(NodeType::BINARY_EXPR, "+", {
						MakeNode(NodeType::MEMBER_ACCESS, ".", {
							MakeNode(NodeType::IDENTIFIER, "pt"),
							MakeNode(NodeType::IDENTIFIER, "x"),
						}),
						MakeNode(NodeType::MEMBER_ACCESS, ".", {
							MakeNode(NodeType::IDENTIFIER, "pt"),
							MakeNode(NodeType::IDENTIFIER, "y"),
						})
					})
					})
				})
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
		MakeNode(NodeType::COMPOUND_STMT, "", {
			// let int a = 5;
			MakeNode(NodeType::VAR_DECL, "let", {
				MakeNode(NodeType::BUILTIN_TYPE, "int"),
				MakeNode(NodeType::DECLARATOR, "", {
					MakeNode(NodeType::IDENTIFIER, "a"),
					MakeNode(NodeType::LITERAL, "5"),
				})
			}),
				// let int b = 10;
				MakeNode(NodeType::VAR_DECL, "let", {
					MakeNode(NodeType::BUILTIN_TYPE, "int"),
					MakeNode(NodeType::DECLARATOR, "", {
						MakeNode(NodeType::IDENTIFIER, "b"),
						MakeNode(NodeType::LITERAL, "10")
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
							MakeNode(NodeType::EXPRESSION_STMT, "", {
								MakeNode(NodeType::FUNCTION_CALL, "", {
									MakeNode(NodeType::IDENTIFIER, "print"),
									MakeNode(NodeType::IDENTIFIER, "a")
								})
							})
						}),
						MakeNode(NodeType::COMPOUND_STMT, "", {
							MakeNode(NodeType::EXPRESSION_STMT, "", {
								MakeNode(NodeType::FUNCTION_CALL, "", {
									MakeNode(NodeType::IDENTIFIER, "print"),
									MakeNode(NodeType::IDENTIFIER, "b"),
								}),
							})
						})
					}),
				}),
					// else block
					MakeNode(NodeType::COMPOUND_STMT, "", {
						MakeNode(NodeType::EXPRESSION_STMT, "", {
							MakeNode(NodeType::FUNCTION_CALL, "", {
								MakeNode(NodeType::IDENTIFIER, "print"),
								MakeNode(NodeType::LITERAL, "0")
							})
						})
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